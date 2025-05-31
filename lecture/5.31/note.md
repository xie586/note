## 工作共享构造：section 指令 
我们已经学习了 OpenMP 指令来并行化 for 循环。为了并行化代码而不是 for 循环，我们可以使用 sections 指令：
```C
#pragma omp sections [clause...] newline
{
    #pragma omp section
    structured_block;
    #pragma omp section
    structured_block;
    // ... 更多 section
}
```
在 sections 构造中，每个 section 由一个线程执行 。默认情况下，omp sections 结束时会有一个 barrier。使用 nowait 子句可以关闭 barrier 。sections 指令必须在并行区域内部 。

- 示例： 顺序代码 
```C
v = alpha();
w = beta();
x = gamma(v, w);
y = delta();
z = zita(x, y);
```
函数 alpha 和 beta 可以并行执行 。函数 gamma 和 delta 也可以并行执行 。

## 带有 sections 指令的 OpenMP 代码： 
```C
#pragma omp parallel
{
    #pragma omp sections
    {
        #pragma omp section
        v = alpha();
        #pragma omp section
        w = beta();
    }
    #pragma omp sections
    {
        #pragma omp section
        x = gamma(v, w);
        #pragma omp section
        y = delta();
    }
    z = zita(x, y);
}
```
## 超越 for 循环的结构 
考虑一个简单的链表遍历：
```C
node *p = head;
while (p)
{
    process(p);
    p = p->next;
}
```
OpenMP loop 工作共享构造只适用于循环迭代次数可以在编译时用封闭形式表达式表示的循环 。while 循环不受支持 。然而，在实践中，有时我们确实需要并行化 while 循环（以及其他通用数据结构） 。

### 一种解决方案： 
首先计算链表中的节点数，将每个节点的指针复制到一个数组中，然后使用 OpenMP for 构造 。
```C
p = head;
count = 0;
while (p) {
    parr[count] = p; // 将每个节点的指针复制到数组中 
    p = p->next;
    count++;         // 计算链表中的节点数 

#pragma omp parallel
{
    #pragma omp for schedule(static, 1)
    for (int i = 0; i < count; i++)
        process(parr[i]); // 使用 for 循环并行处理节点 
}
```
这种方法需要多次遍历数据 。

- 更简洁的方法： 使用 OpenMP task 构造 。

## OpenMP firstprivate 子句 
在使用 private(list) 子句时，private 子句中的 list 会为每个线程创建一个本地副本，但该值未初始化 。

- 示例： 
```C
int tmp = 0; // tmp 在这里是 0 
#pragma omp parallel for private(tmp)
for (int i = 0; i < n; i++)
    tmp += i;
printf("%d\n", tmp); // tmp 未初始化 
```
使用 firstprivate 子句时，私有副本将从共享变量初始化 。
- 示例： 
```C
int tmp = 0;
#pragma omp parallel for firstprivate(tmp)
for (int i = 0; i < n; i++)
    tmp += i;
printf("%d\n", tmp); // 每个线程都获得自己的 tmp 副本，初始值为 0 
```
## OpenMP task 构造 
>#pragma omp task

任务是独立的工作单元 。任务由以下部分组成：

- 要执行的代码 
- 数据环境 
- 内部控制变量 (ICV) 

线程执行每个任务的工作 。运行时系统决定何时执行任务 。基本思想是建立一个任务队列 。当一个线程遇到 task 指令时，它会打包一个新的任务实例，然后继续执行 。一些线程会在稍后某个时间执行该任务 。

## 使用 OpenMP task 构造解决链表遍历问题： 
```C
#pragma omp parallel // 1. 创建一个线程组 
{
    #pragma omp single // 2. 一个线程执行 single 构造 
    {
        node *p = head;
        while (p) { // 3. 该单个线程每次都会使用不同的 p 值创建一个任务 
            #pragma omp task firstprivate(p)
            process(p);
            p = p->next;
        }
    } // 其他线程在 single 构造结束处的隐式 barrier 处等待 
    // 4. 在 barrier 处等待的线程执行任务 
    // 一旦所有任务完成，执行将超越 barrier 
}
```
- 图示： 
该图显示了使用 OpenMP task 构造进行链表遍历的并行执行流程。单个线程（Thr1）负责创建任务，而其他线程（Thr2, Thr3, Thr4）在等待任务创建完成后，共同并行执行这些任务。

