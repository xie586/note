## 1. NWChem 简介 
- 定义: NWChem 是一款高性能计算化学软件套件 。
- 功能: 它提供广泛的计算化学功能，包括从经典分子动力学 (MD) 到原子轨道密度泛函理论 (AO DFT)，以及从 MP2 到 CCSD 等多种方法 。
- 特点:
```
多尺度: 支持 QM/MM (量子力学/分子力学) 和嵌入式计算 。
NWPW: 一个基于 MPI 的从头算分子动力学 (AIMD) 代码 。
并行设计: 针对 HPC 系统的大规模并行设计，大约始于 2000 年 。
Global Arrays 中的进程级并行: 采用进程级并行 。
模块化设计: 模块化设计便于积分、SCF (自洽场) 等的重用 。
面向对象: 采用遗留 Fortran 语言的面向对象设计 。
线程化: 最初通过 BLAS/LAPACK 实现线程化 。
```
## 2. NWChem 软件架构 
- 系统接口:
1. 基本 I/O (例如命令行参数) 。
2. 计时器和许多其他操作系统封装器 。
- 内存分配:
1. 快速 (无系统调用，堆栈模式) 。
2. 共享内存 (IPC) 。
3. 必要时为网络固定 。
- Global Arrays (GA) 和 TCGMSG:
```
进程管理 (作业启动) 。
单边通信 。
集合操作 。
负载均衡器 。
```
- Generic Tasks:
```
SCF 能量、梯度等 。
DFT 能量、梯度等 。
MD、NMR、溶剂化等 。
优化、动力学等 。
```
- Run-time Database (RTDB):
1. 几何对象 。
2. 基组对象 。
- Integral API 。
- PeIGS 。
- Molecular Modeling Toolkit 。
- Parallel IO (并行 I/O) 。
- Memory Allocator (内存分配器) 。
- Global Arrays 。
- Molecular Software Development Toolkit 。
## 3. Global Arrays (GA) 
- 定义: 分布式数组抽象层，支持通信原语和数值线性代数方法 。
- 特点:
```
用户观察到具有强大异步进度的单边通信 。
ARMCI 是 GA 内部的单边通信抽象层 。
GA 维护一个转换表，将数组句柄和全局索引映射到排名和基指针 。
单个 N 维 GA 操作可能需要与N^2或更多远程排名通信 。
对于N>1 维数组，最可能的情况是每个目标都是非连续子数组 (即向量的向量) 。
```
## 4. ARMCI 到 MPI 的映射 
- ARMCI / ComEx: 基于 Send-Recv (发送-接收) 的 MPI 语义实现 。
异步进度机制包括 Spawn process (生成进程)、Split process (拆分进程) 和 Spawn thread (生成线程) 。
- ARMCI-MPI: 基于 RMA (One-Sided) (远程内存访问/单边通信) 的 MPI 语义实现 。
- 异步进度机制位于 MPI 库内部，包括 Thread (线程)、Other* (其他，例如 Blue Gene 上的中断，硬件卸载机制如智能网卡) 和 None (无) 。
- Casper 是一种 Split process (拆分进程) 机制 。
## 5. NWChem 评估 
- 基准测试系统: 
```
1hsg_28 。
122 个原子，1159 个基函数 。
H、C、N、O 元素，采用 cc-pVDZ 基组 。
半直接算法 。
闭壳层 (RHF) 。
```
- 相关研究: E. Chow 等人发表的 “Scaling up Hartree-Fock Calculations on Tianhe-2” 。
- GTFock 在这些拍级规模的运行中使用了 GA/ARMCI-MPI 和 MPICH-Glex 。
## 6. NWChem SCF 性能 (新) 
- 在8个节点上运行，每个节点 24 ppn (processes per node)，通信均使用 2 ppn。
- NWChem 6.3/ARMCI-MPI3/Casper: 迭代 6 次，时间为 128.0 。
- NWChem Dev/ARMCI-MPIPR (NERSC 2015 年 9 月构建): 迭代 6 次，时间为 125.7 。
## 9. SPEC CCSD(T) 库在 NERSC Cori 超级计算机 KNL 节点上的扩展性 
- 科学成就: 计算并苯二聚体 (一种石墨烯的典型系统) 的结合能 。
- 重要性和影响: 能够获得大型系统的精确相互作用能；迄今为止最大的 CCSD(T) 计算 (9.14 PFLOPS)，使用了 538,650 个 Knight's Landing (KNL) 核心 (9,450 个节点；每个节点 57/68 个核心) 。
- 研究细节:
```
216 个电子，1776 个基函数 (cc-pVTZ 基组) 。
OpenMP 用于 CCSD 和 CCSD(T) 中的多线程 。
CCSD 中的检查点重启功能 。
使用 Global Arrays (GA) 工具改进了 KNL 节点上 (T) 修正的节点间并行性 。
```
## 10. NWChem 软件要求 
- CPU: 确定支持 x86_64、AArch64、PowerPC，其他 CPU 也应能工作 。
- GPU: 通过 CUDA 和 OpenACC 支持 NVIDIA GPU 。
- 支持 OpenMP 4 卸载 。
- 操作系统: 所有类 Linux 系统 (Linux、MacOS、BSD) 。
- 编译器: 经过广泛测试的有 GCC、Intel、NVHPC (PGI)，其他编译器也应能工作 。
- 数学库: Netlib、MKL、OpenBLAS、BLIS、ARMPL、Apple Accelerate 等 。
- 通信: 假定使用 MPI 。
```
MPI-PR 适用于所有已知的 MPI 库 。
ARMCI-MPI 与旧版 Open-MPI 不安全，但 v5.0+ 版本非常好 。
基于 MPICH 的 (Intel、MVAPICH2) 应始终可用 (最近的 bug 已超过 5 年) 。
```
## 11. 性能变量 
- 编译器优化: 主要针对原子积分计算 。
- 数学库: BLAS、LAPACK、math.h/libm 。
- GA/ARMCI/MPI:
1. ARMCI 与 ARMCI-MPI 的设计 。
2. MPICH 与 Open-MPI RMA 的设计 。
- Linux 中的进程间 I/O 扩展性 。
## 12. 实验设置 
- 硬件: 双插槽 Ampere Altra Q80-30 (80 核心，最大 3.0 GHz) 。
- 编译器和数学库:
1. GCC 8.3.1 和最新版 OpenBLAS 。
2. NVHPC 21.5 和附带的 OpenBLAS 。
- MPI:
1. 最新版 MPICH 。
2. 最新版 Open-MPI 。
- GA/ARMCI:
1. MPI-PR 。
2. ARMCI-MPI 。
- 模拟类型: B3LYP/cc-pVTZ，这是一种常见的分子模拟类型，主要由原子积分计算、交换关联正交和少量密集线性代数主导 。
- 特点: 这些模拟是计算密集型的，除非出现问题 。
## 13. 实验结果 ((H2O)21 B3LYP/cc-pVTZ) 
- 壁钟时间: 以秒为单位。
- 条形图: 降序排列的条形图分别表示 10、20、40、80、160 个进程的每个二进制文件。
- 图例:
```
G=GCC 
N=NVHPC 
A=ARMCI-MPI 
P=MPI-PR 
M=MPICH 
O=Open-MPI 
```
## 14. 性能分析 
- NVHPC 二进制文件：比 GCC 二进制文件快约 2-7% 。这与 DFT 代码中观察到的编译器影响一致 。GCC 8 可能缺少 ARM Neoverse N1 核心的重要优化 。
- Open-MPI 二进制文件：比 MPICH 二进制文件快约 0-20% 。Open-MPI 在 RMA 中似乎具有更好的共享内存支持，但未探索所有调优参数 (例如 MPICH 使用 OFI 而非 UCX) 。
- ARMCI-MPI 与 MPI-PR：性能相差 ±10% 。
```
MPI-PR 使用的核数少 1 个，这在低核数 (例如 10 个) 时很重要 。
由于进度机制，MPI-PR 在大多数情况下相对于 ARMCI-MPI 具有更好的扩展性 。
带有 Casper 的 ARMCI-MPI (此处未测试) 也会分配 1 个或更多核心用于进度 。
```
## 15. NWChem DFT 基准测试 (W21 B3LYP/cc-pVTZ) 
- 图表显示了 Xeon、Epyc 和 Altra 处理器在不同进程数 (nproc) 下的壁钟时间 (wall time)。
- 显示 Altra 在较高进程数时性能相对稳定。