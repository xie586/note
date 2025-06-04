## 1. 现代GPU架构 
### 1.1 硬件组成
- Host CPU (主机CPU)：通过FSB (前端总线) 和North Bridge (北桥) 连接到System DRAM (系统内存)。GPU通过PCI-Express连接到North Bridge 。
- GPU (图形处理器)：自身拥有高速的GDDR (图形DDR) 显存 。
- Streaming Multiprocessor (SM) (流多处理器)：GPU的核心计算单元 。 

1. NVIDIA Fermi架构：包含512个处理单元 (SP) 。每个SM包含指令缓存、Warp调度器、寄存器文件 (32.768 x 32位)、LD/ST单元、SFU (特殊功能单元)、互连网络以及64 KB的共享内存/L1缓存和统一缓存 。
2. 新一代GPU (例如GeForce RTX 4090)：每个SM拥有128个核心 (流处理器SP)、16MB共享内存、32MB寄存器文件和4个Tensor核心 。整个芯片拥有128个SM，即16,384个核心、512个Tensor核心和72MB L2缓存 。
### 1.2 内存层级
- System DRAM (系统内存)：连接到Host CPU 。
- GDDR (显存)：连接到GPU 。
- L2 Cache (二级缓存)：位于GPU内部 。
- Shared Memory / L1 Cache (共享内存/L1缓存)：位于每个SM内部，64KB 。
## 2. CUDA执行模型 
### 2.1 异构计算 
- CUDA应用程序是异构的C程序，由主机 (CPU) 和设备 (GPU) 组成 。
- 主机C代码执行程序的串行部分 。
- 设备内核代码执行程序的并行部分 。
- GPU用于并行化计算密集型函数 。
### 2.2 SIMT (单指令多线程) 
- CUDA执行模型是SIMD (单指令多数据) 和多线程的结合，称为SIMT 。
- 需要细粒度并行 (fine-grained parallelism) 。
- 通过内核函数调用实现并行：KernelA<<< nBlk, nTid >>>(args); 。
### 2.3 数据并行性：向量加法示例 
- 基本思想：将数据分区，并将计算与数据关联起来 。
- 多线程任务：任务由多个线程执行，多个操作同时在多个SP上进行 。
- 细粒度并行：一个任务仅仅是一个加法操作，线程数量等于加法操作的数量 。
### 2.4 并行线程阵列 
- Grid (网格)：一个CUDA内核由一个线程网格执行 。
- SPMD (单程序多数据)：网格中的所有线程都运行相同的内核代码 。
- Thread Block (线程块)：线程阵列被划分为多个块，这些块分配给不同的SM 。
- 线程协作：块内的线程通过共享内存、原子操作和屏障同步 (barrier synchronization) 进行协作 。
- 线程索引：每个线程都有自己的索引，用于计算内存地址和做出控制决策 。
### 2.5 线程块的可扩展性 
- 每个块可以以任何顺序独立执行 。
- 硬件可以随时将块分配给任何多处理器 。
- 内核可以扩展到任意数量的并行处理器 。
### 2.6 线程块的执行 
- 线程以块的粒度分配给流多处理器 (SM) 。
- 一个SM可以根据资源分配多达8个块 。例如，Fermi SM最多可以容纳1,536个线程，可以是256个线程/块 * 6个块，或者512个线程/块 * 3个块等 。
- SM维护线程/块索引号，并管理/调度线程执行 。
### 2.7 Warps (线程束) 作为调度单元 
- 每个块以32个线程的warp为单位执行 。这是一个实现细节，不是CUDA编程模型的一部分 。
- Warps是SM中的调度单元 。
- Warp中的线程以SIMD方式执行 。
- 零开销Warp调度：SM实现了零开销的warp调度 。当warp的下一条指令的操作数准备好时，它们就可以执行 。当一个warp被选中时，其中的所有线程执行相同的指令 。
## 3. CUDA设备内核代码 
### 3.1 向量加法内核示例 
```C
__global__
void vecAddKernel(float* A, float* B, float* C, int n)
{
    int i = threadIdx.x + blockDim.x * blockIdx.x; // 计算当前线程处理的元素索引 
    if (i < n) {
        C[i] = A[i] + B[i]; // 每个线程执行一对加法 
    }
}
```
### 3.2 CUDA函数声明 
- __global__：定义一个内核函数 。 
- 内核函数必须返回void 。
- __device__：表示函数在设备上执行，只能从设备调用。
- __host__：表示函数在主机上执行，只能从主机调用。
- __device__ 和 __host__ 可以一起使用 。
- 如果单独使用 __host__，它是可选的 。
- 声明CUDA变量时，__device__ 在与 __shared__ 或 __constant__ 一起使用时是可选的 。
### 3.3 CUDA内存模型概述 (编程者视角) 
- Registers (寄存器)：每个线程私有，读/写访问 。
- Shared Memory (共享内存)：每个线程块私有，读/写访问 。速度远高于全局内存 。生存期与线程块相同，线程结束执行后内容消失 。
- Global Memory (全局内存)：所有线程共享，读/写访问 。主机代码可以向/从全局内存传输数据 。
- Constant Memory (常量内存)：所有线程共享，主机可写，设备只读。
#### 3.3.1 共享内存的特性 
- 一种特殊类型的内存，其内容在内核源代码中明确定义和使用 。
- 每个SM中有一个 。
- 访问速度 (延迟和吞吐量) 比全局内存快得多 。
- 访问和共享范围：线程块 。
- 生命周期：线程块，在相应线程完成并终止执行后内容将消失 。
- 通过内存加载/存储指令访问 。
- 共享内存变量声明示例：
```C
__global__
void myKernel(unsigned char * in, unsigned char * out, int w, int h)
{
    __shared__ float ds_in[TILE_WIDTH][TILE_WIDTH]; // 在内核中声明共享内存变量 
    // …
}
```
## 4. CUDA主机代码 
### 4.1 数据传输 
- cudaMalloc()：在设备全局内存中分配对象 。 
参数：指向已分配对象指针的地址，以及已分配对象的大小 (字节) 。
- cudaFree()：从设备全局内存中释放对象 。 
参数：指向要释放对象的指针 。
- cudaMemcpy()：内存数据传输 。 
1. 参数：目标指针、源指针、拷贝字节数、传输类型/方向 。
2. 传输到设备是异步的 。
- 向量加法主机代码示例：
```C
void vecAdd(float *h_A, float *h_B, float *h_C, int n)
{
    int size = n * sizeof(float);
    float *d_A, *d_B, *d_C;

    // 分配设备内存并拷贝数据
    cudaMalloc((void **) &d_A, size);
    cudaMemcpy(d_A, h_A, size, cudaMemcpyHostToDevice);
    cudaMalloc((void **) &d_B, size);
    cudaMemcpy(d_B, h_B, size, cudaMemcpyHostToDevice);
    cudaMalloc((void **) &d_C, size);

    // 内核调用代码 (稍后展示)

    // 从设备内存拷贝结果到主机
    cudaMemcpy(h_C, d_C, size, cudaMemcpyDeviceToHost);

    // 释放设备内存
    cudaFree(d_A); cudaFree(d_B); cudaFree (d_C);
}
``` 

