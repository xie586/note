## 1. 概述 
## 归约操作 (Reduction operation) 
- 直方图 (Histogram) 
- 直方图的并行算法 
- 分段分区 (Sectioned partitioning) 
- 交错分区 (Interleaved partitioning) 
- CUDA中的原子操作 (Atomic operations in CUDA) 
- 基本文本直方图内核中的原子操作 
- DRAM和共享内存上的原子操作 
- 私有化 (Privatisation) 
- 用于直方图的共享内存原子操作 
- 回顾与开发者工具 
## 2. 归约操作 (Reduction Operation)
归约操作是将一组输入值总结为一个值的过程 。常见的归约操作包括：

1. 最大值 (Max) 
2. 最小值 (Min) 
3. 求和 (Sum) 
4. 乘积 (Product) 
归约操作通常与用户定义的归约函数一起使用，只要该操作满足以下条件：

1. 具有结合律和交换律 
2. 具有明确定义的恒等值 
- 例子: 用户可以提供一个自定义的“最大值”函数，用于三维坐标数据集，其中每个坐标数据元组的量级是距原点的距离 。

## 3. 分区和总结 (Partition and Summarize) 
这是一种常用的处理大型输入数据集的策略 。

- 数据集中的元素处理顺序没有要求 (结合律和交换律) 。
- 将数据集分成更小的块 。
- 让每个线程处理一个块 。
- 使用归约树将每个块的结果汇总到最终答案中 。
### 并行归约树算法 ：

执行 N−1 次操作，在 log(N) 步内完成 。
## 4. 并行求和归约 (Parallel Sum Reduction) 
- 并行实现：递归地将线程数量减半，每一步每个线程添加两个值 。
- 对于 n 个元素，需要 log(n) 步，需要 n/2 个线程 。
- 原地归约 (in-place reduction)：假设使用共享内存进行原地归约 。 
1. 原始向量在设备全局内存中 。
2. 共享内存用于保存部分和向量 。
3. 每一步都会使部分和向量更接近总和 。
4. 最终的总和将位于部分和向量的第0个元素中 。
5. 减少了由于部分和值产生的全局内存流量 。
### 朴素的线程到数据映射 ：

- 每个线程负责部分和向量的偶数索引位置 (责任位置) 。
- 每一步之后，一半的线程不再需要 。
- 其中一个输入总是来自责任位置 。
- 每一步中，一个输入来自越来越远的距离 。
### 简单的线程块设计 ：

1. 每个线程块接收 2×BlockDim.x 个输入元素 。
2. 每个线程将2个元素加载到共享内存中 。
```C++
shared float partialSum[2*blockDim.x]; 
unsigned int t = threadIdx.x; 
unsigned int start = 2*blockDim.x*blockIdx.x;
unsigned int j = 2*t; 
partialSum[j] = input[start + j]; 
partialSum[j+1] = input[start + j+1]; 
```
- 内存访问不合并 (Memory access is not coalesced) 。
### 更好的内存合并方式 ：

```C++
partialSum[t] = input[start + t]; 
partialSum[blockDim+t] = input[start + blockDim.x+t]; 
for (unsigned int stride = 1; stride <= blockDim.x; stride *= 2) 
{
 __syncthreads(); 
 if (t % stride == 0) 
  partialSum[2*t]+= partialSum[2*t+stride]; 
__syncthreads() 是必需的，以确保在进入下一步之前，每个版本的部分和的所有元素都已生成 
```
### 控制分支 (Control Divergence) ：

- 当warp中的线程通过做出不同的控制决策而采用不同的控制流路径时，就会发生控制分支 。 
1. 有些线程采用then-path，另一些线程采用else-path 。
2. 有些线程的循环迭代次数与其它线程不同 。
- 在当前的GPU中，采用不同路径的线程执行是串行化的 。
- 在执行每条路径时，所有采用该路径的线程将并行执行 。
- 当考虑嵌套控制流语句时，不同路径的数量可能很大 。
### 对朴素归约内核的一些观察 ：

