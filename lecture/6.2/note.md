## 高斯消元与 MPI 派生数据类型
本讲座回顾了作业 6，并深入探讨了带部分主元的高斯消元的并行算法，特别是在分布式内存机器上的实现。此外，还详细介绍了 MPI 派生数据类型及其在优化通信中的应用。

### 1. 带部分主元的高斯消元 (Gaussian Elimination with Partial Pivoting - GEPP) 
GEPP 是一种数值稳定的矩阵分解算法，它将矩阵 A 分解为 P⋅L⋅U，其中 P 是置换矩阵，L 是下三角矩阵，U 是上三角矩阵。

#### 算法步骤: 
对于 i=1 到 n−1:
1. 查找主元： 找到并记录 k，使得 ∣A(k,i)∣=max{i≤j≤n}∣A(j,i)∣ (即当前列剩余部分中绝对值最大的元素)。
2. 奇异性检查： 如果 ∣A(k,i)∣=0，则发出警告并退出，因为矩阵可能是奇异的或接近奇异的。
3. 行交换： 如果 k!=i，则交换矩阵 A 的第 i 行和第 k 行。
4. 缩放： A(i+1:n,i)=A(i+1:n,i)/A(i,i) (将当前列 i 的下方元素进行缩放)。
5. 更新子矩阵： A(i+1:n,i+1:n)=A(i+1:n,i+1:n)−A(i+1:n,i)⋅A(i,i+1:n) (更新右下角子矩阵)。
#### 问题与优化: 

- BLAS 级别： 基本的 GEPP 算法主要使用 BLAS 1 (向量缩放) 和 BLAS 2 (秩 1 更新) 操作。然而，BLAS 3 (矩阵乘法) 通常是最快的操作。
- BLAS 3 转换 (阻塞 - Blocking)： 
1. 延迟更新 (Delayed Updates)： 核心思想是保存多个连续的 BLAS 2 (秩 1) 更新，然后在一个 BLAS 3 (矩阵乘法) 操作中同时应用这些更新。
2. 块大小 (b)： 需要选择一个合适的块大小 b。
b 应足够小，以便活动子矩阵（b 列）能放入缓存。

b 应足够大，以使 BLAS 3 操作高效。
- 算法流程 (基于 BLAS 3 的高斯消元)：  对于 ib=1 到 n−1 步长 b: 
1. end = ib + b - 1
2. 应用 BLAS 2 版本的 GEPP 到 A(ib:n,ib:end) 得到 P′L′U′。
3. 设 LL 为 A(ib:end,ib:end)+I 的严格下三角部分。
4. A(ib:end,end+1:n)=LL^−1⋅A(ib:end,end+1:n) (更新 U 的下一个 b 行)。
5. A(end+1:n,end+1:n)=A(end+1:n,end+1:n)−A(end+1:n,ib:end)⋅A(ib:end,end+1:n) (通过单次矩阵乘法应用延迟更新)。
### 2. 分布式内存机器上的任务/数据划分与分配 
与共享内存机器不同，分布式内存机器需要显式地在进程间分发数据并使用消息传递进行数据交换。最小化通信成本是关键考虑因素。

#### 不同数据布局对并行 GEPP 的影响： 

1. 1D 列块布局 (1D Column Blocked Layout):
- 优点：可权衡负载平衡和 BLAS 3 性能。
- 缺点：P0 在前 n/4 步后空闲，负载平衡不佳。
2. 1D 列循环布局 (1D Column Cyclic Layout):
- 优点：负载平衡。
- 缺点：难以有效利用 BLAS 3，寻址复杂。
3. 1D 列块循环布局 (1D Column Block Cyclic Layout):
- 优点：负载平衡，可以有效利用 BLAS 3。被认为是“赢家”。
4. 块倾斜布局 (Block Skewed Layout):
- 缺点：P0 在前 n/2 步后空闲，负载平衡不佳。
5. 2D 行列块布局 (2D Row and Column Blocked Layout):
6. 2D 行列块循环布局 (2D Row and Column Block Cyclic Layout):
#### 分布式内存机器上的 LU 算法： 
对于 ib=1 到 n−1 步长 b:
1. end = min(ib + b - 1, n)
2. 对于 i=ib 到 end:
- (1) 找到主元行 k，列广播。
- (2) 交换块列中的行 k 和行 i，广播行 k。
- (3) A(i+1:n,i)=A(i+1:n,i)/A(i,i)
- (4) A(i+1:n,i+1:end)−=A(i+1:n,i)⋅A(i,i+1:end)
3. (5) 广播所有交换信息到右侧和左侧。
4. (6) 将所有行交换应用于其他列。
5. (7) 广播 LL 到右侧。
6. (8) A(ib:end,end+1:n)=LL∖A(ib:end,end+1:n)
7. (9) 广播 A(ib:end,end+1:n) 向下。
8. (10) 广播 A(end+1:n,ib:end) 到右侧。
9. (11) 消元 A(end+1:n,end+1:n)。
### 3. MPI 派生数据类型 (MPI Derived Datatypes) 
MPI 预定义了基本数据类型 (如 MPI_CHAR, MPI_INT, MPI_FLOAT, MPI_DOUBLE)。此外，MPI 允许用户根据这些基本类型构建自己的数据类型。 