## 总结 
### 共享内存机器： 
- 全局数据由所有线程共享。
- 数据在不同线程使用时无需移动。
- 任务划分和分配相对简单。
- 线程协调必须通过共享变量上的同步显式完成，且必须正确应用，尽量减少同步开销。
- 局部性非常重要：优化单处理器或核心上的性能，并增加计算强度（内存层次结构）。
### OpenMP： 
- 一种基于指令的应用程序编程接口（API），用于在共享内存架构上开发并行程序。
- 一个小型 API，用更简单的指令隐藏繁琐的线程调用。
- 允许程序员将程序分为串行区域和并行区域，而不是显式创建并发执行的线程。
- 提供了一些同步构造。
## OpenMP 指令、子句和函数： 
- 创建线程组： #pragma omp parallel
- 在线程之间共享工作： #pragma omp for, #pragma omp single, #pragma omp sections, #pragma omp task
- 防止冲突（防止数据竞争）： #pragma omp critical, #pragma omp atomic, #pragma omp barrier
- 数据环境子句： private(list), firstprivate(list), shared(list), reduction(op:list)
- 循环调度子句： schedule(static, chunk), schedule(dynamic, chunk), collapse(n)
- 禁用隐式 barrier： nowait
- 设置/获取环境变量函数： omp_set_num_threads(), omp_get_num_threads(), omp_get_thread_num(), omp_get_procs()
- 获取时钟时间函数： omp_get_wtime()
- 更多信息可在 openmp.org 找到。
### OpenMP 不会： 
- 自动并行化。
- 保证加速比。
- 提供免于数据竞争的自由。
## 编写 OpenMP 程序的典型步骤： 
1. 并行算法设计。
2. 相应地编写顺序程序。
3. 优化顺序程序。
4. 添加必要的 omp 指令、子句、函数。
5. 测试和调整性能。 这些步骤可能会重复，也可以尝试其他可能的算法。

## 分布式内存平台 
- 分布式内存平台由一组处理单元（或计算节点）组成。
- 每个处理单元都有自己的（独占）内存，其他处理单元无法访问（进程之间没有共享变量）。
- 处理单元使用（变体）发送和接收原语进行显式通信。
- 流行的库如 MPI 和 VPM 提供了用于进程通信的此类原语。
### 与共享内存机器的区别： 
- 对于共享内存机器，任务分配相对简单，因为全局数据不归任何线程拥有。
在分布式内存平台的并行编程中：
- 数据必须被显式地划分、分发并保持在每个进程的本地。
- 数据的分解影响工作分配。
- 对于阻塞发送和接收（进程在数据接收之前被阻塞），发送/接收对充当同步点，因此同步是隐式的。

- 对于新手来说，为分布式内存平台编写并行程序可能比为共享内存平台编写更困难。
- 如今，计算机集群拥有多个计算节点，每个节点都是多核处理器的情况非常普遍。我们需要在集群的多核节点内使用共享内存编程，并在节点之间使用消息传递 。
### 在算法设计和实现中需要认真考虑的问题： 

- 数据如何划分和分布。
- 任务（或工作）如何分配，这也与数据划分有关。
- 如何最小化通信开销。
- 数据局部性（以减少通信开销）。
- 当然，如何平衡工作负载。
##  矩阵乘法 
- 任务依赖图： 用于计算每个独立c_ij的数据依赖性。
所有输出元素c_ij都可以同时计算，没有任何依赖关系。

