# OpenVR Space Calibrator

这有助于你将一家公司的 VR 设备与其它任何公司的设备一起使用。它通过一个快速的校准步骤来对齐多个跟踪系统。它可能不适用于你的设置，但在许多情况下可以部分工作，有些情况下效果非常好。

- Rift CV1 与 Vive 设备：与 v2（蓝色标志）跟踪器配合效果非常好，v1 跟踪器（灰色标志，已停产）在红外光谱中存在重大干扰问题。控制器 Wands（both gen）和 Index 控制器效果都非常好。
- Rift S、Quest、Windows MR 及其他 SLAM 内向外跟踪的 HMD 与 Vive 设备：当你不在房间内大幅移动时（例如玩 Beat Saber）效果很好，但大量走动会导致系统之间出现相当程度的漂移。具体效果可能会因你的空间而异。一些问题可能通过改进的校准算法在软件中解决。
- 目前，Quest 的无线串流表现特别差，需要频繁重新校准，但在短时间内还是可以工作的，直到多个因素中的一个导致漂移。对于无线设备，校准时缓慢移动并使用慢速或非常慢的校准模式可以有效减少初始误差。
- 任何非 Rift HMD 与 Touch 控制器：无法使用，Oculus 驱动程序要求 HMD 必须是 Rift。理论上可以通过软件解决，但据我所知，还没有人这样做，因为这需要相当多的逆向工程工作。

在 [**Discord**](https://discord.gg/m7g2Wyj) 上有一个几千人的社区，并且在 [**Reddit**](https://www.reddit.com/r/MixedVR/) 上也有一个新社区。你可以在 [**wiki**](https://github.com/pushrax/OpenVR-SpaceCalibrator/wiki) 中找到你的问题的答案。

可以观看一个展示旧版本 (~v0.3) 如何工作的快速视频，链接是：https://www.youtube.com/watch?v=W3TnQd9JMl4 。自那时以来，用户界面已经升级，现在可以通过 SteamVR 仪表板菜单进行校准，并且有更多的可配置选项。

### 安装软件

在按照下面的说明操作之前，请下载并运行 [最新版本](https://github.com/hyblocker/OpenVR-SpaceCalibrator/releases/latest) 的安装程序。这将自动将 SteamVR 设置为使用多个跟踪系统（`activateMultipleDrivers: true`）。有很多指南说你需要手动编辑 SteamVR 配置，但其实不需要。

> 此汉化版没有自动安装程序，意味着软件可能不会自动设置 SteamVR 设置，你需要手动编辑！

### 使用方法

一旦 空间校准器 完成校准，它会在后台工作以保持设备的正确配置。从 v0.8 版本开始，除了创建校准之外，所有操作都是自动化的。

### 校准

这个分支支持连续校准。这意味着有两种校准方式：

#### 1. 普通校准

>  当你在头戴设备上没有 Tracker 时，请使用此方法。

作为第一次设置的一部分，或者当你对空间进行更改（例如移动传感器）时，偶尔由于校准随时间漂移（消费级 VR 跟踪并不完全稳定），你需要运行一次校准：

1. 从你的 HMD 的游戏空间中复制 Chaperone/Guardian 边界。如果自上次复制以来 HMD 的游戏空间没有变化，则不需要运行此步骤。__示例：__ 如果你在使用 Rift 和 Vive 跟踪器时碰到一个 Vive Lighthouse，或者校准刚刚稍微漂移，你可能不需要运行此步骤，但如果你碰到一个 Oculus 传感器，则需要运行此步骤（在重新运行 Oculus Guardian 设置之后）。
   
   1. 运行 SteamVR，并且只开启你 HMD 跟踪系统中的设备。__示例：__ 对于 Rift 和 Vive 跟踪器的组合，不要先开启跟踪器。
   2. 确认你的 Chaperone/Guardian 已正确设置墙壁位置。如果之后进行更改，你需要再次运行此步骤。
   3. 在 SteamVR 叠加层中打开 SPACE CAL。
   4. 点击 `将 Chaperone 边界复制到配置文件`

2. 校准设备。
   
   1. 如果你还没有打开 SteamVR，请先打开它。然后开启一些或全部设备。
   2. 在 SteamVR 叠加层中打开 SPACE CAL。
   3. 从左侧的 主设备 中选择一个设备，从右侧的 目标设备 中选择一个设备。如果你开启了多个来自同一空间的设备且无法确定哪个被选中，点击“标识选定设备”以使其 LED 闪烁或振动。__示例：__ 对于 Rift 和 Vive 跟踪器，你会看到左侧是 Touch 控制器，右侧是 Vive 跟踪器。__小贴士：__ 如果你只开启了一个 Vive 跟踪器，你不需要弄清楚哪个被选中。
   4. 将这两个设备用一只手拿起，并握牢它们。如果它们在校准过程中发生偏移，最终效果可能会受到影响。
   5. 点击 `开始校准`
   6. Move and rotate your hand around slowly a few times, like you're calibrating the compass on your phone. You want to sample as many orientations as possible.
   7. Done! A profile will be saved automatically. If you haven't already, turn on all your devices. Space Calibrator will automatically apply the calibration to devices as they turn on.

#### 2. 连续校准

> 仅在你的头戴设备上有 Tracker 时才使用此方法。

连续校准允许你持续校准游戏空间，从而无需执行上述任何操作。

1. 启动 SteamVR，并佩戴你希望使用的 VR 头显。

2. **仅开启** 附加在 VR 头显上的 Tracker。

3. 选择 VR 头显和 Tracker，然后进行校准。

4. 打开你的其它设备。

5. 在进行初始校准时，当你在游戏空间中移动一会儿之后应该会看到它们与你对齐。

### VR 外的校准

你可以在打开 SteamVR 后通过恢复 空间校准器 来进行校准（它默认以最小化状态启动）。如果你正在为没有任何跟踪系统设备的独立 HMD 进行校准，这是必需的。

### 编译你自己的版本

在 Visual Studio 2017 中打开 `OpenVR-SpaceCalibrator.sln` 并进行编译。没有外部依赖项。

### 数学原理

有关详细信息，请参见 [math.pdf](https://github.com/pushrax/OpenVR-SpaceCalibrator/blob/master/math.pdf)。
如果你有一些改进校准过程的想法，请在[原项目](https://github.com/hyblocker/OpenVR-SpaceCalibrator)上告诉作者！