- 在每次迭代中，每个warp将顺序遍历两条控制流路径 。 
- 执行加法的线程和不执行加法的线程 。
- 不执行加法的线程仍然消耗执行资源 。
### 线程索引使用很重要 (Thread Index Usage Matters) ：

- 在某些算法中，可以通过改变索引使用来改善分支行为 。 
- 对于可交换和结合的运算符 。
- 保持活动线程的连续性 。
- 始终将部分和压缩到 partialSum[] 数组的前面位置 。
### 一个更好的归约内核 ：

```C++

for (unsigned int stride = blockDim.x; [cite: 103]
 stride > 0; stride /= 2) [cite: 103]
{
 __syncthreads(); [cite: 103]
 if (t < stride) [cite: 104]
  partialSum[t] += partialSum[t+stride]; [cite: 104]
}
```
### 快速分析 ：

- 对于一个1024线程块：
1. 前5步没有分支 。
2. 在每一步中，1024、512、256、128、64、32个连续的线程是活跃的 。
3. 每个warp中的所有线程要么全部活跃，要么全部不活跃 。
4. 只有在最后5步中，第一个warp仍然会有分支 。
### 回到全局图 ：

- 在内核结束时，每个线程块中的线程0将其线程块的总和 partialSum[0] 写-入一个由 blockIdx.x 索引的向量中 。
- 如果原始向量非常大，则会有大量这样的总和 。
- 可以进行另一次迭代 。
- 如果总和数量很少，主机可以直接将数据传回并加在一起 。
## 5. 直方图 (Histogram) 
- 直方图是一种从大型数据集中提取显著特征和模式的方法 。 
- 图像中对象识别的特征提取 。
- 信用卡交易中的欺诈检测 。
- 天体物理学中天体运动的关联 。
- ...
- 基本直方图：对于数据集中的每个元素，使用其值来标识一个“bin计数器”并递增 。
### 文本直方图示例 ：

- 将bin定义为字母表的四字母部分：a–d、e–h、I–l、n–p等 。
- 对于输入字符串中的每个字符，递增相应的bin计数器 。
## 6. 简单的并行直方图算法 (A Simple Parallel Histogram Algorithm) 
- 将输入分成几段 。
- 让每个线程处理输入的一部分 。
- 每个线程遍历其部分 。
- 对于每个字母，递增相应的bin计数器 。
### 输入分区影响内存访问效率 ：

- 分段分区 (Sectioned partitioning) ： 
1. 导致内存访问效率低下 。

2. 相邻线程不访问相邻内存位置 。

3. 访问不合并 。

4. DRAM带宽利用率低 。

4. 注意：为了合并，我们需要一个指令在线程之间具有局部性，而不是一个线程在后续指令之间具有局部性 。
### 交错分区 (Interleaved partitioning) ： 

- 改为交错分区 。
- 所有线程处理连续的元素部分 。
- 它们都移动到下一部分并重复 。
- 内存访问是合并的 。
## 7. 文本直方图中的读-修改-写 (Read, Modify, Write in the Text Histogram) 
- 多个线程尝试同时访问和修改同一数据位置会导致数据竞争问题 。
## 8. 原子操作的关键概念 (Key Concepts of Atomic Operations) 
- 原子操作 (atomic operation) 是由单个硬件指令对内存位置地址执行的读-修改-写操作 。 
1. 读取旧值，计算新值，并将新值写入该位置 。
2. 硬件确保在当前原子操作完成之前，没有其他线程可以对同一位置执行另一个读-修改-写操作 。
3. 任何尝试对同一位置执行原子操作的其他线程通常会被放入队列中 。
4. 所有线程在同一位置串行执行其原子操作 。
### CUDA中的原子操作 ：

