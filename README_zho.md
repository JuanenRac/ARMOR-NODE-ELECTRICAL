<p align="center">
  <img src="images/ARMOR_BANNER.svg" alt="ARMOR-NODE-ELECTRICAL banner" width="100%">
</p>

# ⚡ ARMOR-NODE-ELECTRICAL

<p align="center">
  <a href="README.md">🇺🇸 English</a> |
  <a href="README_spa.md">🇪🇸 Español</a> |
  <a href="README_fra.md">🇫🇷 Français</a> |
  <a href="README_ita.md">🇮🇹 Italiano</a> |
  <a href="README_deu.md">🇩🇪 Deutsch</a> |
  🇨🇳 <b>简体中文</b> |
  <a href="README_jpn.md">🇯🇵 日本語</a> |
  <a href="README_por.md">🇵🇹 Português</a>
</p>

### 电气节点：读取住宅交流和直流电网的电压、电流、功率和电能，并保存其开关规则（已在电脑上测试的核心和可编译的固件；尚未在电路板上运行过）

<p align="center">
  <img src="https://img.shields.io/badge/License-GPL%203.0--or--later-blue.svg" alt="GPL 3.0 or later">
  <img src="https://img.shields.io/badge/Language-C%2B%2B17-00599c.svg" alt="Language">
  <img src="https://img.shields.io/badge/Meters-PZEM--004T%20%2F%20017-ffb020.svg" alt="Meters">
  <img src="https://img.shields.io/badge/Checks-376%2C195-2ea44f.svg" alt="Checks">
  <img src="https://img.shields.io/badge/Tests-382264%20checks-2ea44f.svg" alt="Tests">
  <img src="https://img.shields.io/badge/Version-0.5.1-0f6cbd.svg" alt="Version">
  <img src="https://img.shields.io/badge/Maturity-functional-00E5FF.svg" alt="Maturity">
</p>

---

**诚实性检查 - 今天真正能运行的部分：** 核心（PZEM-004T v3 和 PZEM-017 电表的帧、轮询它们的总线、节点的消息、模拟输入和切换规则：19 个测试程序中的 382,264 项检查，其中 375,036 项针对切换规则）在计算机上用替身电表和接触器测试，它生成的消息被 ARMOR-COMMON 接受。节点固件（设置、串行线路、面板的电表和读数页面、蓝牙、签名更新）可在 ESP-IDF 5.5.5 容器中为两种板子构建。**它从未在板上运行，没有连接任何电表或接触器，这里的任何东西都不会切换任何设备。**

---

## 📋 速览

| | |
|---|---|
| 版本 | `0.5.1` |
| 成熟度 | 可用 |
| 角色 | 电气现场节点：电表、输入和开关操作（家族：Field Nodes，上级：ARMOR-COMMON） |
| 技术栈 | C++17, ESP-IDF 5.5.5, ESP32-S3 |
| 测试 | 382264 项检查（19 个主机程序和 5 项 Python 检查，`ctest`） |
| 设备 | PZEM-004T v3 和 PZEM-017 电表（最多 16 台）、模拟输入、继电器；开关操作默认关闭 |
| 运行位置 | ESP32-S3 电路板（以太网或 Wi-Fi）；主机测试可在任何计算机上运行 |
| 真实硬件 | 暂无：没有制作板子，也没有连接电表或接触器；不会切换任何设备 |
| 许可证 | GPL-3.0-or-later |