### 4.2 内核启动 
- 通过 <<<DimGrid, DimBlock>>> 语法启动内核 。 
- Grid dimension (网格维度)：总块数 。
- Block dimension (块维度)：每个块的线程数 。
- 总线程数 = 网格维度 * 块维度 。
- 示例：
```C
// 运行ceil(n/256.0)个块，每个块256个线程
vecAddKernel<<<ceil(n/256.0), 256>>>(d_A, d_B, d_C, n);
``` [cite: 128]
或者使用 `dim3` 结构体：
```c
dim3 DimGrid((n-1)/256 + 1, 1, 1); // 等效于ceil函数 
dim3 DimBlock(256, 1, 1);
vecAddKernel<<<DimGrid, DimBlock>>>(d_A, d_B, d_C, n);
``` 

### 4.3 blockIdx 和 threadIdx 
- 每个线程使用索引来决定处理哪个数据 。
- blockIdx：1D、2D或3D (CUDA 4.0) 。
- threadIdx：1D、2D或3D 。
- 简化处理多维数据时的内存寻址，例如图像处理或求解体积上的PDE 。
- 2D数组示例：
```C
__global__ void PictureKernel(float* d_Pin, float* d_Pout,
                             int height, int width)
{
    // 计算d_Pin和d_Pout元素的行号
    int Row = blockIdx.y * blockDim.y + threadIdx.y; [cite: 135]
    // 计算d_Pin和d_Pout元素的列号
    int Col = blockIdx.x * blockDim.x + threadIdx.x; [cite: 136]

    // 如果在范围内，每个线程计算一个d_Pout元素
    if ((Row < height) && (Col < width)) {
        d_Pout[Row * width + Col] = 2.0 * d_Pin[Row * width + Col]; // 每个像素值乘以2.0 
    }
}
```
- 主机代码启动2D内核示例：
```C

dim3 DimGrid((width - 1) / 16 + 1, (height - 1) / 16 + 1, 1);
dim3 DimBlock(16, 16, 1);
PictureKernel<<<DimGrid, DimBlock>>>(d_Pin, d_Pout, height, width);
```
## 5. 内存优化
### 5.1 全局内存 (DRAM) 带宽 
- GPU的峰值浮点运算能力通常远高于其DRAM带宽所能支持的带宽需求 。
- 例如，一个拥有1,500 GFLOPS峰值浮点速率和200 GB/s DRAM带宽的GPU，需要6,000 GB/s的带宽才- 能达到峰值FLOPS速率，但实际只能达到50 GFLOPS，仅为峰值浮点执行速率的3.33% 。
- 为了接近峰值FLOPS性能，需要大幅减少内存访问 。 
1. 内存合并 (Memory coalescing) 。
2. 有效利用共享内存 。
### 5.2 DRAM Bursting (突发传输) 
- 现代DRAM系统设计为始终以突发模式访问 。
- 当访问一个位置时，同一突发段中的所有其他位置也会被传输到处理器 。
- 如果访问的不是连续位置，突发传输的字节将被丢弃 。
- 实践中，通常有至少4GB的地址空间，突发段大小为128字节或更多 。
### 5.3 内存合并 (Memory Coalescing) 
- 当一个warp的所有线程执行加载指令时，如果所有访问的位置都落在同一个突发段内，则只会发起一个DRAM请求，此时访问是完全合并的 。
- 未合并访问：当访问的位置跨越突发段边界时，合并失败，会发起多个DRAM请求，访问未完全合并，部分传输的字节未被线程使用 。
- 判断访问是否合并：如果数组访问的索引形式为 A[(不依赖于threadIdx.x的表达式) + threadIdx.x]，则warp中的访问是连续的 。
- 需要的是针对一个指令的线程之间的局部性，而不是针对一个线程的后续指令之间的局部性 。
### 5.4 Tiling/Blocking (分块/分瓦) 
- 基本思想：将全局内存内容划分为小块 (tiles) 。
- 将线程的计算集中在一次处理一个或少量几个小块上 。
- 这种技术有助于提高内存访问的局部性，从而提高性能。