- 优点： 

1. 一次 MPI 调用通信非连续数据或不同数据类型。
2. 使代码更紧凑和可维护。
3. 提高非连续数据通信的效率 (取决于具体实现)。
4. 可用于点对点和集体通信。
#### 问题示例：发送矩阵的列 
- 在 C 语言中，矩阵按行主序存储。发送一行数据很容易，因为它们是连续的。
- 发送一列数据则不然，因为元素不连续。
- 解决方案一 (低效)： 每次发送一个元素 -> 多个小消息，高启动开销。 
- 解决方案二 (次优)： 将元素打包到临时缓冲区，然后发送单个消息 -> 额外的内存复制开销。 
- 派生数据类型 (最优)： 无需额外复制，直接从非连续内存到网络通信缓冲区，或从网络通信缓冲区到非连续用户内存。 
#### 定义新数据类型的步骤： 
1. 声明： MPI_Datatype new_type;
2. 构造： 使用 MPI 提供的构造函数，如 MPI_Type_vector(nblk, len, stride, type, &new_type);
3. 提交： MPI_Type_commit(&new_type);
4. 使用： 然后可以在通信中使用新数据类型。
5. 释放： 不再需要时，使用 MPI_Type_free(&new_type); 释放。
#### 常用的数据类型构造函数： 
- MPI_Type_contiguous(int count, MPI_Datatype oldtype, MPI_Datatype *newtype): 创建一个包含 count 个 oldtype 连续元素的 newtype。 
- 示例： 发送矩阵 a 第 1 行的前 n 个元素。 
```C
MPI_Datatype row_t;
MPI_Type_contiguous(n, MPI_DOUBLE, &row_t);
MPI_Type_commit(&row_t);
MPI_Send(&a[1][0], 1, row_t, 1, des, MPI_COMM_WORLD);
```
- MPI_Type_vector(int count, int blocklen, int stride, MPI_Datatype oldtype, MPI_Datatype *newtype): 创建一个包含 count 个块的 newtype，每个块有 blocklen 个 oldtype 元素，块之间间隔 stride。 
1. count: 块的数量
2. blocklen: 每个块中的元素数量
3. stride: 两个连续块起始元素之间的元素数量
- 示例： 发送矩阵 a 的第 1 列。 
```C
// For a 2D matrix of size m x n
MPI_Datatype col_t;
MPI_Type_vector(m, 1, n, MPI_DOUBLE, &col_t); // m blocks, each 1 element, stride n
MPI_Type_commit(&col_t);
MPI_Send(&a[0][1], 1, col_t, r_nbr, 200, MPI_COMM_WORLD);
```

- 接收列到连续缓冲区：MPI_Recv(w_bf, m, MPI_DOUBLE, l_nbr, 200, MPI_COMM_WORLD, &status); 
- 接收列到非连续列：MPI_Recv(&a[0][1], 1, col_t, l_nbr, 200, MPI_COMM_WORLD, &status); 
- MPI_Type_indexed(int count, int blocklens[], int displs[], MPI_Datatype old_type, MPI_Datatype *newtype): 创建一个 newtype，包含 count 个块，每个块有不同的长度和位移。 
1. count: 块的数量
2. blocklens: 每个块的元素数量数组
3. displs: 每个块的位移数组
- 示例： 创建一个表示上三角矩阵的派生数据类型。 
```C
double a[100][100];
int disps[100], blocklens[100];
MPI_Datatype upper;
for (i=0; i<100; i++) {
    disps[i] = 100*i+i;
    blocklens[i] = 100-i;
}
MPI_Type_indexed(100, blocklens, disps, MPI_DOUBLE, &upper);
MPI_Type_commit(&upper);
MPI_Send(a, 1, upper, dest, tag, MPI_COMM_WORLD);
```
- MPI_Type_create_subarray(...): 用于发送矩阵的子矩阵。 
- MPI_Type_struct(...): 处理比 indexed 更一般的情况，可以包含多种数据类型。 
### 4. MPI Cartesian Topology Routines 
MPI 允许程序员将处理器组织成逻辑 k 维网格。 
- MPI_Cart_create(MPI_Comm comm_old, int ndims, int *dims, int *periods, int reorder, MPI_Comm *comm_cart): 创建一个新的笛卡尔拓扑通信器。 
- MPI_Cart_coord(MPI_Comm comm_cart, int rank, int maxdims, int *coords): 给定组中的 rank，确定其在笛卡尔拓扑中的坐标。 
- MPI_Cart_rank(MPI_Comm comm_cart, int *coords, int *rank): 给定笛卡尔位置，确定其在通信器中的 rank。 
- MPI_Cart_shift(MPI_Comm comm, int direction, int displ, int *source, int *dest): 给定移位方向和量，找到源和目标 rank。 
- MPI_Cart_sub(MPI_Comm comm, const int remain_dims[], MPI_Comm* new_comm): 将笛卡尔拓扑划分为子网格。 
### 5. 实验练习 (Lab Exercises)
练习 1：列块循环划分 

