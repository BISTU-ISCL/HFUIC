# HFUIC

基于 Qt Widgets + Charts 的人因信息展示与分析原型，包含实时监控界面与赛后分析界面两个主要窗口。

## 构建与运行

```bash
cmake -S . -B build
cmake --build build
./build/HFUIC
```

需要 Qt Widgets 与 Qt Charts 组件（Qt 6 优先，Qt 5 亦可）。

## 数据接口

- `RealTimeDisplayWindow::updateSample` 接收实时人脸识别/情绪/心率/负荷、鼠标轨迹与点击、注视点和键盘按键列表，驱动实时展示界面。
- `RealTimeDisplayWindow::updateGazeHeatmap` 可将外部眼动矩阵映射到界面作为热度提示。
- `PostMatchAnalysisWindow::setHeartRateSeries`、`setWorkloadSeries`、`setMouseTrajectory`、`setGazeHeatmap`、`setKeyboardHeatmap`、`setReport` 用于赛后分析界面填充全程曲线、鼠标轨迹密度、眼动/键盘热图以及结构化报告。
