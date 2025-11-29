# HFUIC

基于 Qt Widgets + Charts 的人因信息展示与分析原型，包含实时监控界面与赛后分析界面两个主要窗口。

## 构建与运行

```bash
cmake -S . -B build
cmake --build build
./build/HFUIC
```

需要 Qt Widgets 与 Qt Charts 组件（Qt 6 优先，Qt 5 亦可）。