## 划分与分配 
- 在共享内存平台的算法设计中，我们讨论了如何划分矩阵 C 并将输入数据与计算关联起来。
- 输入矩阵是共享的，不需要在主内存中移动。
划分技术，例如一维循环、一维阻塞或一维块循环。
- 然而，对于分布式内存机器，输入数据矩阵 A 和 B 也需要被划分并分发到不同的进程。
- 关键问题： 如何划分任务和数据，以及如何将它们分配给进程，更重要的是，如何在进程之间移动数据以确保：
1. 正确的数据在正确的时间到达正确的进程 。
2. 通信开销也最小化 。
## 任务和数据关联 
- 在一维阻塞中，C 的一个块行（任务）与 A 的一个块行和整个 B 关联。
- 在二维阻塞中，C 的一个块（任务）与 A 的一个块行和 B 的一个块列关联。
## 一维数组/环上的并行矩阵乘法 (MM) 
- 假设我们有 p 个处理器，它们被组织成一维数组/环。
- 我们将每个矩阵（A、B 和 C）划分为行块矩阵，并将每个块行（A(i)、B(i) 和 C(i)）分配给一个处理器 。
- 为了计算每个 C(i)，我们需要 A(i) 和整个 B 。
- 即 C(i)=C(i)+A(i)∗B=C(i)+∑A(i,j)∗B(j) 。
- 然而，B(j) 分布在不同的处理器上，因此我们需要显式通信 。
### 图示： 
该图展示了在进行矩阵乘法时，每个处理器拥有矩阵 A 的一个块行和矩阵 B 的一个块行，为了计算 C(i)，需要所有 B(j) 块。

### 改进方案（广播）： 
在阶段 i，处理器 pi广播 B(i)。一次广播需要 logp 次发送/接收步骤。

### 改进方案（移位）： 
在每个阶段，处理器并行地向/从邻居发送/接收 B(i)。

## 二维网格/环面上的并行矩阵乘法 (MM) 
- 使用二维划分，并假设 $p=p_r \times p_c$，即 $p$ 个进程组织成一个二维网格/环面。
- 让 C(i,j) 指代一个大小为 $n/p_r \times n/p_c$ 的子矩阵，对于 A(i,j) 和 B(i,j) 类似。
- 每个 C(i,j) 需要 A(i,j) 的一个块行和 B(i,j) 的一个块列，这些都由不同的进程持有。
- 通信： 将 A(i,j) 水平移动，将 B(i,j) 垂直移动。

### 二维算法 1 (广播)： 
计算 $C(1,2)=A(1,0)*B(0,2)+A(1,1)*B(1,2)+A(1,2)*B(2,2)$ 的过程图示。
### 二维算法 2 (移位)： 
初始化： A(i,j) 向左移 i 次，B(i,j) 向上移 j 次。
## 并行矩阵乘法算法比较 
### 一维算法： 
- 矩阵被划分为块行 ($n/p \times n$)。
- 在一个并行步骤（或阶段）中，每个进程：
1. 发送一个块行 ($B(j)$) 并接收一个块行 ($\sim n^2/p$)。
2. 执行 A(i,j) ($n/p \times n/p$) 和 B(j) ($n/p \times n$) 的乘法。
3. 操作次数约为 $\sim n^3/p^2$。
4. 计算强度约为 $\sim n/p$。
- 并行步骤数为 $p$。
### 二维算法： 
- 矩阵被划分为多个子矩阵 ($n/p_r \times n/p_c$)。
- 在一个并行步骤中，每个进程：
1. 水平发送一个子矩阵 A(i,j) 并接收一个子矩阵 ($\sim n^2/p$)。
2. 垂直发送一个子矩阵 B(i,j) 并接收一个子矩阵 ($\sim n^2/p$)。
3. 执行 A(i,k) 和 B(k,j) 的乘法。
4. 操作次数约为 $\sim n^3/(p_r p_c)$。
5. 计算强度约为 $\sim n/\sqrt{p}$。
- 并行步骤数为 $\sqrt{p}$。
## 结论： 
尽管二维算法在一个并行步骤中需要移动两倍的数据量（移动 A(i,j) 和 B(i,j)），但操作次数增加了 $\sqrt{p}$ 倍。计算强度增加了 $\sqrt{p}$ 倍。由于给定问题的总工作量是固定的，并行步骤的数量也减少了 $\sqrt{p}$ 倍。这大大减少了通信开销。因此，二维并行矩阵乘法算法更好。
## 作业 5 
通过添加循环展开（展开因子 = 4）来优化你的 OpenMP 矩阵乘法程序 C=AA^T
。