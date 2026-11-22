# JamBar — 中文版跨平台实时网络合奏插件

<p align="center">
  <strong>公开即兴排练室 · Windows 64 位 VST3 / ASIO &amp; macOS Apple Silicon AU / VST3 / 独立 App</strong><br/>
  作者：<strong>何岸文</strong> · macOS 贡献者：<strong>Bassist Max</strong><br/>
  官方排练室主页：<a href="https://tanwen.cc">https://tanwen.cc</a> · 默认服务器：<code>tanwen.cc:2049</code>
</p>

> 本仓库为 **JamBar** 官方文档、使用指北、更新日志、问题反馈（Issues）与正式版本发布（Releases）仓库。

---

## 快速下载（最新版 v2.3.1）

前往 [**Releases 页面**](https://github.com/KeenyHo/JamBar/releases/latest) 或点击下方直链下载：

| 平台 | 格式包含 | 压缩包直链 | 文件大小 | SHA-256 校验 |
| :--- | :--- | :--- | :---: | :--- |
| **Windows 64 位** | `JamBar.vst3` + ASIO 独立版 `JamBar.exe` | [JamBar-Windows-x64-VST3.zip](https://github.com/KeenyHo/JamBar/releases/download/v2.3.1/JamBar-Windows-x64-VST3.zip) | `7.67 MB` | `4861dbe1bfbf5bf6b14ca4e731a6ac52befa57e934c52e113e9b29a61b1a4424` |
| **macOS Apple Silicon** | `JamBar.component` (AU) + `JamBar.vst3` + `JamBar.app` | [JamBar-macOS-arm64.zip](https://github.com/KeenyHo/JamBar/releases/download/v2.3.1/JamBar-macOS-arm64.zip) | `18.9 MB` | `d0b0d94be1346d3c5368c68d253423b13f198b122024f593131f7ea9c640eca2` |

---

## 核心特性

### 1. Windows & macOS 全平台原生形态
- **Windows x64**：原生 64 位 VST3 插件（通过 Tracktion `pluginval` Strictness 5 全项严格校验），同时附带内置 **Steinberg ASIO** 低延迟驱动的独立桌面版 `JamBar.exe`。
- **macOS arm64 (M1/M2/M3/M4)**：原生 Audio Unit（Logic Pro / GarageBand）、VST3 与独立 App，已内置 `libcrypto.3.dylib` 动态库，开箱即用，支持 macOS 12 及以上系统（特别鸣谢 **Bassist Max** 贡献 macOS 适配与打包方案）。

### 2. 低延迟 32 / 64 采样深度优化
- 音频线程采用原位零拷贝 SPSC 环形缓冲与 256 采样广播聚合引擎，即使将 ASIO / CoreAudio / DAW 缓冲设为 **64 或 32 采样**，也能保持稳定零丢包、清晰无杂音。

### 3. 三轨小节同步循环器（Loop Station）
- 内置 **Loop 1 / Loop 2 / Loop 3** 三轨独立循环录放器，每轨配备独立音量条（`0%–150%`，双击归 `100%`），最多支持三轨同时叠加。
- **一键循环操作**：点按进入「待录」 -> 自动对齐**下一个循环第一拍**开始录制纯干声乐器（不串录鼓机和其他 Loop，首尾 5ms 无爆音淡入淡出） -> 循环结束自动无缝循环播放（本地监听且同步广播到房间） -> 再点一下即清空停止。
- 支持最长 **60 秒**单轨循环，在线合奏与离线练习均可用。

### 4. 内置真实录音棚采样鼓机（3 套真鼓 × 12 风格 × 54 鼓型）
- 内置 **3 套多力度分层真实录音棚/经典硬件采样鼓组**，播放中随时无缝秒切：
  - **GSCW 温暖真鼓**：Yamaha 16x16 底鼓 + CustomWorks 6x13 军鼓 + Sabian AAX 踩镲 / 22" RobMor 叮叮镲
  - **GMRock 摇滚真鼓**：均衡干净的经典排练室原声真鼓
  - **TR-909 电子鼓**：传奇硬件电子舞曲采样
- 涵盖流行 / 摇滚 / 放克 / 爵士 / 布鲁斯 / 雷鬼 / 拉丁 / 嘻哈 / 电子 / 金属 / 民谣 / 基础练习共 **12 大风格 54 套鼓型**，支持真实闭踩镲 Choke 闷音、Swing 律动与小节首拍 Crash。
- 鼓机声音直接从本地立体声输入进入；**进房仅同步房间 BPM**（不受 DAW 宿主 BPM 干扰），**未进房时可直接点击底部 BPM / BPI 自由调节离线练习**。

### 5. 为实时排练打磨的现代中文交互（tanwen.cc 设计语言）
- **极简单立体声输入/输出**：告别繁琐多总线，单声道吉他/话筒自动双声道监听；左侧常驻本地通道 `(用户名) 我` + 国旗，右侧常驻总控与节拍器，未进房与旁观也能实时监听。
- **欢迎页一键三连**：**进入 JamBar**（直连 `tanwen.cc:2049`）、**听听 JamBar**（匿名旁观）、**浏览国际房间**（自动过滤房间机器人计数）。
- **旁观模式 & 自动缩放世界地图**：旁观与进房均实时显示乐手名称、矢量国旗与国家/地区，并在底部世界地图用折线互联（全在国内自动放大为中国地图，跨国自动切换全球视图）。
- **独立点按测速 & 首小节同步倒计时**：手动点按即时测 BPM（3 秒无点击归零，不联动房间）；进房等待首小节同步出声时实时显示剩余拍数与秒数提示。

---

## 安装指南

### Windows（64 位）
1. 下载并解压 `JamBar-Windows-x64-VST3.zip`。
2. **VST3 插件**：将整个 `JamBar.vst3` 文件夹复制到：
   ```text
   C:\Program Files\Common Files\VST3\
   ```
   重启 Cubase / Studio One / REAPER / Ableton Live / FL Studio 扫描插件，在效果器列表找到 **JamBar** 加载到音频轨即可。
3. **独立桌面版**：直接双击 `JamBar.exe`，点击左上角音频设置选择你的 **ASIO** 声卡与输入/输出通道。

### macOS（Apple Silicon / M1–M4）
1. 下载并解压 `JamBar-macOS-arm64.zip`。
2. **AU 插件（Logic Pro / GarageBand）**：将 `JamBar.component` 复制到 `~/Library/Audio/Plug-Ins/Components/`。
3. **VST3 插件**：将 `JamBar.vst3` 复制到 `~/Library/Audio/Plug-Ins/VST3/`。
4. **独立桌面版**：将 `JamBar.app` 拖入「应用程序」文件夹打开。
   > 若首次打开提示“无法验证开发者”（ad-hoc 签名），请打开 **系统设置 → 隐私与安全性**，在底部点击 **「仍要打开」** 即可。

---

## 更多文档与社区反馈

- 📖 [**更新日志（CHANGELOG）**](./CHANGELOG.md)
- ❓ [**常见问题排查（FAQ）**](./FAQ.md)
- 💬 **Bug 反馈与功能建议**：欢迎直接在本仓库提交 [**Issues**](https://github.com/KeenyHo/JamBar/issues)
- 🎸 **官方即兴排练室主页**：[https://tanwen.cc](https://tanwen.cc)