- 修改程序以实现矩阵的列块循环划分和分布。
- 进程 0 创建完整矩阵 A，所有进程创建 AK (存储分配的列块)。
- 分发完成后，所有进程将其 process_id + 1 写入接收到的列块，然后将块发送回进程 0。
- 最终，所有块应返回其原始位置。

练习 2：广播列块进行更新 
- 在并行块高斯消元中，每次迭代都会从一个进程广播一个列块的乘数，以更新尾随子矩阵。
- 每个进程创建一个大小为 M×b 的工作数组 AW 并初始化为零，用于接收广播数据。
- 进程轮流广播其 AK 中的列块，每次一个块。
每次广播后，块中的行数减少 b (块大小)，进程 0 首先以 M−b 行开始。
- 接收到的块将覆盖 AW 中的旧数据。

### 练习 3：广播下三角乘数 
- 在并行块高斯消元中，每次迭代都会从一个进程广播前导块列中一个小的 b×b 子矩阵的下三角乘数，以更新前导行块。
- 修改 Lab Exercise 4 的程序，使进程轮流广播 AK 中每个列块顶部小 b×b 子矩阵下三角中的元素，一次一个。
- 注意，每次广播后，下一个列块的行长度减少 b，进程 0 首先从 M 行开始。
### 6. 总结 (Summary)
- 分布式内存平台特点： 
1. 每个进程拥有自己的局部寻址空间，无法直接访问其他进程的局部变量。
2. 没有全局变量可供所有进程访问。
3. 进程必须通过消息传递机制 (send 和 receive) 显式地交换数据。
- 分布式内存机器上的数据与任务划分： 
1. 需要同时考虑数据和任务的划分与分配。
2. 对数据/任务分配的限制更多。
3. 必须认真考虑数据局部性、负载平衡和资源利用效率。
4. 数据通信会产生额外成本，必须认真考虑如何最小化通信开销。
- MPI (Message Passing Interface)： 
1. 用于开发可移植消息传递程序的行业标准库。
2. 几乎所有商用并行计算机都提供 MPI 实现。
3. 标准规定了超过一百个例程。
4. 单程序多数据 (SPMD) 模式： 所有进程在不同的数据部分和 P 个处理元素上运行相同的程序。MPI 程序通常使用这种模式。
5.  使用 rank (ID 从 0 到 P−1) 来控制不同进程执行程序的特定部分。 
- 已学 MPI 例程回顾： 
1. 初始化与终止： MPI_Init, MPI_Finalize
2. 点对点通信： MPI_Send, MPI_Recv, MPI_Sendrecv, MPI_Sendrecv_replace, MPI_Isend, MPI_Irecv
3. 查询信息： MPI_Comm_size, MPI_Comm_rank, MPI_Get_count
4. 集体通信：

一对多： MPI_Bcast, MPI_Scatter, MPI_Scatterv

多对一： MPI_Gather, MPI_Gatherv

多对多： MPI_Allgather, MPI_Alltoall, MPI_Allgatherv

其他： MPI_Barrier, MPI_Reduce

5. 时钟时间： MPI_Wtime
6. 笛卡尔拓扑： MPI_Cart_create, MPI_Cart_coord, MPI_Cart_rank, MPI_Cart_shift, MPI_Cart_sub
### 7. 作业 7 (Homework 7) 
- 任务 1： 使用 1D 列块循环数据划分实现带部分主元的高斯消元并行算法。
- 任务 2： 为具有特殊形式的矩阵设计高效的并行高斯消元算法。 
在这种矩阵中，除了 c_(i,i+1)之外，上三角的所有元素都为零。

例如，对于 n=7 的矩阵形式： c_(0,0) c(0,1) c_(1,0) c_(1,1)  c_(1,2)  c_(2,0)  c_(2,1)  c_(2,2)  c_(2,3)  c_(3,0)  c_(3,1)  c_(3,2)  c_(3,3)  c_(3,4)  c_(4,0)  c_(4,1)  c_(4,2)  c_(4,3)  c_(4,4)  c_(4,5)  c_(5,0)  c_(5,1)  c_(5,2)  c_(5,3)  c_(5,4)  c_(5,5)  c_(5,6)  c_(6,0)  c_(6,1)  c_(6,2)  c_(6,3)  c_(6,4)  c_(6,5)  c_(6,6)
​
