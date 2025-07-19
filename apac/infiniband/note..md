## InfiniBand 技术深度解析：从基础到高级应用
### 第一部分：InfiniBand 基础与核心概念
#### 1.1 什么是 InfiniBand？ 
InfiniBand 是一种高性能互连技术，用于连接处理器节点和 I/O 节点，形成一个系统区域网络（System Area Network, SAN）。它的架构独立于主机操作系统（OS）和处理器平台。
- 主要特点：
1. 开放的行业标准规范。
2. 定义了输入/输出架构。
3. 提供点对点双向串行链路。
- 网络拓扑示例：
在一个 InfiniBand 网络中，计算服务器（Compute Servers，包含 CPU 和 GPU 节点）通过 InfiniBand HCA（Host Channel Adapter）以高速率（例如 400Gb/s NDR）连接到 InfiniBand 交换机（Switches）。这些交换机再连接到存储前端/后端（Storage Front/Back-End）系统。同时，通过网关（Gateway to Ethernet）可以连接到以太网环境。
#### 1.2 InfiniBand 关键特性 
InfiniBand 具备以下核心优势，使其成为高性能计算和数据中心的理想选择：
- 简化管理 (Simplified Management): 得益于子网管理器（SM）。
- 高带宽 (High Bandwidth): 提供极高的数据传输速率。
- CPU & GPU 卸载 (CPU & GPU Offloads): 减轻主机处理器负担。
- 超低延迟 (Ultra-Low Latency): 实现极快的响应时间。
- 网络横向扩展 (Network Scale-Out): 支持大规模集群。
- 流量优先级 (Traffic Prioritization): 确保关键数据优先传输。
- Fabric 弹性 (Fabric Resiliency): 提高网络的可靠性和容错能力。
- 服务质量 (Quality of Service): 保证不同服务等级的数据传输需求。
- MPI & NCCL 超高性能 (MPI & NCCL Super Performance): 对并行计算和深度学习通信进行优化。
- 支持多种拓扑结构 (Variety of Supported Topologies): 适应不同的部署需求。
#### 1.3 CPU 卸载和 RDMA (Remote Direct Memory Access) 
InfiniBand 架构通过最小化 CPU 干预来支持数据包传输，从而更有效地利用主机 CPU 资源并提供卓越的低延迟性能。这得益于：
1. RDMA 支持: 允许数据直接从一个节点的内存传输到另一个节点的内存，无需 CPU 参与。
2. 硬件加速的传输协议 (Transport offload): 传输层协议在硬件中实现。
3. 内核旁路 (Kernel bypass): 数据传输绕过操作系统内核。
4. 零拷贝 (Zero copy): 数据直接从应用程序内存复制到网络适配器，无需中间缓冲区。
5. 硬件 - 主机通道适配器 (Host Channel Adapter, HCA): 专门的硬件实现这些功能。
- 工作原理示意： 数据从 Server 1 的应用程序缓冲区直接通过 RDMA 传输到 Server 2 的应用程序缓冲区，中间经过 Switch，整个过程绕过了两端的操作系统内核和 CPU。
### 第二部分：InfiniBand 架构与组成
#### 2.1 InfiniBand 架构层级 
InfiniBand 采用多层处理堆栈来传输数据，每层描述其特定的功能、协议和设备。每一层向上层提供服务，并向下层发出服务请求。
- 上层 (Upper Layer)
- 传输层 (Transport Layer)
- 网络层 (Network Layer)
- 链路层 (Link Layer)
- 物理层 (Physical Layer)
#### 2.2 InfiniBand 网络栈 
InfiniBand 网络栈提供 CPU 卸载功能，并通过各种服务和协议提供更大的适应性。
- 上层协议 (Upper Layer Protocols): 负责 I/O 操作。
- 传输层 (Transport Layer): 处理 IBA (InfiniBand Architecture) 操作和消息队列对 (QP)。
- 网络层 (Network Layer): 负责子网路由 (Subnet Routing) 和跨子网路由 (Inter Subnet Routing, GRH)。
- 链路层 (Link Layer): 处理数据包中继 (Packet Relay)、链路编码 (Link Encoding)、媒体访问控制 (MAC) 和流量控制 (Flow Control)。
- 物理层 (Physical Layer): 负责端点节点之间的物理连接，包括交换机 (Switch-L2) 和路由器 (Router-L3)。
#### 2.3 数据包结构 (Data Packet Structure)
InfiniBand 数据包由多个部分组成，其内部结构与协议层级对应：
1. 起始定界符 (Start Delimiter)
2. 数据符号 (Data Symbols): 包含了实际的数据包。
3. 结束定界符 (End Delimiter)
4. 空闲符号 (Idles)
- 数据包内部详细结构：
```
LRH (Local Routing Header): 8 字节，链路层。
GRH (Global Routing Header): 40 字节，网络层。
BTH (Base Transport Header): 12 字节，传输层。
ETH (Extended Transport Header): 可变大小，传输层。
Payload (有效载荷): 256-4096 字节，实际数据。
ICRC (Internal Cyclic Redundancy Check): 4 字节，链路层。
VCRC (Virtual Channel Cyclic Redundancy Check): 2 字节，链路层。
```
#### 2.4 InfiniBand Fabric 组件
一个典型的 InfiniBand Fabric 主要由以下组件构成：
1. 子网管理器 (Subnet Manager, SM): 负责网络的发现、配置和管理。通常有主（SM Master）和备用（SM Standby）实例。
2. 主机/服务器 (Host/Server): 包含 CPU 和 GPU 的计算节点。
主机通道适配器 (Host Channel Adapter, HCA): 主机连接到 InfiniBand 网络的接口卡。
3. 交换机 (Switch): 用于连接各个节点和路由数据。
网关 (Gateway): 用于连接 InfiniBand 子网到其他网络（如以太网）。
4. 路由器 (Router): 用于连接不同的 InfiniBand 子网。
#### 2.5 Fabric 全局唯一标识符 (GUIDs) 
GUID (Globally Unique Identifier) 是一个 64 位由厂商烧录在硬件上的唯一地址，在设备重启后仍然保持不变。
- GUID 的三种类型：
1. 节点 GUID (Node GUID): 标识整个节点。
2. 端口 GUID (Port GUID): 标识 HCA 上的特定端口。
3. 系统镜像 GUID (System image GUID): 标识共享相同系统镜像的设备（例如，同一机箱内的多个交换机）。
### 第三部分：寻址与管理
#### 3.1 Layer 2 寻址 - LID (Local Identifier) 
LID (Local Identifier) 是一个 16 位的 Layer 2 地址，由子网管理器 (SM) 在端口配置期间分配。
- HCAs 的每个端口都会被分配一个 LID。
- 交换机：
1. 1 IC (Integrated Circuit) 交换机被分配一个单一的 LID。
2. 总线控制器交换机 (Director switches) 的每个交换机模块在机箱中被分配一个 LID。
- 通过 ibstat 命令可以查看 HCA 的详细信息，包括分配给 HCA 端口的 Base lid。
#### 3.2 Layer 3 寻址 - GID (Global Identifier) 
GID (Global Identifier) 是一个 128 位字段，位于全局路由头 (GRH) 中，用于标识单个端点端口或多播组。
- GID 在多个子网中是全局唯一的。
- GID 的结构： 基于一个 64 位子网前缀与端口 GUID 组合而成，类似 IPv6 地址。
- 每个 HCA 端口会自动分配一个默认的 GID，该 GID 只能在本地子网中使用。
#### 3.3 子网管理器 (SM) - 规则与角色 
子网管理器 (SM) 是 InfiniBand Fabric 的核心，负责：
- 发现拓扑 (Discovering the topology): 自动识别网络中的所有设备和连接。
- 分配 LID 给设备 (Assigning LIDs to devices): 为 HCA 端口和交换机分配本地标识符。
- 计算和编程交换机转发表 (Calculating and programming switch forwarding tables): 根据拓扑和路由规则生成并配置交换机的转发路径。
- 管理 Fabric 中的所有元素 (Managing all the elements in the fabric): 确保网络的正常运行和配置。
- 监控子网中的变化 (Monitoring changes in subnet): 实时检测设备加入、离开或状态变化，并相应地更新拓扑和路由。
- 静态路由算法 (Static routing algorithms): 用于构建初始的转发表。
- 重要规则：
1. SM 可以在 Fabric 中的任何节点上实现。
2. 一个子网中只允许有一个主 SM (master SM)，通常会有一个备用 SM (standby SM) 提供容错能力。
#### 3.4 简化管理 (Simplified Management) 
InfiniBand 的管理通过子网管理器 (SM) 大大简化：
1. SM 是一个运行和管理 Fabric 的程序。
2. 任何 InfiniBand Fabric 都有自己的单一（master）SM。
3. SM 使 Fabric 管理非常简单，支持即插即用 (Plug & Play) 的终端环境和集中式路由管理器 (Centralized route manager)。
#### 3.5 管理元素 (Management Elements) 
一个 InfiniBand 子网包含以下管理元素：
1. 节点 (Node): 任何受管实体，如交换机、HCA、路由器。
2. 管理器 (Manager, SM): 主动实体，发出命令和查询。
3. 代理 (Agent): 大部分是被动实体，响应管理器的请求（但也能发出源陷阱）。
4. 管理数据报 (Management Datagram, MAD): 用于管理器-代理通信的标准消息格式。
#### 3.6 Fabric 分段 - 分区 (Partitions) 
分区描述了 Fabric 内一组可以相互通信的终端节点。
- 当端口被分配到分区时，其成员资格类型可以设置为：
1. 完全成员 (Full membership): 可以在分区内与任何其他节点通信。
2. 受限成员 (Limited membership): 只能与分区内具有完全成员资格的节点通信。
- 节点可以同时是多个分区的成员。
- PKEY (Partition Key): 是 BTH 头中的一个字段，用于确定分区成员资格。
- 默认分区键是 0x7FFF。MSB (最高有效位) 定义了完全/受限成员资格。
1. 0x7FFF 表示受限和完全 0xFFFF
2. 0x0001 表示受限和完全 0x8001
- 使用分区目的： 用于在 Fabric 中隔离不同的工作负载或用户，提供安全性和多租户能力。
### 第四部分：性能与优化
#### 4.1 InfiniBand 带宽世代 
InfiniBand 随着时间推移不断发展，提供更高的带宽：

