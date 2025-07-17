## 高性能计算 (HPC) HOOMD-blue 性能优化学习笔记
### 第一部分：HPC 基础与 MPI 应用程序执行
#### 1.1 HPC 简介与访问方式
高性能计算 (HPC) 中心提供了强大的计算资源，通常通过互联网访问。用户通过用户账户登录到登录节点/前端。通过 SSH 连接（可以使用网页浏览器中的 Jupyter Notebook、基于 Web 的任务或本地机器上的终端）连接到登录节点。作业会通过作业调度器（如 Slurm、PBS）提交，由其负责资源分配和队列管理，最终在计算和 GPU 集群上执行。HPC 系统还包括存储系统，用于存储用户数据和系统文件。

#### 1.2 MPI 应用程序在 HPC 集群上的执行原理
MPI (Message Passing Interface) 是一种用于并行计算的通信协议。程序员通过编写 MPI 程序并使用相关库来实现并行化。MPI 应用程序在 HPC 集群上的执行流程通常涉及以下几个关键环节：
- 估算资源和编写作业脚本: 程序员根据程序需求估算所需资源，并编写包含作业调度器指令的脚本。
- 作业调度器: 负责资源的分配和管理，将程序分发到不同的计算节点上。
- MPI 运行时: 负责进程管理和通信层的建立，例如 OpenMPI 或 MPICH。
- 工作负载 (MPI 程序): 包含应用程序的并行算法和对 MPI API 的调用。
- 网络 (互联): 高速网络（如 Infiniband、Ethernet）用于实现节点间的高效通信。
#### 1.3 典型的 MPI 应用程序执行时间线
MPI 应用程序的执行通常分为六个主要阶段:
1. 作业设置 (Phase 1: Job Setup): 包括作业提交、资源分配、节点分配、环境设置和 MPI 运行时启动。
2. MPI_Init() (Phase 2: MPI_Init()): 进程创建、MPI_COMM_WORLD 设置、通信结构初始化和内存注册。
3. 应用程序逻辑 (Phase 3: App Logic): 获取输入和大小、数据分布、局部计算和通信模式的确定。
4. 通信 (Phase 4: Communication): 包括点对点通信、集合通信、同步、数据交换和屏障操作。
5. 并行 I/O (Phase 5: Parallel I/O): 文件打开、数据收集、数据聚合、文件同步和文件关闭。
6. 清理 (Phase 6: Cleanup): MPI_Finalize、进程清理、作业完成和节点释放。

在节点间，进程流呈现出数据在节点间传递、计算、同步和I/O操作的详细过程，通过高速网络互联。关键的 MPI API 调用贯穿整个生命周期，包括初始化、通信、同步、并行 I/O 和清理等方面的函数调用。

### 第二部分：HPC 平台基础设施与 HOOMD-blue 案例研究
本节将介绍 GADI 和 ASPIRE2A 这两个 HPC 平台的具体基础设施，以及 HOOMD-blue 在其上的案例研究。
#### 2.1 GADI 和 ASPIRE2A 基础设施概述
|特性	|GADI|	ASPIRE2A|
|------|------|------|
|操作系统 (OS)|	Rocky Linux 8.10 (Green Obsidian)，内核版本 4.18.0-553.8.1.el8.x86_64	|Red Hat Enterprise Linux 8.4 (Ootpa)，内核版本 4.18.0-305.25.1.el8_4.x86_64|
|CPU 信息	|架构: x86_64，96 个 CPU，24 核心/插槽，2 个插槽，Intel(R) Xeon(R) Platinum 8274 CPU @ 3.20GHz|	架构: x86_64，128 个 CPU，64 核心/插槽，2 个插槽，AMD EPYC-Milan Processor @ 1996.249 MHz|
|内存信息|	总内存: 188 Gi，已用: 77 Gi，空闲: 26 Gi，共享: 0 Gi	|总内存: 125 Gi，已用: 7.3 Gi，空闲: 38 Gi，共享: 6.6 Gi|
|单节点架构|	2 个 Socket (Socket 0, Socket 1)，每个 Socket 包含 2 个 NUMA 节点 (NUMA 0-3)，通过 UPI 互联。	|2 个 Socket (Socket 0, Socket 1)，每个 Socket 包含 AMD EPYC-Milan 处理器，核心范围 0-63 (Socket 0) 和 64-127 (Socket 1)。|

