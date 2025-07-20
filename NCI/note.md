## NCI (National Computational Infrastructure) 及 Gadi 超级计算机全面学习笔记
### 第一部分：NCI 简介与 Gadi 概述
#### 1.1 什么是 NCI？ 
NCI (National Computational Infrastructure) 是澳大利亚的一个世界级的、高端的超级计算和数据科学中心。它的主要职能和目标包括：
- 提供核心服务：
1. 高性能计算 (超级计算)
2. 云计算
3. 数据存储与服务
- 支持对象： 支持超过 5500 名澳大利亚研究人员。
- 研究影响： 赋能变革性研究，为政策制定提供数据支持。
- 国家效益： 交付具有国家层面效益的科研成果。
#### 1.2 Gadi：澳大利亚的旗舰计算平台 
Gadi 是 NCI 的核心，也是澳大利亚最快的基于 CPU 的研究超级计算机。
- 卓越性能：
1. 峰值性能： 超过 10 Petaflops (即每秒超过十亿亿次浮点运算)。
2. Petaflop 解释： 1 Petaflop = 10^15(一千兆) 次浮点运算每秒。
- 计算节点： 4997+ 台计算节点服务器。
- 计算核心： 超过 250,000 个计算核心，集成多种处理器架构 (如 Intel Sapphire Rapids, Cascade Lake, Skylake, Broadwell，以及 NVIDIA V100, DGX A100 GPU)。
- 总内存： 930 TB。
- 网络带宽： 200 Gb/s InfiniBand HDR 网络。
- 全球排名： 首次亮相时排名世界第 24 位，目前排名第 62 位。
### 第二部分：Gadi 文件系统与数据管理
#### 2.1 Gadi 文件系统结构
Gadi 的文件系统被划分为几个不同的逻辑区域，每个区域都有其特定的用途、性能特点和数据保留策略：
1.  /home
- 用途： 个人用户空间。
- 备份： 已备份。
- 配额： 每个用户固定配额 10 GiB。
- 适用场景： 存放重要且难以重现的文件。
- 路径示例： /home/<InstituteCode>/<UserName> (例如：/home/123/abc000)
2. /scratch
- 用途： 项目空间，由 NCI 管理。
- 备份： 不备份。
- 性能： 提供最快的读写速度。
- 数据保留策略： 超过 100 天未访问的文件将自动删除。
- 适用场景： 存放原始输出数据。
- 路径示例： /scratch/<ProjectCode>/<UserName> (例如：/scratch/ab12/abc000)
3. /g/data
- 用途： 永久磁盘空间。
- 管理： 由赞助项目或机构管理。
- 适用场景： 存放长期大型数据文件，可存储 HPC 应用程序的数据，以及长期使用的数据、输入代码、源代码等。
- 路径示例： /g/data/<ProjectCode>/<UserName> (例如：/g/data/ab12/abc000)
4. /apps
- 用途： 只读空间。
- 管理： 由 NCI 拥有和管理。
- 内容： 集中安装的软件应用程序和模块。
- 路径示例： /apps/sptware/version (例如：/apps/python3/3.10.4)
5. Massdata (mdss)
- 用途： 项目空间，由赞助项目或机构管理。
- 备份： 已备份。
- 存储介质： 使用先进的磁带存储库。
- 适用场景： 适合归档系统中不经常访问的重要文件。
- 常用命令：
```
man mdss：阅读所有 mdss 命令的手册。
mdss dmls -l：列出具有在线/磁盘缓存 (REG)、磁带 (OFF) 或两者兼有 (DUL) 状态的文件。
Mdss put/get：上传或检索 mdss 中的文件。
```
#### 2.2 数据传输工具 
NCI 提供多种数据传输工具以满足用户需求：
- Secure Copy (scp) 和 Secure File Transfer Protocol (sftp)： 常用的命令行工具。
1. 上传示例： scp -p newsample.mph jjj777@gadi-dm.nci.org.au:/scratch/c25/jjj777/
2. 下载示例： scp -p jjj777@gadi-dm.nci.org.au:/g/data/c25/jjj777/README.pdf /Users/me/Downloads/
- rsync： 用于同步文件和目录。
- aspera： 高速文件传输工具。
- aws client： 用于与亚马逊云服务交互。
- Filezilla, WinSCP： 带有图形用户界面的文件传输客户端，尤其适合 Windows 用户。
#### 2.3 存储实用工具
为了帮助用户管理存储空间，NCI 提供以下命令行工具：
- quota
1. 功能： 提供主目录 (/home) 配额和使用情况信息。
2. 命令： quota -s
- lquota
1. 功能： 提供所有连接项目空间 (在 /scratch 和 /gdata 文件系统上) 的配额和使用情况。
2. 命令： Lquota
- nci_account
1. 功能： 除了基本信息外，还提供赞助方案名称、总计算分配以及每个用户的计算时间使用情况。
2. 命令示例： nci_account -P c25 -v -p 2024.q1
- nci-files-report
1. 功能： 给出项目数据在 /scratch 和/或 /gdata 上的占用空间。
2. 命令示例： nci-files-report -P c25 -f scratch
- nci-file-expiry
1. 功能： /scratch 数据过期管理工具。
2. 命令示例： nci-file-expiry list-quarantined
### 第三部分：Gadi 计算资源与成本
#### 3.1 计算资源分配 
- 分配对象： 计算资源是分配给项目的，而不是直接分配给用户，通过特定的分配方案进行。
- 计量单位： 服务单位 (Service Unit, SU) 是衡量 Gadi 计算小时的单位。
- 运行作业前提： 一个项目必须有计算分配才能运行作业。
- 分配周期： 计算分配通常按季度进行。
- 分配调整： 分配可以在项目 CI (首席研究员) 或方案经理之间进行转移、增加、减少和移动。
- 分配结转： 如果在当前季度的前两周内通知 NCI，分配可以结转到下一个季度。
#### 3.2 服务单位 (SU) 计算：作业成本 
SU 的计费基于作业预留的资源和实际运行时间 (walltime)。预留资源是根据请求的 CPU 数量或请求的内存比例中的最大值计算的。
- 作业成本 (SU) = 队列收费率 × Max (请求的 CPU 数量, 内存比例) × 实际运行时间 (小时)
1. 队列收费率 (Queue Charge Rate)： 不同队列的收费率，通常在队列限制中列出。请注意，快速队列 (express queues) 会提高作业优先级，但也会增加作业成本。
2. 请求的 CPU 数量 (NCPUs)： 通过 PBS -l ncpus 请求的 CPU 数量。
3. 内存比例 (Memory Proportion)： 请求的内存 ÷ 每核心内存 (每节点内存 ÷ 每队列每节点 CPU 数量)。
- 作业成本示例：
1. 示例 1： 在 normal 队列中运行 1 个 CPU 30 分钟的作业，将收取 1 SU。
2. 示例 2： 在 normal 队列中运行 4 个 CPU、16 GiB 内存，壁挂时间 5 小时的作业，将收取 40 SU。
计算：4 CPU × 5 小时 × 2 SU/小时 (根据 normal 队列的收费) = 40 SU。
3. 复杂性： 一些作业可能需要较少的 CPU 但更多的内存，或需要 GPU，或需要使用快速队列，这些都会影响最终的 SU 消耗。
#### 3.3 计算资源收费策略 
计费基于 (请求的 CPU 数量, 内存单位) 中的最大值。