|年份|	名称|	带宽 (Gb/s)|	单通道速度 x 通道数|
|------|------|------|------|
|2002|	SDR|	10|	|
|2008|	QDR|	40|	2.5×4|
|2011|	FDR|	56|	10×4|
|2015|	EDR|	100|	14×4|
|2018|	HDR|	200|	25×4|
|2021|	NDR|	400|	50×4|
|2023|	XDR|	800|	100×4|
|2025|	GDR|	1600|	200×4|

- 带宽计算公式： 链路速率 = 单通道速度 × 组合通道数 (宽度)。
#### 4.2 InfiniBand 带宽：HDR 示例 
以 200Gb/s HDR 为例：由 4 个 50Gb/s 的单通道组成，实现 50Gb/s×4=200Gb/s 的总带宽。
#### 4.3 可扩展性与灵活性 
InfiniBand 为数据中心提供了卓越的扩展能力和灵活性：
- InfiniBand 使得数据中心易于扩展，具有极大的灵活性。
- 单个 InfiniBand 子网可扩展到多达 48,000 个受管节点。
- 一个 InfiniBand 网络（通过多个子网）可扩展到一百万（1,000,000）个节点以上。
#### 4.4 服务质量 (Quality of Service, QOS) 
QoS 是指为不同的应用、用户或数据流提供不同优先级的服务能力。
- QoS 的实现方式：
1. 在适配器级别定义 I/O 通道。
2. 在链路级别定义虚拟通道 (Virtual Lanes)。
QoS 允许控制网络上的拥塞，确保关键应用和数据流获得所需的带宽和低延迟。
#### 4.5 自适应路由 (Adaptive Routing, AR) 
启用自适应路由 (AR) 后，路由算法会识别一组具有相同代价（最小跳数）通往目标 LID 的端口，并将其安装到交换机转发表中。
1. 对于每个连接，交换机会动态选择最不拥塞的端口。
2. 如果端口发生故障或出现拥塞，数据包会被重新路由到组内的另一个端口。
- 自适应路由的性能优势：
在 VASP (Vienna Ab initio Simulation Package) 和 BSMbench (计算粒子物理基准测试) 等应用中，自适应路由相比静态路由能显著提升作业处理能力和 GFlops 性能。
#### 4.6 SHARP - 网络内计算 (In-Networking Compute) 
SHARP (Scalable Hierarchical Aggregation and Reduction Protocol) 是一种基于 InfiniBand 交换机硬件中专用处理单元的技术。
- SHARP 将网络从根本上转变为一个协处理器，参与到应用程序的运行。
通过将计算从 CPU 卸载到网络交换机，SHARP 提升了并行计算应用的性能。
- SHARP 减少了数据在网络中传输的次数，从而增强了应用程序的性能。
- SHARP All Reduce 性能优势：
1. 7 倍更高的 MPI 性能: 显著降低 Allreduce 延迟。
2. 2.5 倍更高的 AI 性能: 显著提升 NCCL Allreduce 带宽。
### 第五部分：InfiniBand 主流应用与最新平台
#### 5.1 InfiniBand 主要用例 
InfiniBand 广泛应用于以下领域，这些领域对高带宽、低延迟和可扩展性有严格要求：
```
数据中心 (Data Centers)
云计算 (Cloud Computing)
高性能计算 (HPC)
机器学习 (Machine Learning)
人工智能 (Artificial Intelligence)
```
#### 5.2 NVIDIA Quantum-2 InfiniBand 平台 (400G) 
NVIDIA Quantum-2 InfiniBand 平台是面向下一代高性能计算和 AI 基础设施的解决方案，提供 400G 的速率。主要组件包括：
- ConnectX-7 适配器 (ConnectX-7 Adapter):
```
400G InfiniBand 接口。
PCIe Gen5 接口。
可编程数据路径。
支持网络内计算。
```
- Quantum-2 交换机 (Quantum-2 Switch):
1. 提供 64 端口 400G InfiniBand 或 128 端口 200G InfiniBand 配置。
2. 支持网络内计算。
- BlueField-3 DPU (Data Processing Unit):
```
400G InfiniBand 接口，集成 Arm 核心。
PCIe Gen5, DDR5。
AI 应用加速器。
可编程数据路径。
支持网络内计算。
```
- 线缆 (Cable):
1. 铜缆 (Copper Cables)。
2. 有源铜缆 (Active Copper Cables)。
3. 光收发器 (Optical Transceivers)。
该平台通过集成 400G 带宽、先进的 PCIe 技术、强大的网络内计算和 DPU，为数据中心和 AI 工作负载提供了无与伦比的性能。