#### 2.2 HOOMD-blue 案例研究：运行与配置
本案例研究关注 HOOMD-blue 的 md_pair_wca 模型，使用 200,000 个粒子输入数据 (hard_sphere_200000_1.0_3.gsd)，并进行强缩放 (1-8 节点) 和弱缩放 (1-32 节点) 分析。

编译器和 MPI 库配置：

|平台	|编译器	|MPI	|库	|架构	|操作系统|
|------|------|------|------|------|------|
|GADI	|gcc-14.1.0 / Intel-2021.10.0	|OpenMPI-4.1.5 / Intel MPI-2021.13.0	|Eigen-3.4.0, Cereal-1.3.2, Pybind11-2.13.5, Intel MKL-2024.2.0, Intel TBB-2021.13.0	|X86_64	|Linux|
|ASPIRE-2A	|gcc 11.2.0 (C, C++)	|openmpi/4.1.2-hpe	|Cereal, Pybind11, Eigen3|	X86_64	Linux|

- 在 GADI 上运行 HOOMD-blue：
1. 使用 IntelMPI：示例 PBS 脚本展示了如何加载 intel-mpi/2021.13.0 和 intel-compiler/2021.10.0 模块，设置 OMP_NUM_THREADS=1，并通过 mpirun 启动 HOOMD-blue 基准测试。输出显示，使用 48 核时，每秒时间步长约为 412.06。
2. 使用 OpenMPI：示例 PBS 脚本展示了如何加载 openmpi/4.1.5 模块，并通过 mpirun 启动基准测试。输出显示，每秒时间步长约为 417.96。
- 在 ASPIRE2A 上运行 HOOMD-blue：
1. 使用 IntelMPI：示例 PBS 脚本展示了加载 intel-oneapi/2024.0、op enmpi/4.1.2-hpe 等模块，并配置 mpirun 参数。输出显示每秒时间步长约为 939.11。
2. 使用 OpenMPI：示例 PBS 脚本类似地展示了加载 openmpi/4.1.2-hpe 模块和 mpirun 配置。输出显示每秒时间步长约为 1106.76。
### 第三部分：性能评估与优化工具
#### 3.1 性能评估的必要工具与方法
性能评估需要正确运行代码（健全性、活性、从源代码构建），并衡量程序和系统的性能（获取运行时资源报告、性能分析或跟踪）。
- 选择“指标”: 测量（计数、持续时间、大小）和导出（速率、吞吐量），并推导理论峰值。
- 性能热点: 识别主导执行时间的函数，量化其影响，并分析是否存在合理原因。
- 问题: 常见问题包括负载不平衡、开销，以及需要进行强缩放和弱缩放分析。
- 理想情况: 线性加速，这有助于优化作业提交策略和资源请求，并指导扩展到更大系统的决策。
### 3.2 执行分析工具对比
下表对比了常用的执行分析工具及其特点：
|特性	|Intel Advisor	|Intel VTune	|ARM Forge	|Score-P*	|Extrae*|
|------|------|------|------|------|------|
|MPI/OpenMP 可扩展性	|No|	Limited|	Yes (详细分析)|	Yes (详细跟踪和分析)|	Yes (详细跟踪，尤其是在 MPI 环境中)|
|细粒度事件跟踪	|No	|Limited (跟踪模式)	|Yes	|Yes|	Yes|
|Roofline 分析|	Yes|	No|	No|	No|	No|
|向量化分析|	Yes (详细 SIMD 分析)|	Limited|	Yes (基本 SIMD 分析)|	Limited (侧重于整体性能，而非 SIMD 详细信息)|	Limited (缺乏详细 SIMD 分析)|
|线程分析|	Yes (并行建议)	|Yes (线程争用)|	Yes (通用)|	Yes (基本线程分析与并行区域)	|Yes (基本支持线程级跟踪)|
|微架构探索|	Limited	Yes (深度分析)|	Limited|	Limited (主要高级性能数据)|	Limited (不侧重于深度微架构分析)|
* 尝试过但未按预期工作。
#### 3.3 GADI 单节点性能报告 (使用 ARM Profiler)
使用 ARM Profiler 确认了 Intel 编译器 + Intel MPI (未优化) 在 GADI 单节点上的性能状况：
- CPU: 占 CPU 时间的 69.1% (89.3s)。其中标量数值操作占 19.6% (17.5s)，向量数值操作占 0.0% (0.0s)，内存访问占 80.4% (71.8s)。
- MPI: 占 MPI 时间的 29.8%。集体调用时间占 60.0%，点对点调用时间占 40.0%。有效进程集体速率 798 KB/s，有效进程点对点速率 949 MB/s。MPI 通信耗时 29.8%，标量操作表示分支指令。
- I/O: 占 I/O 时间的 0.3%。读操作时间占 67.2%，写操作时间占 32.8%。有效进程读速率 16.4 MB/s，有效进程写速率 21.5 bytes/s。
- 内存: 平均进程内存使用量 299 MiB，峰值进程内存使用量 435 MiB，峰值节点内存使用量 23.0%。
#### 3.4 确认改进的 ARM 性能报告 (多节点)
对多节点性能的报告显示了优化后的数据，例如 MPI 时间占比为 26.4% (1.1s)。
#### 3.5 Intel Vtune 性能测量与热点分析
在 GADI 单节点上，使用 Intel 编译器和 Intel MPI 构建的 HOOMD-blue（未优化），通过 Intel Vtune 进行了性能测量：
- 已用时间: 252.250s，CPU 时间: 11400.001s。
- 微架构使用率: 35.8% 的流水线槽，提示平台效率过低，可能原因包括内存停顿、指令饥饿等。
- CPI 率: 0.988，总线程数: 97。
- 有效物理核心利用率: 100.0% (25个中的25个)，有效逻辑核心利用率: 47.1% (96个中的45.195个)。较低的逻辑核心利用率可能表明物理核心利用率较差。