|队列类型|	SU / CPU / 小时|	SU / 内存单位 / 小时|	内存单位大小 (GiB)|
|------|------|------|------|
|copyq|	2|	2|	4 GiB|
|normal|	2|	2|	4 GiB|
|express|	6|	6|	4 GiB|
|hugemem|	3|	3|	32 GiB|
|megamem|	5|	5|	64 GiB|
|gpuvola|	3|	3|	8 GiB|
|dgxa100|	4.5|	4.5|	16 GiB|
|normalsr|	2|	2|	5 GiB|
|expresssr|	6|	6|	5 GiB|

- 收费计算示例：
- 示例 1： 在 normal 队列中运行 48 CPU、内存 <= 190 GiB，壁挂时间 4 小时的作业将消耗：
1. CPU 成本：2 SU/CPU/小时 × 48 CPU × 4 小时 = 384 SUs。
2. 由于是取最大值，此处主要由 CPU 决定。
- 示例 2： 在 normal 队列中运行 1 CPU、12 GiB 内存，壁挂时间 4 小时的作业将消耗：
1. 首先计算内存单位：12 GiB / 4 GiB/内存单位 = 3 内存单位。
2. 比较请求的 CPU 数量 (1) 和内存单位 (3)，取最大值 Max(1, 3) = 3。
3. SU 消耗：2 SU/内存单位/小时 × 3 内存单位 × 4 小时 = 24 SUs。
### 第四部分：连接到 Gadi
4.1 两种主要连接方式 
- Secure Shell Protocol (SSH)：
1. 基于 Linux 环境。
2. 作业以批处理形式提交。
3. 是使用 HPC 最正规和最有效的方式。
- Australian Research Environment (ARE)：
1. 提供交互式环境。
2. 适用于非批处理作业。
3. 易于使用。
#### 4.2 SSH 连接指南 
- Linux 或 macOS：
打开终端并输入命令：ssh username@gadi.nci.org.au
输入密码 (注意：输入时屏幕上不会显示任何字符)。
- Windows：
推荐使用 PuTTY (www.putty.org) 或 MobaXterm (www.mobaxterm.mobatek.net) 等 SSH 客户端。
#### 4.3 Australian Research Environment (ARE) 连接 
- 直接通过网页浏览器访问：https://are.nci.org.au/
- 使用 NCI 用户名和密码登录。
- ARE 提供了多种特色应用程序：
```
Jupyter Lab (用于启动 Jupyter Notebook 实例)
Virtual Desktop (常规和 GPU 虚拟桌面实例)
Gadi Terminal (直接连接到 Gadi 的终端)
RStudio (R 集成开发环境，支持 Docker 镜像)
```
#### 4.4 登录节点使用注意事项 
登录节点是用户首次通过 SSH 连接 Gadi 时的入口。您可以在登录节点上执行以下操作：
1. 准备作业提交脚本。
2. 提交和监控作业。
3. 编辑文件、构建程序、编译小型应用程序、在您的主目录/项目空间安装软件等。
4. 下载/上传少量数据。
- 重要警告：
为确保公平使用和系统稳定性，任何超过以下限制的进程将被立即终止：

使用超过 30 分钟的 CPU 时间 和/或

使用超过 4 GiB 内存。

- 强烈建议：在登录节点上运行计算密集型作业是一个非常糟糕的主意！ 这些任务应该通过作业调度系统提交到专用的计算节点上执行。
### 第五部分：使用 Gadi 的重要考量 
使用 Gadi 这样的高性能计算资源，需要记住以下几点：
- 高需求： Gadi 是一个国家级资源，通常处于高需求状态，合理规划和使用资源至关重要。
- 昂贵： 运营和维护如此庞大的超级计算机成本高昂。
- 能源密集型： 超级计算机消耗巨大的电能。
- 有人在支付费用： 您所使用的计算资源是由政府、研究机构或资助者支付的。因此，高效、负责任地使用资源是每个用户的责任。