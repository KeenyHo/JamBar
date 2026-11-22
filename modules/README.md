# JamBar 独立音频模块参考源码（`modules/`）

本目录开放了 JamBar 中自研的两个无锁实时音频 DSP 模块源码，供音频插件开发者与乐手交流学习：

| 模块目录 | 文件 | 说明 |
| :--- | :--- | :--- |
| [`DrumMachine/`](./DrumMachine/) | `DrumMachine.h` / `DrumMachine.cpp` | 实时零分配多声部采样鼓机引擎（3 套真实采样鼓组 × 12 大风格 × 54 套带 Swing 律动鼓型，支持双力度分层、闭踩镲 Choke 闷音与小节同步） |
| [`LoopStation/`](./LoopStation/) | `LoopStation.h` / `LoopStation.cpp` | 三轨小节同步立体声循环器（干声采集防串录、单采样点精度循环边界对齐、5ms 无爆音淡入淡出、每轨最长 60 秒无锁实时录放） |

> 注：上述模块依赖 JUCE 8 框架接口；如需依据 GPL-2.0 协议获取完整工程可编译源码，请通过本仓库 [Issues](https://github.com/KeenyHo/JamBar/issues) 或访问 [https://tanwen.cc](https://tanwen.cc) 联系作者 **何岸文**。
