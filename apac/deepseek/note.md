### 1. DeepSeek 模型架构
DeepSeek V3/R1 模型在设计上融合了多头潜在注意力（Multi-Head Latent Attention, MLA）和混合专家（Mixture of Experts, MoE）架构，以实现高效推理和经济训练 。
- MLA (Multi-Head Latent Attention)：
```
MLA 是一种注意力机制，通过压缩潜在键值（KV）缓存来节省内存，同时保持注意力计算的效率 。
它通过将键（Key）和值（Value）投影到较低维度的潜在空间来减少 KV 缓存的大小，从而降低内存占用 。
与传统的 Multi-Head Attention (MHA)、Grouped-Query Attention (GQA) 和 Multi-Query Attention (MQA) 相比，MLA 在缓存效率方面有显著优势 。
文档中提到，MLA 通过矩阵吸收（Mat Absorb）进一步优化，将某些权重矩阵合并，以减少运行时计算量 。例如，将 
W^UK吸收进 W^UQ,将 W^UV吸收进 W^O，从而简化注意力权重的计算和最终输出的生成 。
```
- MLA 的优化方法包括：
1. CD (Cache Decompressed): 解压潜在 KV 缓存为 K/V，并存储 K/V，不节省内存 。
2. CC (Cache Compressed): 压缩为潜在 KV，并存储潜在 KV，节省内存但增加计算 。
3. A_CC (Absorbed_CacheCompressed): 在运行时将 W^UK吸收进 W^UQ,将 W^UV吸收进 W^O，并计算 W^UK∗W^UQ。
4. A_CC_ME (Absorbed_CacheCompressed_MoveElision): 额外单独计算 Rope 和 Nope 。
5. AM_CC_ME (Absorbed Materialized_CacheCompressed_MoveElision): 提前计算 W^Uk∗W^UO。
- DeepSeekMoE：
```
DeepSeek V3 MoE 包含 1 个共享专家（始终活跃），用于捕捉通用特征，以及 256 个路由专家，用于捕捉独特特征 。每个 token 激活 8 个专家 。

前 3 层使用 1 个共享专家 + 8 个固定活跃专家，或者 1 个密集 MLP 。

DeepSeekMoE 通过路由器（Router）机制将输入 token 路由到不同的专家，并结合共享专家和路由专家的输出来生成最终输出 。
```
- 多 token 预测 (Multi-token Prediction, MTP)：
```
MTP 模块旨在加速模型解码速度 。

它包含 D 个顺序模块，用于预测 D 个额外的 token 。

DeepSeek V3 中，D=1，包含 1 个 MTP 模块，第二个额外 token 的接受率约为 85-90%，使得解码速度提高 1.8 倍 TPS 。

在推理时，可以直接丢弃 MTP 模块以使用主模块，或重用这些 MTP 模块进行推测性解码（speculative decoding）以改善生成延迟 。
```
### 2. DeepSeek 推理框架
文档中比较了不同推理框架的关键特性，包括并行化、内核、量化和其它优化 。
1. 并行化 (Parallelism)：
- TP (Tensor Parallelism)：在多个设备上分割模型的张量，以处理大型模型 。
- DP (Data Parallelism)：在多个设备上复制模型，并在不同设备上处理不同批次的数据 。
- EP (Expert Parallelism)：针对 MoE 模型，将不同的专家放置在不同的设备上 。
- PP (Pipeline Parallelism)：将模型的不同层放置在不同的设备上，形成流水线 。
2. DP - MLA：
- 对于 MLA，每个 GPU 维护 
bs/DP*seq_len 的 KV 缓存 。
- 通过 Allgather 操作，每个 GPU 获取完整的序列隐藏状态，然后进行 TP MoE 。
3. EP - MoE：
- 使用 DeepEP 进行专家并行化 。
- DeepEP 具有自动模式（用于非 PD 解耦服务）、正常模式（用于 PD 预填充服务器）和低延迟模式（用于 PD 解码服务器） 。
- 关键优势在于更高效的 token 路由 。
4. 注意力 (Attention)：
- 支持混合 DP 和 TP 。
- 关键优势：消除跨设备的 KV 缓存重复，并通过避免冗余 KV 缓存存储来减少通信 。
5. 密集 FFNs (MLP)：
- 支持纯 DP 或纯 TP 。
- 纯 TP 可能导致碎片化 。
- 引入 DP 以减少碎片化，提高内存和计算效率 。
- 关键优势：降低峰值内存使用，将纯 TP 中的两次 all-reduce 操作替换为一次 reduce-scatter + all-gather（减少 50% 开销），纯 DP 密集 MLP 与纯 DP Attention 彻底消除设备间通信 。
6. 稀疏 FFNs (MoE)：
- 采用 DeepEP 。
- DeepEP 自动模式用于非 PD 解耦服务，正常模式用于 PD 预填充服务器，低延迟模式用于 PD 解码服务器 。
7. LM Head：
- 采用 DP 而不是 TP (传统词汇并行) 。
- 关键优势：更低的内存开销，简化通信 。
8. PD (Prefill-Decode) 解耦实现：
```
非阻塞传输：数据发送和接收操作在后台线程中运行，不中断调度器的事件循环 。

基于 RDMA 的传输：利用队列对（queue pairs）进行连接，以及散布-收集元素（scatter-gather elements, SGE）进行非连续内存块的高效传输 。

灵活的 API 集成：SGLang 提供可适应的 API，集成 Mooncake 和 NIXL 等高性能 RDMA 库，简化数据传输 。
```
8. 大规模专家并行化 (Large-scale Expert Parallelism)：
- DeepEP：用于大规模 EP 通信的理念，包括正常和低延迟内核 。
- 自适应模式的局限性：无法在同一通信组中同时支持正常调度（用于预填充）和低延迟调度（用于解码） 。PD 解耦可以解决此问题 。
- DeepGEMM：包含连续布局内核（用于上下文阶段 + 正常 DeepEP 内核）、掩码布局内核（用于解码阶段 + 低延迟 DeepEP 内核）和通用 GEMM 内核（用于 MoE TP 和通用 GeMM） 。
9. 两批次重叠 (Two-batch Overlap, TBO)：
- 将批次分为两个微批次，以重叠计算和通信 。
- SGLang 预填充重叠实现：优先将计算任务提交到 GPU，然后启动 CPU 阻塞通信 。
- TBO 实现了 27% 到 35% 的吞吐量提升 。
10. 专家并行负载均衡器 (Expert Parallelism Load Balancer, EPLB)：
- EPLB 将专家分布统计数据作为输入，并计算出专家的最佳安排以最小化不平衡 。
- DeepSeek 采用冗余专家策略并启发式地打包重复专家 。
- EPLB 必须与实际服务工作负载紧密匹配，有两种策略可以增强这种匹配：增加批次大小（通过扩展集群或使用 MTP 等）和周期性再平衡 。
- SGLang 再平衡实现包括：系统加载阶段（从磁盘到主内存预加载权重）、再平衡准备阶段（异步传输权重到设备内存）和再平衡执行阶段（设备到设备复制更新权重） 。
### 3. DeepSeek 推理基准
- 基本概念：
1. 并发 (Concurrency)：推理期间，单个推理服务同时处理的请求数量 。
2. 吞吐量 (Throughput)：在同时服务多个用户的情况下，系统处理的总 token 数。包括输入吞吐量和输出吞吐量。通常客户更关注与硬件利用率相关的输出吞吐量 。
3. TOPT (Time Per Output Token)：每输出 token 时间 。
4. TTFT (Time To First Token)：首 token 时间 。
5. TPS/user (Tokens Per Second per user)：每用户每秒 token 数，与在线用户体验相关 。
6. 更高的并发性导致更高的输出吞吐量和硬件利用率，但 TOPT 和 TPS/user 会变差 。
7. MTP (Multi-Token Prediction)：显著加速模型解码速度 。
8. 典型场景：
```
延迟不敏感：例如文档摘要场景，可以设置尽可能大的并发性以获得最高的系统吞吐量和硬件利用率，而不受 TTFT 和 TOPT 限制 。
延迟敏感：例如聊天机器人场景，要求在线服务满足 TTFT 和 TOPT (TPS/user) 阈值。这意味着在给定 TTFT 和 TOPT (TPS/user) 要求下，需要使用最大并发性来获得最高输出吞吐量和硬件利用率 。典型的 TPS/user 为 20 tokens/s/user 和 10 tokens/s/user 。
```
9. 设置 (Setup)：
- 硬件：GPU 8xH20(141G)，CPU Intel(R) Xeon(R) Platinum 8468V，系统内存 2T，8x400G IB (E-W)，2x200GE (N-S) 。
- 软件：最新版本的 TensorRT-LLM torch flow, SGLang, vLLM 。
- 测试用例：比较 IFB 性能，不包括静态批处理和 PD 解耦 。只测试 ISL/OSL:1000/1000。并发选择 8/64/2048，覆盖单节点和双节点，使用 Pareto 曲线比较性能 。
10. 性能比较 (Performance Comparison)：
- TRTLLM/Sglang/vllm 都使用 launch server + 
- sglang.bench_serving 进行测试 。
- 性能对比：TRTLLM(w4a8) > Sglang(fp8) > vllm(fp8) 。
- 单节点会获得更好的 TPS/GPU，双节点会加速 TTFT 和 TPOT 但会引入更多通信开销 。
- SGLang v0.4 相比 SGLang v0.3 实现了 1.9 倍的解码吞吐量提升 。
- 在 NVIDIA Hopper 上，SGLang Disaggregation 和大规模专家并行化在 4 个月内实现了 26 倍的推理吞吐量提升 。
- 对于 2000-token 输入序列，每节点性能：52.3k 输入 toks/s，22.3k 输出 toks/s 