主要热点分析：

发现了三个主要热点：
1. MPI ([libmpi.so.12.0.0]): 51.5% 的 CPU 时间 (5873.704s)。
2. MD 模块 ([_md.cpython-312-x86_64-linux-gnu.so]): 23.1% 的 CPU 时间 (2628.140s)。
3. HoOMD ([_hoomd.cpython-312-x86_64-linux-gnu.so]): 16.7% 的 CPU 时间 (1904.161s)。

- 计算需求最高的库 (微架构使用率)：
1. 四个库显示 100% 的 CPU 效率：libucm.so.0.0.0、_multiarray_urmath.cpython-312-x86_64-linux-gnu.so、libucp.so.0.0.0 和 libuct_lb.so.0.0.0。其中三个用于通信，一个用于计算。
2. 计算指令 vs. 通信指令：
_hoomd.cpython-312-x86_64-linux-gnu.so 和 _md.cpython-312-x86_64-linux-gnu.so 具有非常高的指令数（10^12级别），表明它们是计算密集型模块。libmpi.so.12.0.0、libmpifort.so.12.0.0、libucp.so.0.0.0 和 libuct_lb.so.0.0.0 等与通信相关的库也具有较高的指令数。
#### 3.6 Roofline 分析：识别性能机会 (GADI 单节点)
Intel Advisor 的 Roofline 分析显示，性能受计算和内存带宽的限制。图表中红色圆圈内的点表示存在内存局部性问题。

### 第四部分：性能优化实践与结果
#### 4.1 最佳 HOOMD-blue 构建配置
在 GADI 上，最佳的 HOOMD-blue 构建配置是使用 Intel 编译器和 Intel MPI，并应用以下优化标志：-O3 -xCORE-AVX512 -fma -mavx512f -qtbb -qmkl=parallel -fp-model precise。此外，发现单节点的最佳 OpenMP 线程数为 OMP_NUM_THREADS=2。