**跳转到：** [概述](#-概述) · [架构](#-架构) · [安全](#-安全与防护) · [接口](#-接口) · [配置](#-配置) · [快速开始](#-快速开始) · [测试](#-测试) · [故障排除](#-故障排除) · [结构](#-仓库结构) · [相关项目](#-相关项目)

## 🚦 项目状态

每一行是本仓库的一个部分，灯号表示它今天做到了哪一步。内容依据仓库能够展示的证据（测试、构建、运行），而不是计划。

🟢 已完成并验证 · 🟡 可用，有已知限制 · 🟠 部分完成或尚未验证 · 🔴 未完成或不可用

| 领域 | 状态 | 在这里的含义 |
|---|---|---|
| 软件 | 🟢 | 固件可在 ESP-IDF 5.5.5 容器中为两块板（Wi-Fi 和以太网）构建。 |
| 测试 | 🟢 | 19 个主机程序中的 382,264 项检查通过（`ctest`），其中 375,036 项针对切换规则；电表和接触器均为模拟。 |
| 协议与数据 | 🟡 | PZEM-004T v3 和 PZEM-017 的帧与读数已对照文档中的帧和录制的样本检查；目前还没有电表响应过。 |
| 界面 | 🟡 | 网页面板的电表和读数页面已在浏览器中对模拟节点测试过，尚未在板子上测试。 |
| 硬件 | 🔴 | 尚未制作板子，也没有连接电表或接触器：还没有在真实设备上运行过。 |
| 安全 | 🟡 | 设置码、限制错误密码次数的登录、蓝牙 PIN 和签名更新已在主机上测试；没有经过独立审查。 |
| 更新 | 🟠 | 哈希和 Ed25519 签名检查以及回滚逻辑在主机上通过；空中更新从未在板子上运行过。 |
| 写入设备 | 🟠 | 切换接触器的规则只对照模拟接触器测试过；这里还没有切换过任何真实设备。 |
| 集成 | 🟢 | 节点的消息已对照 ARMOR-COMMON 的共享模式和一致性向量验证。 |
| 文档 | 🟢 | 八种语言、固件指南、接线说明和更新日志均为最新。 |

## 🎯 概述

* **读取电表：** Peacefair **PZEM-004T v3**（交流）和 **PZEM-017**（直流）的 Modbus RTU 帧：CRC、读寄存器的请求以及应答的解码（电压、电流、功率、电能、频率、功率因数、报警）。只构造读请求：向电表写入的请求（地址、阈值、电能清零）在任何地方都不构造。不匹配的应答，或包含任何真实电表都给不出的数值的应答，会被拒绝。
* **总线：** 带多个电表的线路逐个轮询并带超时；保留每个电表最后一次正常读数，静默的电表在十秒后不再发布。
* **消息** `armor/electrical/<节点>/state`：每个通道（一条回路、一条线路、电网入口、一条直流母线）一项，含交流或直流、电压、电流、功率、电能、频率、功率因数、节点所见的开关状态和报警；它属于共享契约，只带状态，绝不带命令。
* **开关规则，** 与任何硬件无关：两个电源之间转换的控制器，绝不同时指令两者，要求开关操作被允许、节点在刚才已布防、两个接触器在整个死区时间内都被确认断开，并把没有显示所指令状态的接触器当作故障，直到被确认才消除。**开关默认关闭，没有任何硬件被驱动。**
* **对开关的命令（未启用）：** 状态消息可以携带节点各开关的状态，共享契约中有一条命令（`arm`，然后带一次性令牌的 `close_a` 或 `close_b`、`open`、`acknowledge`）以及节点的应答；核心按上述规则读取并应答，已用共享向量和模拟接触器测试。没有任何固件镜像包含它，因此不能驱动任何接触器；除非已启用（`ARMOR_ELECTRICAL_SWITCHING=1`）且代理的 ACL 允许，ARMOR-SERVER 不会发送命令。
* **在哪里显示：** ARMOR-STUDIO 的*电气设计器*绘制住宅的电网，并在每个元件上显示其通道的测量值；ARMOR-SERVER 保存读数、历史和合计。
* **节点固件**（Wi-Fi 的 ESP32-S3-WROOM-1 N16R8，或有线的 Waveshare ESP32-S3-ETH）：设置、供最多十六块电表使用的一条串口线路，以及与其他节点相同的网页面板（设置、用户、Wi-Fi、代理、无线更新、HTTPS），另有自己的电表页和读数页（八种语言）。只读取：开关规则没有链接进去。参见[固件](docs/NODE_FIRMWARE.md)。
* **通过蓝牙用手机配置：** 与雷达节点相同的通道。ARMOR 应用会以 `ARMOR-XXXXXX` 找到节点，并使用面板的用户和设置码来设置其名称、Wi-Fi、地址、代理和蓝牙模式（[协议](docs/BLE_PROVISIONING.md)）。除非另行设置，它只在节点还没有用户时监听。无线部分从未在电路板上运行过。
* **签名的无线更新：**节点只会从 GitHub 安装 SHA-256 匹配且带有维护者有效 Ed25519 签名的镜像（公钥在固件中，私钥从不离开维护者）；管理员在面板中手动上传的文件仍然被接受。面板的页面和消息有八种语言。
* **带 PIN 的蓝牙配对：**六位 PIN 由设置码派生，打印在 USB 控制台上并向管理员显示在面板中；手机必须输入它才能写入节点的设置。
* **尚未完成：** 在电路板上运行的固件、硬件、对真实设施的任何测量以及任何开关操作。
* **只有家中才能使用面板：**面板默认只响应私有网络（或所有者列出的网络，如 VPN），拒绝会把保存者自己挡在外面的列表，按人和按地址统计错误密码，并可在服务器代理之后工作，从而无需公开节点就能在家外访问。
* **估算值会标明：** 由电流输入生成的通道带有 `power_estimated`，Studio 和服务器会把它的功率显示为估算值（由电流和额定电压得出的视在功率），并标记包含它的电网数值。
* **可选的与代理加密连接(TLS):** 默认关闭,因此节点仍像以前一样使用未加密的 MQTT。代理页面中的一个开关和一个用于填写本安装自有证书颁发机构证书(PEM,最大 2 KB)的框,会让节点用 `mqtts://` 连接、只信任该机构,并检查代理证书是否为其所到达的地址而签发;不是单个 PEM 证书的内容会被拒绝。已有主机测试覆盖;握手尚未在电路板上运行。
* **健康报告：** 节点大约每分钟发布一次：已运行多久、上次为何重启、可用内存、连接与 Wi-Fi 信号、连接代理的次数以及记录的最后一个错误（`armor/health/<节点>`，即 ARMOR-COMMON 的 `node_health` 契约）。生成它的代码由所有节点共享并在计算机上测试；尚未在开发板上运行。
* **由服务器控制：** Studio 可以干净地重启节点、将其暂停至多一小时（自动恢复）、让其工作进入待机（无期限，重启后仍保持，节点保持连接并继续应答）、让它显示自己、请求诊断快照，并把它标记为维护中（`armor/control/<节点>/...`，ARMOR-COMMON 的 `node_control` 契约）。每条命令分两步，使用只属于该操作的一次性令牌，只有服务器可以在代理上写入，并且没有恢复出厂设置、擦除或深度睡眠。节点处于暂停或待机时，所有继电器保持断开，无论谁请求（面板、代理或中央）都不会动作；电表仍继续读取。 该代码由所有节点共用，并已在计算机上测试；尚未在电路板上运行过。
* **说明自身新鲜度的读数：** 每个电表通道都带有其读数的时间和对该电表失败的请求次数，因此服务器能区分真实的 0 W 与已停止应答的电表；电表任务卡死时，看门狗会重启电路板。
* **无需线缆的诊断：** 在一次不干净的启动之后（崩溃、看门狗、电源跌落、内存分配失败），健康报告会在十分钟内说明原因；崩溃会留下核心转储（core dump），可在面板中查看、下载或删除；已保存的设置会在每次固件安装前被复制，并在更新损坏它们时恢复；管理员还可以通过服务器，用一次性令牌向节点索取诊断包或其日志。
* **继电器互锁：**一个继电器可以指定另一个绝不能同时闭合的继电器（同一负载的两路电源、电机的两个转向）；在另一个闭合时要求闭合此继电器，无论来自面板、broker 还是中心，都会被拒绝，节点也绝不会自行断开另一个。已在计算机上测试；尚未在开发板上运行。
* **只读方式读取电表自身的设置:** 在第一次成功读数之后,以及每隔 `bus.check_settings_s`(600 秒,0 = 从不),节点会在读取测量值的同一轮中读取 PZEM 的保持寄存器(004T v3:功率阈值和地址;017:电压阈值、地址和分流器),并在保存的地址与所用地址不一致时发出警告;不会向电表写入任何内容。
* **可选的已存储机密封存:** 默认关闭(`security.seal_secrets`,或服务器的 `nodes.nvs_encryption`)。开启后,节点保存的密码、令牌、密钥、PIN 和密码哈希会用 ChaCha20-Poly1305 封存,其余内容仍可读取;开启或关闭时会无损重写,即使节点中途停止也不会丢失。不会烧写 eFuse,闪存也不加密:密钥在同一芯片的一个分区中,因此**不能**防范持有电路板的人;在该分区出现之前安装的节点会保持原样,直到用新的分区表重新烧录。

## 🧩 在 A.R.M.O.R. 中的角色

ARMOR-NODE-ELECTRICAL 通过电表测量住宅的交流和直流网络，并保存在电源之间切换的规则。它把读数发布给 [ARMOR-SERVER](https://github.com/JuanenRac/ARMOR-SERVER)，由后者保存历史和每日能量并触发报警；[ARMOR-STUDIO](https://github.com/JuanenRac/ARMOR-STUDIO) 的*电气*菜单和*电气设计器*以及 [ARMOR-ANDROID-CONTROL](https://github.com/JuanenRac/ARMOR-ANDROID-CONTROL) 的电气界面显示这些内容。切换已经设计好（预备、一次性令牌、互锁、两个接触器的确认）但未启用。

| 名称 | 方式 | 用途 |
|---|---|---|
| ARMOR-SERVER | MQTT `armor/electrical/<id>/{state,command,result}` | 接收通道并下达转换命令（仅当三处都开启开关操作时） |
| ARMOR-STUDIO | 电气菜单、Electrical Designer | 显示通道和报警并打开节点面板 |
| ARMOR-ANDROID-CONTROL | Bluetooth | 在现场设置节点并显示通道 |
| ARMOR-COMMON | 电气合约 | 消息按其模式校验；公共固件由此复制 |

## 🏛️ 架构

<p align="center">
  <img src="images/architecture.svg" alt="ARMOR-NODE-ELECTRICAL 的架构：谁调用它、它包含什么、它依赖什么" width="100%">
</p>

## 🖼️ 屏幕截图

在测试台上以 1920×1080 截取，使用示例数据，背后没有真实的安装。

<table>
<tr><td width="50%" valign="top"><img src="images/screenshots/overview.png" alt="概览：节点状态一目了然——网络、代理、固件及其上报内容。" width="100%"><br><sub>概览：节点状态一目了然——网络、代理、固件及其上报内容。</sub></td><td width="50%" valign="top"><img src="images/screenshots/meters.png" alt="电表：节点读取的电能表和模拟通道，各有名称和量程。" width="100%"><br><sub>电表：节点读取的电能表和模拟通道，各有名称和量程。</sub></td></tr>
<tr><td width="50%" valign="top"><img src="images/screenshots/readings.png" alt="读数：节点此刻读取到的内容，以及每个数值的时效。" width="100%"><br><sub>读数：节点此刻读取到的内容，以及每个数值的时效。</sub></td><td width="50%" valign="top"><img src="images/screenshots/zigbee.png" alt="Zigbee 无线电：通往协调器模块的串口桥接、允许使用它的客户端以及桥接状态。" width="100%"><br><sub>Zigbee 无线电：通往协调器模块的串口桥接、允许使用它的客户端以及桥接状态。</sub></td></tr>
</table>

## 🔒 安全与防护

| 威胁 | 防御 |
|---|---|
| 明文或编译进固件的密码 | PBKDF2-HMAC-SHA256，10 000 轮；固件中没有密码（没有用户的节点会进入设置）；最多 4 个用户（管理员或查看者），8 到 64 个字符；最多 6 个会话，空闲 30 分钟 |
| 接管刚刷写的节点 | 设置需要启动码：构建时的码（`CONFIG_ARMOR_SETUP_CODE`），或保存在闪存中并显示在 USB 控制台上的随机码；有机群密钥时为 `HMAC-SHA256(密钥, MAC)`；它也是设置用 Wi-Fi 的密钥（WPA2，绝不桥接） |
| 从互联网访问面板 | 默认只响应家庭的私有网络（10/8、172.16/12、192.168/16、169.254/16、127/8，以及某些 VPN 使用的 100.64/10）；设置 `web.admin_sources` 可列出其他 IPv4 地址和网络（8 项，120 个字符）；除公开身份 `GET /api/v1/session` 外，其余一律拒绝；会把保存者自己锁在外面的列表会被拒绝 |
| 通过 HTTP 明文访问面板 | 在 443 端口提供 HTTPS，证书由节点自己生成（自签名、存于闪存，显示 SHA-256 指纹以便比对）；面板可设为 http、https 或两者；会话 Cookie 始终为 HttpOnly 和 SameSite=Strict（TLS 下还有 Secure） |
| 来自 GitHub 的伪造固件 | 只有 SHA-256 匹配且带有维护者密钥有效 Ed25519 签名的镜像才会被安装（`no_checksum`、`no_signature`、`bad_signature`、`checksum_mismatch`、`wrong_firmware`、`not_newer`、`version_mismatch` 会拒绝）；管理员手动上传不做校验：这是明确的限制 |
| 劫持蓝牙设置 | 由设置码派生的 6 位 PIN，显示在 USB 控制台并对管理员可见；手机必须先输入才能写入设置；设置窗口时长为 `CONFIG_ARMOR_BLE_SETUP_MINUTES`（默认 10，范围 1 到 120） |
| 造成危害的远程命令 | 两步操作，使用与该操作绑定的一次性令牌；只有服务器能向代理写入这些主题；暂停最长一小时；远程命令没有恢复出厂、擦除和深度睡眠 |
| 拿到电路板的人读取存储的机密 | 可选（默认关闭，开关为 `security.seal_secrets` 或服务器的 `nodes.nvs_encryption`）：用 ChaCha20-Poly1305 保护密码、令牌、密钥、PIN 和哈希。它不加密闪存也不烧写 eFuse：密钥位于同一芯片的分区中，因此无法防御拿到电路板的人 |
| 与代理的明文连接 | 可选 TLS（`mqtts://`），使用所有者自己的证书颁发机构（PEM 最大 2 KB）并检查名称；默认关闭。握手只在主机测试中运行过，没有在电路板上运行 |
| 电路板上的危险市电或直流电 | 节点只读取 PZEM 模块（封闭模块，串口线已隔离），电路板上从不带市电；负责开关的是单独的一级，在台架上用灯泡和隔离变压器测试 |
| 两个电源同时闭合 | 转换互锁从不同时命令两者：需要允许开关操作、节点刚刚布防，并且在整个死区时间内两个接触器都确认为断开（`dead_time_ms` 默认 2000）；未显示所命令状态的接触器是持续到被确认的故障（`kDidNotOpen`、`kDidNotClose`）；接触器之间仍必须有机械互锁 |
| 被重放的命令，或没人要求的开关操作 | `arm` 返回一个 64 位一次性令牌，`close_a` 或 `close_b` 必须在 `arm_window_ms` 内携带它；命令从不被保留；最多 4 个开关；除非固件允许（`switching_enabled`）、服务器以 `ARMOR_ELECTRICAL_SWITCHING=1` 运行且代理的访问列表对该节点开放，否则开关操作处于关闭状态 |
| 故障、反馈丢失或开关操作关闭 | 每个控制器在故障、反馈丢失或开关操作关闭时都以所有线圈断开作答（故障即断开；断开对负载是否安全由安装方决定）；消息中的状态是辅助触点显示的状态，而不是被命令的状态；继电器对相互联锁，节点从不自行断开其搭档 |
| 过期读数被当作真实值显示 | 无响应的电表在 10 秒后不再发布；每个通道带有读数的时效和失败请求次数；电表任务卡死会由看门狗重启电路板；每隔 `bus.check_settings_s`（600 秒，0 = 从不）读取电表设置（只读），若地址变化则报告 |

市电（230 V 交流）和电池组的直流电可能致命并引发火灾。[docs/SAFETY.md](docs/SAFETY.md) 说明了设计的假设；它不是安装设计，也不能取代所在国家的法规或合格的安装人员。目前这里的一切都不会控制任何开关。

## 🔌 接口

| 部分 | 作用 |
|---|---|
| MQTT | `armor/electrical/<id>/state`（每路输入一个通道：交流或直流、电压、电流、功率、电能、频率、功率因数、开关状态、报警；ARMOR-COMMON 合约）、`armor/electrical/<id>/command` 和 `/result`、`armor/device/<id>/<继电器>/{state,set}`、`armor/health/<id>`、`armor/control/<id>/...` |
| 命令 | `arm`、`close_a`、`close_b`（带令牌）、`open` 和 `acknowledge`（清除故障） |
| 电表 | Peacefair PZEM-004T v3（交流）和 PZEM-017（直流），通过 Modbus RTU，一条串口线上最多 16 台；只会构造读取请求（写地址或阈值的请求根本不存在）；模拟输入、触点和继电器在底板上；还有 Zigbee 桥（[docs/ZIGBEE.md](docs/ZIGBEE.md)） |

## ⚙️ 配置

| 变量 | 默认值 | 作用 |
|---|---|---|
| `bus.check_settings_s` | 600 | 电表设置只读检查之间的秒数（0 = 从不） |
| `dead_time_ms`, `confirm_timeout_ms` | 2000 | 转换的死区时间以及接触器显示所命令状态的时限 |
| `switching_enabled` | off | 除非此设置、服务器（`ARMOR_ELECTRICAL_SWITCHING=1`）和代理的访问列表都允许，否则开关操作为关闭 |
| `CONFIG_ARMOR_NODE_ID`, `_SETUP_CODE`, `_FLEET_SECRET`, `_MQTT_*`, `_HEARTBEAT_S`, `_BLE_SETUP_MINUTES`, `_BOOT_FACTORY_RESET` | - | 所有节点共有的设置：名称（为空则为 `armor-` 加 MAC 的最后字节）、设置码（为空则随机）、机群密钥（16 个字符以上，绝不放入仓库）、最初的代理设置、心跳（2 到 300 秒）、蓝牙设置的分钟数（1 到 120）以及用 BOOT 重置（前 30 秒内按 8 秒） |

## 🚀 快速开始

**要求:** 用于主机测试的 C++17 编译器；用于构建镜像的 ESP-IDF 5.5.5 容器；ESP32-S3 电路板（以太网或 Wi-Fi）。

```bash
cmake -S tests -B build/host && cmake --build build/host && ctest --test-dir build/host   # host tests, -Werror
node tools/panel_mock.mjs --user admin:adminpass123   # the panel without a board
tools/build_node.sh generic s3-wifi                  # the image in the ESP-IDF container (or s3-eth)
```

See the [firmware guide](docs/NODE_FIRMWARE.md) and the [Bluetooth channel](docs/BLE_PROVISIONING.md).

参见[设计](docs/DESIGN.md)、[安全说明](docs/SAFETY.md)、[开关规则](docs/SWITCHING.md)、[协议](docs/PROTOCOLS.md)和[消息](docs/ELECTRICAL_MESSAGES.md)。

## 🧪 测试

```bash
cmake -S tests -B build/host && cmake --build build/host && ctest --test-dir build/host
python -I tools/armor_ci_validate.py
```

`ctest` 运行 19 个主机测试程序和 5 个 Python 检查；主机程序统计的检查共 382264 项，使用 `-Werror` 构建。

Python 检查：八种语言的面板文字、任务、保存路径、受锁保护的蓝牙会话以及面板调用的路由（部分节点还包括 HTTP 状态码或字体字形）

主机测试需要任意 C++17 编译器（Linux、WSL、MSYS2）；网页面板还会在真实浏览器中对着模拟节点（`tools/panel_mock.mjs`、`tools/panel_browser_test.mjs`）进行验证。

大部分检查是对开关规则的穷举遍历（`test_switch_set` 和 `test_interlock`：375 036）；电表和接触器是替身。`test_clock` 只输出 `ok`。

## 🏗️ 构建与发布

`tools/build_node.sh` 在 ESP-IDF 5.5.5 容器中构建镜像，发布版带有镜像、其 SHA-256 和 Ed25519 签名，每块电路板一份；镜像第一次通过 USB-C 写入，之后从面板更新。

## 🩺 故障排除

| 现象 | 原因 | 处理 |
|---|---|---|
| 忘记了密码，或节点不再加入网络 | 处于设置状态且 90 秒后仍无地址的节点会开启自己的 Wi-Fi `ARMOR-SETUP-xxxxxx`（打开 `http://192.168.4.1/`） | 要重新开始，请在上电后 30 秒内按住 BOOT 8 秒（会清除设置和用户），或使用面板中的*恢复出厂设置* |
| 来自 GitHub 的更新被拒绝 | 发布版缺少校验和或签名，或二者不匹配（`e_no_checksum`、`e_no_signature`、`e_checksum_mismatch`、`e_bad_signature`），没有适用于此电路板的镜像（`e_no_asset_for_board`），或属于另一个项目（`e_wrong_firmware`） | 节点未发生任何变化；请重试，或由管理员手动上传文件 |
| 开关显示故障 | 接触器没有按命令显示断开或闭合（`kDidNotOpen`、`kDidNotClose`）；会保持到被确认 | 检查辅助触点的接线，然后发送 `acknowledge` |
| 命令被拒绝 | 开关操作已关闭（`switching_enabled` 为 false）、令牌无效或已过期，或节点刚才没有布防 | 重新布防并用新令牌发送命令 |

## 📂 仓库结构

<p align="center">
  <img src="images/repository-structure.svg" alt="ARMOR-NODE-ELECTRICAL 的仓库结构" width="100%">
</p>

<details>
<summary>目录树的文本版</summary>

```text
ARMOR-NODE-ELECTRICAL/
├── main/                                固件：一个 ESP-IDF 组件
├── core/                                不依赖 ESP-IDF 的逻辑，在计算机上测试
├── panel/                               嵌入固件的网页面板
├── tests/                               主机测试和 Python 检查
├── docs/                                12 份指南：安全、开关、消息、硬件
├── tools/                               构建、打包和检查工具
├── .github/                             CI 工作流以及 Issue 和 Pull Request 模板
├── images/                              横幅、图标和本 README 的两幅图
├── build.sh build.bat                   通过共享的项目工作流构建
├── test.sh test.bat                     通过共享工作流运行测试
├── run.sh run.bat                       通过共享的项目工作流启动
├── armor.project.json                   清单：版本、角色、成熟度、构建命令
├── CHANGELOG.md                         每个版本的变化
├── SECURITY.md SUPPORT.md LICENSE       安全策略、支持和许可证
├── CONTRIBUTING.md CODE_OF_CONDUCT.md   如何贡献与行为准则
├── CMakeLists.txt                       ESP-IDF 项目
├── partitions.csv                       闪存分区表
├── sdkconfig.defaults                   固件设置
├── sdkconfig.board.s3-eth               某块电路板的设置
├── sdkconfig.board.s3-wifi              某块电路板的设置
├── secrets/                             节点文件示例（真实文件不纳入版本管理）
├── build-test.sh build-test.bat         通过共享工作流运行测试
└── .gitattributes .gitignore            Git 设置
```

</details>

## 🗺️ 路线图与已知限制

**目前已知的限制**

* PZEM-004T v3 和 PZEM-017 的帧与读数已对照文档中的帧和录制的样本检查；目前还没有电表响应过。
* 网页面板的电表和读数页面已在浏览器中对模拟节点测试过，尚未在板子上测试。
* 尚未制作板子，也没有连接电表或接触器：还没有在真实设备上运行过。
* 设置码、限制错误密码次数的登录、蓝牙 PIN 和签名更新已在主机上测试；没有经过独立审查。
* 哈希和 Ed25519 签名检查以及回滚逻辑在主机上通过；空中更新从未在板子上运行过。
* 切换接触器的规则只对照模拟接触器测试过；这里还没有切换过任何真实设备。

**下一步**

* 在带灯泡和隔离变压器的台架上试验电表、接触器和开关规则，然后在真实电路板上试验空中更新。

## 🌍 语言

README 和文本提供英语、西班牙语、法语、意大利语、德语、汉语、日语和欧洲葡萄牙语版本。

## 🔗 相关项目

**A.R.M.O.R.**（Autonomous Radar & Multimodal Observation Range）是一个由相互独立的仓库组成的周界安全系统。每个仓库都有自己的版本、自己的测试和自己的 README；这是整个系列：

* **[ARMOR-COMMON](https://github.com/JuanenRac/ARMOR-COMMON)** - 消息契约、验证器、一致性向量和生成的类型
* **[ARMOR-NODE-RADAR](https://github.com/JuanenRac/ARMOR-NODE-RADAR)** - 面向 ESP32-S3 的现场节点固件，配三个雷达和自带的网页面板
* **[ARMOR-NODE-SOLAR](https://github.com/JuanenRac/ARMOR-NODE-SOLAR)** - 太阳能逆变器和电池的协议，以及网关节点的消息
* **ARMOR-NODE-ELECTRICAL**（本仓库） - 电气节点：电表、电网读数的消息和切换规则
* **[ARMOR-NODE-ALARM](https://github.com/JuanenRac/ARMOR-NODE-ALARM)** - 报警节点与面板：防区、布防、延时、警笛和 PIN，可配合服务器也可独立使用
* **[ARMOR-NODE-HMI](https://github.com/JuanenRac/ARMOR-NODE-HMI)** - 触摸面板：壁挂屏幕上的系统状态、布防与确认，以及语音助手的所在
* **[ARMOR-NETWORK](https://github.com/JuanenRac/ARMOR-NETWORK)** - 本地网络：其设备、互联网以及变化
* **[ARMOR-SERVER](https://github.com/JuanenRac/ARMOR-SERVER)** - 中央协调器：遥测、报警、设备、太阳能读数和摄像头
* **[ARMOR-STUDIO](https://github.com/JuanenRac/ARMOR-STUDIO)** - 网页控制台：摄像头、雷达、报警、太阳能以及 2D/3D 场地设计器
* **[ARMOR-ANDROID-CONTROL](https://github.com/JuanenRac/ARMOR-ANDROID-CONTROL)** - 带实时 2D/3D 雷达的 Android 操作员客户端
* **[ARMOR-IOS-CONTROL](https://github.com/JuanenRac/ARMOR-IOS-CONTROL)** - iPhone 操作员客户端
* **[ARMOR-SERVER-AI](https://github.com/JuanenRac/ARMOR-SERVER-AI)** - 视觉推理策略，会解释其决策且从不驱动设备
* **[ARMOR-VISION](https://github.com/JuanenRac/ARMOR-VISION)** - 面向 Jetson Orin NX 的摄像头区域、遮罩和检测规则
* **[ARMOR-VOICE-AI](https://github.com/JuanenRac/ARMOR-VOICE-AI)** - 离线语音意图，附带无法伪造的确认
* **[ARMOR-HARDWARE](https://github.com/JuanenRac/ARMOR-HARDWARE)** - 外壳、电子器件和台架验收矩阵
* **[ARMOR-DEVOPS](https://github.com/JuanenRac/ARMOR-DEVOPS)** - 部署、CM5 测试台、备份和 TLS
* **[ARMOR-SIMULATOR](https://github.com/JuanenRac/ARMOR-SIMULATOR)** - 带可重复故障的离线遥测模拟器
* **[ARMOR-UPDATER](https://github.com/JuanenRac/ARMOR-UPDATER)** - 检测、安装和更新生态系统自身的仓库
* **[ARMOR-DOCS](https://github.com/JuanenRac/ARMOR-DOCS)** - 架构、安全基线和能力矩阵

## 📚 文档

* [安全](docs/SAFETY.md)和[开关操作](docs/SWITCHING.md)：设计的假设以及转换如何进行
* [消息](docs/ELECTRICAL_MESSAGES.md)、[协议](docs/PROTOCOLS.md)和 [Zigbee](docs/ZIGBEE.md)
* [设计](docs/DESIGN.md)、[硬件](docs/HARDWARE.md)、[底板](docs/BASE_BOARD.md)和[节点固件](docs/NODE_FIRMWARE.md)
* [台架测试](docs/BENCH_TESTS.md)、[设置码](docs/SETUP_CODE.md)和[蓝牙配置](docs/BLE_PROVISIONING.md)
* [能力矩阵：哪些已被证明，哪些没有](https://github.com/JuanenRac/ARMOR-DOCS/blob/main/docs/CAPABILITY_MATRIX.md)
* [项目目录：版本以及各仓库之间如何相互依赖](https://github.com/JuanenRac/ARMOR-DOCS/blob/main/docs/PROJECT_CATALOG.md)
* [本仓库的更新日志](CHANGELOG.md)

## 🤝 参与贡献与社区

* 提交 Pull Request 之前请运行 `ctest --test-dir build/host` and `python -I tools/armor_ci_validate.py`：必须通过。参见 [CONTRIBUTING.md](CONTRIBUTING.md) 和[行为准则](CODE_OF_CONDUCT.md)。
* README 提供八种语言，由 ARMOR-DOCS 中的源文件生成：文本的修改在那里进行。
* 组件之间消息的变更从 ARMOR-COMMON 开始。
* 不要把仅仅构建过的东西描述为已在板子上运行。
* 请私下报告漏洞：[SECURITY.md](SECURITY.md)。问题、想法和报告：electrohobby3d@gmail.com（[SUPPORT.md](SUPPORT.md)）。

## 👤 作者

**JuanenRac (Electro Hobby 3D)** · electrohobby3d@gmail.com

## 📜 许可证

GPL-3.0-or-later - 见 [LICENSE](LICENSE)。
