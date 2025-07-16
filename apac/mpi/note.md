## 高性能计算 (HPC) 中的并行编程：消息传递接口 (MPI) 学习笔记
### 1. 并行计算基础
#### 1.1 处理单元 (Processing Elements - PE)
- 理解并行计算的硬件基础至关重要：
- 服务器/节点/机器：
```
Socket/Package (插槽/封装)： 通常指主板上的 CPU 插槽，每个插槽可以安装一个物理 CPU。
Core/CPU (核心/中央处理器)： 每个物理 CPU 包含多个核心。
Thread/PU (线程/处理单元)： 每个核心可以支持多个硬件线程（如Intel的超线程技术）。
```
- 集群 (Cluster)：
```
由多台服务器/节点组成。
通过 互连 (Interconnect) (例如 InfiniBand, Ethernet) 连接，实现节点间的高速数据传输。
```
- 其他处理单元 (Other PE)：
```
GPU (Graphics Processing Unit)： 图形处理器，擅长并行处理大量简单数据。
FPGA (Field Programmable Gate Arrays)： 现场可编程门阵列，可根据特定应用定制硬件逻辑。
ASIC (Application-specific integrated circuit)： 专用集成电路，为特定应用高度优化。
```
#### 1.2 并行编程模型 (Parallel Programming Models)
不同的并行编程模型适用于不同的硬件架构和编程需求：
- 共享内存 (Shared Memory)：
```
所有处理单元共享同一个内存空间。
pthreads： POSIX 线程，低级线程库。
OpenMP： 一套用于多处理器编程的 API，支持共享内存并行。
SHMEM (Shared Memory Access)： 提供直接内存访问的编程模型。
```
- 分布式内存 (Distributed Memory)：
```
每个处理单元拥有独立的内存空间，通过消息传递进行通信。
Message Passing Interface (MPI)： 本笔记的重点，标准化了分布式内存并行编程的消息传递模型。
```
- 数据并行 (Data Parallel)：
```
通常涉及将数据划分为多个部分，每个处理单元处理一个部分。
Partitioned Global Address Space (PGAS)： 分区全局地址空间，结合了共享内存和分布式内存的特点，允许对远程内存进行直接访问。
```
### 2. 消息传递接口 (MPI) 概述
#### 2.1 什么是 MPI？
- MPI (Message Passing Interface) 是一种标准化了消息传递模型的接口，是并行编程中最常用的接口。
- MPI 标准由 MPI 论坛（一个由供应商、研究人员、开发人员和用户组成的集体）设计和维护。
- MPI 官方文档： http://mpi-forum.org/docs/
- MPI 实现： 市场上存在多种 MPI 实现，它们由不同的供应商提供，例如：
1. Open MPI (http://www.open-mpi.org/)
2. MPICH (http://www.mpich.org)
3. MVAPICH (http://mvapich.cse.ohio-state.edu/)
4. Intel MPI, Microsoft MPI, IBM MPI, Cray MPI 等。
- MPI 进程： 在 MPI 程序中，通常每个处理单元运行一个 MPI 进程，这些进程通过发送和接收消息来协同工作。
#### 2.2 MPI 通信类型和对象 (Partial)
- MPI 定义了多种通信模式和核心对象：
- 通信模式 (Communication patterns)：
```
双边通信 (Two-sided communication)： 最基本的通信模式，涉及发送方和接收方显式配对，如 MPI_Send (发送) 和 MPI_Recv (接收)。
集体通信 (Collective communication)： 涉及通信器中一组进程的协作通信，如 MPI_Barrier (屏障), MPI_Bcast (广播)。
单边通信 (One-sided communication)： 允许一个进程直接访问另一个进程的内存而无需远程进程显式参与，如 MPI_Put, MPI_Get。
MPI I/O： 用于并行文件输入输出的操作。
```
- 对象 (Objects)：
```
通信器 (Communicators)： MPI 进程组和通信上下文的集合。
数据类型 (Data Types)： 描述要发送或接收的数据的结构和类型。
请求 (Requests)： 用于管理非阻塞通信操作的状态。
```
### 3. MPI 通信器 (Communicators)
通信器是 MPI 中管理进程组和通信上下文的核心概念。
#### 3.1 通信器特性 (Characteristics)
- 通信上下文 (Context for communication)：
```
通信只能在同一个通信器内部发生；通信不会跨越通信器。这提供了 MPI 级别的通信隔离。
通信器与两个组（本地组和远程组）相关联。
通信器可以缓存 info objects（信息对象），用于设置通信器的行为。
```
- 通信器类型 (Communicator types)：
```
内通信器 (Intra-communicator)： 本地组与远程组相同 (local group == remote group)。这是最常见的通信器类型，用于同一组进程内部的通信。
互通信器 (Inter-communicator)： 本地组与远程组不重叠 (local group does not overlap remote group)。用于不同进程组之间的通信。
```
- MPI 进程可以是多个通信器的成员：
一个 MPI 进程在不同的通信器中可以拥有不同的等级 (rank)。
#### 3.2 通信器查询函数 (Query Functions)
- 获取通信器中的 MPI 进程数量 (size N)：
```
MPI_COMM_SIZE(comm, size)
IN - comm: 输入，通信器句柄。
OUT - size: 输出，comm 组中的进程数量 (整数)。
```
- 获取调用 MPI 进程在通信器中的等级 (0,1,2,...N-1)：
```
MPI_COMM_RANK(comm, rank)
IN - comm: 输入，通信器句柄。
OUT - rank: 输出，comm 组中调用进程的等级 (整数)。
```
- 比较通信器：
```
MPI_COMM_COMPARE(comm1, comm2, result)
IN - comm1: 输入，第一个通信器句柄。
IN - comm2: 输入，第二个通信器句柄。
OUT - result: 输出，比较结果 (整数，例如 MPI_IDENT, MPI_CONGRUENT, MPI_SIMILAR, MPI_UNEQUAL)。
```
#### 3.3 创建新通信器 (Creating new communicator)
MPI 提供了多种创建新通信器的方法，以支持灵活的进程分组：
- MPI_COMM_DUP_WITH_INFO(comm, info, newcomm)：
```
复制现有的通信器 comm，并可以指定一个 info 对象来定制新通信器行为。
IN - comm: 输入，待复制的通信器句柄。
IN - info: 输入，信息对象句柄。
OUT - newcomm: 输出，复制后的新通信器句柄。
```
- MPI_COMM_CREATE(comm, group, newcomm)：
```
基于现有通信器 comm 的一个子组 group 创建一个新的内通信器。
IN - comm: 输入，父通信器句柄。
IN - group: 输入，父通信器组的一个子集组句柄。
OUT - newcomm: 输出，新通信器句柄。
```
- MPI_COMM_SPLIT(comm, color, key, newcomm)：
```
将一个通信器 comm 分割成多个新的通信器。进程通过 color 值分组，并通过 key 值确定在新通信器中的等级顺序。
IN - comm: 输入，待分割的通信器句柄。
IN - color: 输入，控制子集分配的整数。具有相同 color 值的进程属于同一个新通信器。
IN - key: 输入，控制等级分配的整数。用于在新通信器中对进程进行排序。
OUT - newcomm: 输出，新通信器句柄。
```
- MPI_COMM_CREATE_FROM_GROUP(group, stringtag, info, errhandler, newcomm)：
```
从一个给定的进程组 group 创建一个新的内通信器。
IN - group: 输入，组句柄。
IN - stringtag: 输入，此操作的唯一标识符 (字符串)。
IN - info: 输入，信息对象句柄。
IN - errhandler: 输入，要附加到新内通信器的错误处理器句柄。
OUT - newcomm: 输出，新通信器句柄。
```
### 4. MPI 数据类型 (Data Types)
MPI 数据类型定义了发送和接收缓冲区的结构，允许在不同数据布局之间进行有效通信。
#### 4.1 数据类型特性 (Characteristics)
- MPI 定义了一组预定义的基本数据类型（例如，MPI_INT, MPI_DOUBLE, MPI_CHAR）。
- 用户可以从预定义的数据类型创建新的自定义数据类型，以处理非连续或复杂的数据结构。
- 类型映射 (Typemap)： {(type_0, disp_0), ..., (type_n-1, disp_n-1)}
1. type_t：块 t 的基本数据类型。
2. disp_t：块 t 相对于数据结构起始地址的位移 (以字节为单位)。
- MPI 提供了一组构造函数 (constructors) 来辅助创建新的数据类型。
- 基本指针、数据类型和数量的组合描述了提供给 MPI 实现的缓冲区。
#### 4.2 构造函数示例：MPI_TYPE_VECTOR
- 创建向量数据类型： MPI_TYPE_VECTOR(count, blocklength, stride, oldtype, newtype)
```
IN - count: 输入，块的数量 (非负整数)。
IN - blocklength: 输入，每个块中的元素数量 (非负整数)。
IN - stride: 输入，每个块开始之间元素的数量 (整数)，可以理解为步长。
IN - oldtype: 输入，旧（基本或自定义）数据类型句柄。
OUT - newtype: 输出，新创建的数据类型句柄。
```
- 具体示例 (来自 MPI 标准)：
1. 假设 oldtype 具有类型映射 {(double, 0), (char, 8)}，并且其 extent (数据类型所占用的字节数) 为 16。
2. 调用 MPI_TYPE_VECTOR(2, 3, 4, oldtype, newtype) 将创建以下类型映射：
```
第一个 oldtype 实例：(double, 0), (char, 8)
第二个 oldtype 实例（步长为 4 个 oldtype 元素，即 4×16=64 字节的位移）：(double, 64), (char, 72)
每个 oldtype 实例内有 3 个块 (blocklength=3)，但这里例子展示的是 oldtype 作为一个整体被复制。实际的 MPI_TYPE_VECTOR 会根据 blocklength 和 stride 组织 oldtype 的实例。图片中的示例更像是展开了 oldtype 在内存中的布局，然后根据 count, blocklength, stride 重新组织。
更准确地理解，MPI_TYPE_VECTOR(count, blocklength, stride, oldtype, newtype) 会创建 count 个由 blocklength 个 oldtype 元素组成的块，每个块之间有 stride 个 oldtype 元素的间隔。图中的展开式：
{(double, 0), (char, 8)} （第一个块的第一个元素）
{(double, 16), (char, 24)} （第一个块的第二个元素，与第一个元素间隔 16 字节）
{(double, 32), (char, 40)} （第一个块的第三个元素，与第二个元素间隔 16 字节）
然后是下一个 count 循环，从 stride 处开始。
{(double, 64), (char, 72)} （第二个块的第一个元素，与第一个块的第一个元素间隔 4×16=64 字节）
...依此类推。
```
### 5. MPI 点对点通信 (Point-to-Point Communication)
点对点通信是 MPI 中最基本的通信形式，涉及两个特定进程之间的消息交换。
#### 5.1 阻塞与非阻塞语义 (Blocking vs. Non-blocking Semantics)
- 阻塞语义 (Blocking Semantics)：
```
调用函数直到操作完全完成才返回。对于发送操作，这意味着发送缓冲区可以被安全修改；对于接收操作，这意味着接收缓冲区已准备好被读取。
示例： MPI_Send 和 MPI_Recv (MPI 提供了 4 种类型的阻塞发送函数)。
通信和计算是在通信器中进程组之间协调进行的。
```
- 非阻塞语义 (Non-blocking Semantics) (MPI_Isend/MPI_Irecv)：
```
函数立即返回，不等待通信完成。
通常需要后续调用 完成例程 (MPI_Wait, MPI_Test) 来检查相应的请求是否完成。
可能出现乱序完成 (Out-of-order completion)。
```
- 非阻塞的优点 (Non-blocking Advantages)：
```
允许通信和计算重叠： 在通信进行的同时，程序可以执行其他计算任务。
允许异步进展 (和手动控制)： 进程可以在通信未完成时继续执行，提高了程序的灵活性。
软件流水线 (Software pipelining)： 可以将计算和通信任务安排成流水线，提高效率。
噪声弹性 (Noise resilience)： 允许在某些通信延迟时仍能保持进展，对系统噪声更具鲁棒性。
同时处理多个未完成的操作。
```
#### 5.2 标签匹配 (Tag Matching)
MPI 使用标签来区分同一源/目的地对之间发送的不同类型的消息。
- 标签匹配参数：
```
源进程 (Source process)： 可以是特定的进程等级，也可以是通配符 MPI_ANY_PROC (任何进程)。
目的进程 (Destination process)： 必须是特定的进程等级。
通信器 (Communicator)： 必须是匹配的通信器。
用户标签 (User tag)： 可以是特定的整数标签，也可以是通配符MPI_ANY_TAG (任何标签)。
```
- 标签匹配参数不要求是唯一的。 （例如，多个发送可以有相同的标签，接收方可以使用 MPI_ANY_TAG 来匹配它们）。
- 标签匹配规则：
按发布顺序匹配 (Matching in the order posted)： 当有多个可能的匹配时，首先发布的接收请求会与首先到达的（匹配的）发送消息匹配。
- 应用程序提示 (在通信器信息对象中指定) 以简化匹配：
```
mpi_assert_no_any_tag：断言不使用 MPI_ANY_TAG。
mpi_assert_no_any_source：断言不使用 MPI_ANY_PROC。
mpi_assert_exact_length：断言消息长度精确匹配。
mpi_assert_no_allow_overtaking：断言不允许消息乱序。
```
#### 5.3 示例数据传输例程 (Example data transfer routines)
- 非阻塞发送操作：MPI_ISEND
```
MPI_ISEND(buf, count, datatype, dest, tag, comm, request)
IN - buf: 输入，发送缓冲区的起始地址。
IN - count: 输入，发送缓冲区中的元素数量 (非负整数)。
IN - datatype: 输入，每个发送缓冲区元素的类型 (句柄)。
IN - dest: 输入，目的地的等级 (整数)。
IN - tag: 输入，消息标签 (整数)。
IN - comm: 输入，通信器 (句柄)。
OUT - request: 输出，通信请求 (句柄)，用于后续的完成检查。
```
- 非阻塞接收操作：MPI_IRECV
```
MPI_IRECV(buf, count, datatype, src, tag, comm, request)
IN - buf: 输入，接收缓冲区的起始地址。
IN - count: 输入，接收缓冲区中的元素数量 (非负整数)。
IN - datatype: 输入，每个接收缓冲区元素的类型 (句柄)。
IN - src: 输入，源的等级 (整数)，可以是 MPI_ANY_SOURCE。
IN - tag: 输入，消息标签 (整数)，可以是 MPI_ANY_TAG。
IN - comm: 输入，通信器 (句柄)。
OUT - request: 输出，通信请求 (句柄)，用于后续的完成检查。
```
### 6. MPI 请求 (Requests) 和完成例程 (Completion Routines)
#### 6.1 请求特性 (Characteristics)
- 请求是 MPI 实现提供的通信操作句柄。
- 对于每个非阻塞和持久化操作都是唯一的。
- 用于跟踪“长时间运行”操作的状态。
- 用户通常不显式分配请求——它作为 MPI 调用（如 MPI_Isend 和 - MPI_Bcast_Init）的一部分由 MPI 库分配。
- 用户可以显式释放请求——但是，请勿对活跃 (active) 操作执行此操作，否则会导致未定义行为。
1. MPI_REQUEST_FREE(request)
2. INOUT - request: 输入/输出，通信请求句柄。
#### 6.2 完成例程 (Completion Routines)
- 等待操作完成：MPI_WAIT
```
MPI_WAIT(request, status)
INOUT - request: 输入/输出，通信请求句柄。
OUT - status: 输出，状态对象，包含有关已完成操作的信息。
```
- 语义： 阻塞调用，直到与 request 关联的通信操作完成。
- 测试操作完成：MPI_TEST
```
MPI_TEST(request, flag, status)
INOUT - request: 输入/输出，通信请求句柄。
OUT - flag: 输出，如果操作完成则为真 (逻辑布尔值)。
OUT - status: 输出，状态对象，如果操作完成则包含有关已完成操作的信息。
```
- 语义： 非阻塞调用。它检查与 request 关联的通信操作是否已经完成，并立即返回。如果操作未完成，flag 将为假，且 status 内容未定义。
### 7. MPI 单边操作 (One-sided Operations)
单边操作，也称为远程内存访问 (RMA) 或显式内存，允许一个进程直接读写另一个进程的内存，而无需远程进程的显式协作。
- 数据路径中活跃的一方 (One side active in the data path)：
- 远程写 (Remote write)： MPI_PUT, MPI_RPUT (非阻塞写)。
- 远程读 (Remote read)： MPI_GET, MPI_RGET (非阻塞读)。
- 远程更新 (Remote update)： MPI_ACCUMULATE, MPI_RACCUMULATE (非阻塞累加，例如对远程内存的值进行加法操作)。
- 远程读写和更新 (Remote read and update)： MPI_GET_ACCUMULATE, MPI_RGET_ACCUMULATE, MPI_FETCH_AND_OP (原子地获取远程值并执行操作)。
- 远程原子交换操作 (Remote atomic swap operations)： MPI_COMPARE_AND_SWAP (原子地比较并交换远程值)。
- 控制路径 (Control path) 需要双方都设置好。 虽然数据传输是单向的，但通常需要某种形式的同步机制（例如栅栏或窗口同步）来确保远程内存区域已被正确设置，并且操作是安全的。
- 需要确保远程方处于“就绪”状态。
### 8. MPI 集体通信 (Collective Communication) 深度解析
集体通信操作涉及通信器组中的所有进程协同工作，是实现大规模并行程序高效通信的关键。
#### 8.1 集体通信特性 (Characteristics)
- 集体通信在通信器的上下文中定义。
- 完成语义 (Completion semantics)：
1. 阻塞 (Blocking)： 每个通信器最多允许 1 个活跃的集体操作。当所有参与进程都完成其在该集体操作中的角色时，函数才会返回。
2. 非阻塞 (Nonblocking)： 允许多于 1 个集体操作同时活跃。函数立即返回，并返回一个请求句柄，后续需要通过 MPI_Wait 或 MPI_Test 检查完成。
- 操作范围 (Scope of operation)：
1. 非持久 (Non-persistent)： 每个操作都被假定为唯一的，每次调用都会重新初始化通信。
2. 持久 (Persistent)： 操作可以被调用多次，一次一个，直到持久对象被销毁。这避免了重复创建通信数据结构的开销。
- 参与等级 (Participating ranks)：
1. “常规”： 所有等级都参与到集体操作中。
2. 邻域 (Neighborhood)： 一些集体操作（例如 allgather, allgatherv, alltoall, alltoallw）可以与通信器关联的通信拓扑结构结合使用，只涉及拓扑中的邻居进程。
#### 8.2 集体同步 (Collective Synchronization)
- MPI_BARRIER(comm)：
1. 阻塞通信器 comm 组中的所有进程，直到所有进程都调用了 MPI_BARRIER。
2. 一个进程不能离开屏障，直到所有其他进程都到达了屏障。用于同步程序执行流。
#### 8.3 集体数据移动 (Collective Data Movement)
这些操作用于在进程组之间高效地分发或收集数据。
- MPI_BCAST (广播)：
1. 根进程（例如 P0）将其数据发送到通信器中的所有其他进程。
2. 示例： P0 (A) -> P0 (A), P1 (A), P2 (A), P3 (A)
- MPI_SCATTER/MPI_SCATTERV (分散)：
1. 根进程（例如 P0）将其数据分解成块，并将每个块发送给通信器中的一个进程。V 版本允许发送不同大小的块。
2. 示例： P0 (ABCD) -> P0 (A), P1 (B), P2 (C), P3 (D)
- MPI_GATHER/MPI_GATHERV (聚集)：
1. 通信器中的所有进程将其数据发送到一个进程（根进程）。V 版本允许从不同进程接收不同大小的块。
2. 示例： P0 (A), P1 (B), P2 (C), P3 (D) -> P0 (ABCD)
- MPI_ALLGATHER/MPI_ALLGATHERV (全局聚集)：
1. 所有进程将其数据发送给所有其他进程，并且每个进程最终都会获得所有进程的数据。
2. 示例： P0 (A), P1 (B), P2 (C), P3 (D) -> P0 (ABCD), P1 (ABCD), P2 (ABCD), P3 (ABCD)
- MPI_ALLTOALL/MPI_ALLTOALLV/MPI_ALLTOALLW (全局全交换)：
1. 每个进程将其发送缓冲区划分为 N 个块（N 是通信器中的进程数），并将第 i 个块发送给第 i 个进程。同时，每个进程从其他所有进程接收一个块。V 和 W 版本提供了更灵活的控制，允许不同进程发送和接收不同大小的数据。
2. 示例： P0 (A0 A1 A2 A3), P1 (B0 B1 B2 B3), P2 (C0 C1 C2 C3), P3 (D0 D1 D2 D3)
-> P0 (A0 B0 C0 D0), P1 (A1 B1 C1 D1), P2 (A2 B2 C2 D2), P3 (A3 B3 C3 D3)
#### 8.4 集体计算 (Collective Computation)
这些操作在进程组内执行聚合计算，并将结果分发。
- MPI_REDUCE (归约)：
1. 所有进程贡献一个值，并在一个进程（根进程）上执行归约操作（例如求和、最大值、最小值等）。
2. 示例： P0 (A), P1 (B), P2 (C), P3 (D) -> P0 (f(ABCD)) (其中 f 是归约函数)
- MPI_ALLREDUCE (全局归约)：
1. 所有进程贡献一个值，并在所有进程上执行归约操作。每个进程都获得最终的归约结果。
2. 示例： P0 (A), P1 (B), P2 (C), P3 (D) -> P0 (f(ABCD)), P1 (f(ABCD)), P2 (f(ABCD)), P3 (f(ABCD))
#### 8.5 非阻塞和持久化集体通信
与点对点通信类似，集体通信也有非阻塞和持久化版本，以提高性能和灵活性。
- 所有集体操作的非阻塞变体：
1. 例如 MPI_IBCast(<bcast args>, MPI_Request *req)
2. 允许操作立即返回，并返回一个请求句柄，用户可以通过 MPI_Wait 或 MPI_Test 检查完成状态。
- 所有集体操作的持久化变体：
1. 例如 MPI_BCAST_INIT(<bcast args>, MPI_Request *req)
2. 允许操作在一次初始化后被多次启动（通过 MPI_Start 或 MPI_Startall），从而减少重复设置通信的开销。
- 所有集体操作的邻域变体：
1. 例如 MPI_NEIGHBOR_ALLGATHER(<Allgather args>)
2. 用于在定义了拓扑的通信器中执行集体通信，只涉及拓扑邻居。
### 9. MPI 性能基准测试与分析
性能基准测试和分析是优化 MPI 应用程序的关键环节。
#### 9.1 MPI 基准测试工具 (OSU Micro-benchmarks)
- osu_latency (延迟基准测试)：
```
用于测量小消息的点对点通信延迟。
运行命令示例： mpirun -np 2 -map-by node --report-bindings --map-by node $HPCX_MPI_TESTS_DIR/osu-micro-benchmarks-5.3.2/osu_latency
结果解读： 显示不同消息大小 (Size) 对应的延迟 (Latency)，单位通常是微秒 (us)。通常小消息延迟较低且稳定。
```
- osu_bw (带宽基准测试)：
```
用于测量大消息的点对点通信带宽。
运行命令示例： mpirun -np 2 -map-by node --report-bindings --map-by node $HPCX_MPI_TESTS_DIR/osu-micro-benchmarks-5.3.2/osu_bw
结果解读： 显示不同消息大小 (Size) 对应的带宽 (Bandwidth)，单位通常是 MB/s。通常大消息能达到更高的带宽，直到网络饱和。
```
#### 9.2 HCOLL 启用与禁用比较 (Collective Operations Performance)
HCOLL (Hierarchical Collective Communication Library) 是 Mellanox 针对其 InfiniBand 网络优化的集体通信库。
- 测试 MPI_Barrier 延迟 (112 核心，4 节点，每节点 28 核心)：
```
启用 HCOLL (-mca coll_hcoll_enable 1)： 平均延迟约 3.64 us。
禁用 HCOLL (-mca coll_hcoll_enable 0) 并使用其他 COLL 组件 (basic, tuned, libnbc)： 平均延迟约 6.88 us 或 8.42 us。
结论： 启用 HCOLL 显著降低了 MPI_Barrier 的延迟，表明其在同步操作中提供了更优的性能。
```
- 测试 MPI_Allgather 性能 (112 核心，4 节点，每节点 28 核心)：
```
对比 HCOLL、tuned 和 basic 三种集体通信实现。
结果解读： 针对不同消息大小 (msg_sz)，HCOLL 的性能 (延迟更低) 始终优于 tuned 和 basic 实现。尤其对于大消息，HCOLL 的性能优势更加明显，例如在 1MB 消息时，HCOLL 比 tuned 快约 2 倍，比 basic 快近 10 倍。
结论： HCOLL 对 MPI_Allgather 等集体数据移动操作的性能提升非常显著，在大规模并行应用中至关重要。
```
#### 9.3 使用 IPM 进行性能分析 (Profiling with IPM)
IPM (Integrated Performance Monitoring) 是一个强大的性能分析工具，用于识别 MPI 应用程序中的性能瓶颈。
- IPM 性能分析示例：天气预报应用程序
- 运行环境： 在 4240 个核心上运行。
总 Wallclock 时间： 约 163.44 秒。
- MPI 时间占比： 应用程序总时间的 43% 花费在 MPI 调用上。
- MPI 调用时间分布： 在这 43% 的 MPI 时间中，超过 50% 的时间花费在 MPI_Scatterv 和 MPI_Allreduce 这两个集体通信操作上。
- 具体数据 (部分展示)：
- MPI_Scatterv: 占 MPI 时间的 40.38%，总时间 64.1314 秒。
- MPI_Allreduce: 占 MPI 时间的 32.55%，总时间 140.717 秒。注意，这里的时间单位是秒 (sec)，而不是微秒或毫秒。
- MPI_Allreduce 的最大单次操作时间达到 1.36 秒。
结论： 通过 IPM 的分析，可以清晰地识别出 MPI_Scatterv和MPI_Allreduce 是该天气预报应用程序的主要 MPI 瓶颈，是优化工作应优先关注的地方。