#### 4.2 ASPIRE2A 上的编译器优化选项
在 ASPIRE2A 上，针对 GNU 和 Intel 编译器都有具体的优化选项。
- GNU: 主要使用 -O3 优化级别，并启用针对机器架构、FMA、AVX2 和循环预取等优化。
- Intel: 除了 -O3 和类似 GNU 的优化外，还包括 -ffast-math (可能牺牲精度以提高速度)、-funroll-loops (循环展开)、-fomit-frame-pointer (省略帧指针)、-fno-strict-aliasing (禁用严格别名)、-ftree-vectorize (矢量化优化) 和 -qmkl=parallel (链接 Intel MKL 并行模式)。
#### 4.3 性能优化实践成果
- MPI 调优: 优化了 MPI 库以支持多线程，启用了 MPI 自动调优，激活了进程绑定到核心，配置通信以使用同一节点内的共享内存和用于节点间通信的 OFI，设置了每个节点的最大 MPI 进程数为 48，并将每个 MPI 进程配置为使用 2 个线程。
- 编译器调优: 应用了优化标志以在 Intel CPU 上获得 AVX-512 的最大性能，启用了线程构建块 (TBB)，并使用融合乘加 (-fma) 以提高浮点性能。
- 内存局部性: 强调了内存局部性的重要性，并指出可以通过将进程绑定到特定核心等 MPI 调优来优化。
#### 4.4 单节点性能对比 (GADI)
图表显示了在 GADI 上，不同编译器和环境配置对每秒时间步长的影响。
- Intel - conda - O3 - xCORE-AVX512 - fma - mavx512f - qtbb - qmkl=parallel -fp-model precise 配置在 OMP_NUM_THREADS=2 时显示出最佳性能。
- 在 GNU 编译器 + OpenMPI (未优化) 的单节点测试中，OMP_NUM_THREADS=2 达到了约 426.31 每秒时间步长，而增加到 4 或 8 线程则导致性能显著下降。
#### 4.5 强缩放和弱缩放性能分析 (GADI)
- 强缩放分析: 将资源从 1 个节点增加到 8 个节点时，HOOMD-blue 的 MD Pair WCA 模型显示出几乎线性的加速比，无论是使用 GNU OpenMPI 还是 Intel IntelMPI。优化后的 Intel 编译器 + Intel MPI 性能略优于基准性能。
- 弱缩放分析: 在 GADI 上，随着节点数增加，优化后的 IntelMPI 配置 (带所有优化标志) 在每秒时间步长上表现最佳，并且非常接近理想的线性缩放。
#### 4.6 结果总结与基准对比
通过优化，在 GADI 上我们实现了平均约 7% 的加速比。例如，在 32 个节点上，优化后的每秒时间步长达到 6350，高于基准的 5847。虽然在某些节点数（如 8 个节点）我们的结果略低于基准，但在 4 个节点和 32 个节点上，我们取得了显著的加速比 (分别为 12.95% 和 8.6%)。

在 ASPIRE-2A 上，优化后的性能也有所提升，例如在 8 个节点上，加速比达到 8.98%。

#### 4.7 ASPIRE2A 上的基准和多节点性能
- 基准性能: ASPIRE2A 上 OpenMPI 和 GNU 编译器的基准性能随节点数增加而提高，但从 8 个节点开始，性能增益趋于平缓。
- 多节点性能: 优化后的 Intel 配置在 ASPIRE2A 多节点上表现最佳，尤其是在 32 个节点时达到 4,110.46 每秒时间步长。
### 第五部分：结论与未来展望
- HOOMD-blue 使用域分解进行数据分区，增加 MPI 进程数通常能带来更好的性能。
- md_pair_wca 计算在使用每个节点上物理和虚拟核心上的完整 MPI 进程时，性能会受到影响。
- md_pair_wca 是一项计算和通信密集型任务，内存局部性对其性能至关重要。
- 在使用 BSC 工具 Extrae 分析 HOOMD-blue（一个 Python 模块）时遇到了挑战，未能成功