- 通过调用转换为单条指令的函数来执行 (也称为 intrinsic functions 或 intrinsics) 。
- 原子加 (add)、减 (sub)、增 (inc)、减 (dec)、最小 (min)、最大 (max)、交换 (exch)、比较和交换 (CAS) 。
- atomicAdd：int atomicAdd(int* address, int val); 
1. 从 address 指向的全局或共享内存位置读取32位字 old，计算 (old + val)，并将结果存储回同一地址的内存中 。
2. 函数返回 old 。
- 更多 atomicAdd 类型：
1. 无符号32位整数原子加：unsigned int atomicAdd(unsigned int* address, unsigned int val); 
2. 无符号64位整数原子加：unsigned long long int atomicAdd(unsigned long long int* address, unsigned long long int val); 
3. 单精度浮点原子加 (计算能力 > 2.0)：float atomicAdd(float* address, float val); 
## 9. 基本文本直方图内核 (A Basic Text Histogram Kernel) 
- 内核接收指向字节值输入缓冲区的指针 。
- 每个线程以跨步模式处理输入 。
## 10. 全局内存 (DRAM) 上的原子操作 (Atomic Operations on Global Memory (DRAM)) 
- DRAM位置上的原子操作以读取开始，其延迟为几百个周期 。
- 原子操作以写入同一位置结束，其延迟为几百个周期 。
- 在此期间，没有人可以访问该位置 。
- 每次读-修改-写操作都有两次完整的内存访问延迟 。
- 同一变量 (DRAM位置) 上的所有原子操作都是串行化的 。
### 延迟决定吞吐量 (Latency Determines Throughput) ：

- 在同一DRAM位置上的原子操作吞吐量是应用程序可以执行原子操作的速率 。
- 特定位置的原子操作速率受读-修改-写序列总延迟的限制，对于全局内存 (DRAM) 位置通常超过1,000个周期 。
- 这意味着，如果许多线程尝试在同一位置进行原子操作 (争用)，内存吞吐量将降低到单个内存通道峰值带宽的1/1,000以下！ 
## 11. 硬件改进 (Hardware Improvements)
### Fermi L2缓存上的原子操作 ： 
1. 中等延迟，约为DRAM延迟的1/10 。
2. 所有块共享 。
3. 全局内存原子操作的“免费改进” 。
### 共享内存上的原子操作 ： 
1. 非常短的延迟 。
2. 每个线程块私有 。
3. 需要程序员进行算法工作 。
## 12. 私有化 (Privatisation) 
1. 私有化是一种强大且常用的并行应用程序技术 。
2. 操作需要具有结合律和交换律 。
3. 直方图加法操作是结合律和交换律的 。
4. 如果操作不符合要求，则不能私有化 。
## 13. 用于直方图的共享内存原子操作 (Shared Memory Atomics for Histogram) 
- 吞吐量远高于DRAM (100倍) 或L2 (10倍) 原子操作 。
- 更少的争用：只有同一块中的线程才能访问共享内存变量 。
- 这是共享内存的一个非常重要的用例 。
### 共享内存原子操作需要私有化 ：


- 为每个线程块创建 histo[] 数组的私有副本 。

```C++

__global__ void histo_kernel(unsigned char *buffer,
long size, unsigned int *histo) 
{
 __shared__ unsigned int histo_private[7]; 
 if (threadIdx.x < 7) histo_private[threadIdx.x] = 0; 
 __syncthreads();  // 初始化私有副本中的bin计数器
 // ... (构建私有直方图的循环)
 int i = threadIdx.x + blockIdx.x * blockDim.x; 
 // stride is total number of threads
 int stride = blockDim.x * gridDim.x; 
 while (i < size) { 
  int alphabet_position = buffer[i] – “a”; 
  if (alphabet_position >= 0 && alphabet_position < 26) 
   atomicAdd(&(histo_private[alphabet_position/4]), 1); 
  i += stride; 
 }
 __syncthreads();  // 等待块中所有其他线程完成
 if (threadIdx.x < 7) { 
  atomicAdd(&(histo[threadIdx.x]), histo_private[threadIdx.x] );
 }
}
```