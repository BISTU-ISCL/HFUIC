#include "PostMatchAnalysisWindow.h"

#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>
#include <QtCharts/QPieSeries>
#include <QtCharts/QPolarChart>
#include <QtCharts/QScatterSeries>
#include <QtCharts/QValueAxis>
#include <QGridLayout>
#include <QLegend>
#include <QPainter>
#include <QRandomGenerator>
#include <algorithm>

namespace {
QColor heatColor(double value) {
  const double clamped = std::clamp(value, 0.0, 1.0);
  const int r = static_cast<int>(clamped * 255);
  const int g = static_cast<int>((1 - clamped) * 120);
  const int b = 255 - g / 2;
  return QColor(r, g, b, 210);
}
}

HeatmapWidget::HeatmapWidget(QWidget *parent) : QWidget(parent) {
  setMinimumSize(200, 180);
}

void HeatmapWidget::setData(const std::vector<std::vector<double>> &data) {
  data_ = data;
  update();
}

void HeatmapWidget::paintEvent(QPaintEvent *event) {
  QWidget::paintEvent(event);
  if (data_.empty()) return;

  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);

  const int rows = static_cast<int>(data_.size());
  const int cols = static_cast<int>(data_.front().size());
  const double cellW = width() / static_cast<double>(cols);
  const double cellH = height() / static_cast<double>(rows);

  for (int r = 0; r < rows; ++r) {
    for (int c = 0; c < cols; ++c) {
      const QRectF rect(c * cellW, r * cellH, cellW, cellH);
      painter.fillRect(rect, heatColor(data_[r][c]));
      painter.setPen(QPen(Qt::black, 0.5));
      painter.drawRect(rect);
    }
  }
}

PostMatchAnalysisWindow::PostMatchAnalysisWindow(QWidget *parent)
    : QWidget(parent) {
  auto *root = new QVBoxLayout(this);
  auto *topLayout = new QHBoxLayout();

  topLayout->addWidget(buildIdentityPanel(), 3);
  topLayout->addWidget(buildLineChart(), 5);
  topLayout->addWidget(buildRadarChart(), 4);

  auto *bottomLayout = new QHBoxLayout();
  bottomLayout->addWidget(buildStackedBar(), 3);
  bottomLayout->addWidget(buildGazeHeatmap(), 3);
  bottomLayout->addWidget(buildHeatmapChart(), 4);

  auto *reportBox = new QGroupBox(tr("赛后分析简报"));
  auto *reportLayout = new QVBoxLayout(reportBox);
  summaryLabel_ = new QLabel(tr("总体评价: 正在生成..."));
  advantageBox_ = new QTextEdit();
  issueBox_ = new QTextEdit();
  adviceBox_ = new QTextEdit();
  advantageBox_->setReadOnly(true);
  issueBox_->setReadOnly(true);
  adviceBox_->setReadOnly(true);

  reportLayout->addWidget(summaryLabel_);
  reportLayout->addWidget(new QLabel(tr("优势区")));
  reportLayout->addWidget(advantageBox_);
  reportLayout->addWidget(new QLabel(tr("待改进区")));
  reportLayout->addWidget(issueBox_);
  reportLayout->addWidget(new QLabel(tr("简单建议")));
  reportLayout->addWidget(adviceBox_);

  root->addLayout(topLayout, 6);
  root->addLayout(bottomLayout, 4);
  root->addWidget(reportBox, 3);

  auto *timer = new QTimer(this);
  connect(timer, &QTimer::timeout, this, &PostMatchAnalysisWindow::simulateAnalysis);
  timer->start(1500);
}

QWidget *PostMatchAnalysisWindow::buildIdentityPanel() {
  auto *panel = new QGroupBox(tr("人脸与基础指标"));
  auto *layout = new QVBoxLayout(panel);

  auto *face = new QLabel(tr("3D/2D 人脸模型占位"));
  face->setAlignment(Qt::AlignCenter);
  face->setMinimumHeight(160);
  face->setStyleSheet("background: #0b1a2e; color: #7dd; border: 1px solid #1e90ff;");

  layout->addWidget(face);
  layout->addWidget(new QLabel(tr("验证: 通过")));
  layout->addWidget(new QLabel(tr("状态负荷: 中")));
  layout->addWidget(new QLabel(tr("情绪: 平静")));
  layout->addWidget(new QLabel(tr("平均心率: 82 bpm")));
  return panel;
}

QChartView *PostMatchAnalysisWindow::buildLineChart() {
  heartRateSeries_ = new QLineSeries();
  workloadSeries_ = new QLineSeries();
  heartRateSeries_->setName(tr("心率"));
  workloadSeries_->setName(tr("状态负荷"));

  auto *chart = new QChart();
  chart->addSeries(heartRateSeries_);
  chart->addSeries(workloadSeries_);
  chart->setTitle(tr("全局心率与负荷曲线"));

  auto *axisX = new QValueAxis();
  axisX->setTitleText(tr("时间"));
  axisX->setRange(0, 100);
  auto *axisY = new QValueAxis();
  axisY->setTitleText(tr("强度"));
  axisY->setRange(0, 160);

  chart->setAxisX(axisX, heartRateSeries_);
  chart->setAxisX(axisX, workloadSeries_);
  chart->setAxisY(axisY, heartRateSeries_);
  chart->setAxisY(axisY, workloadSeries_);

  auto *view = new QChartView(chart);
  view->setRenderHint(QPainter::Antialiasing);
  return view;
}

QChartView *PostMatchAnalysisWindow::buildRadarChart() {
  radarChart_ = new QPolarChart();
  radarChart_->setTitle(tr("鼠标运动轨迹密度"));

  auto *series = new QLineSeries();
  series->setName(tr("轨迹"));
  radarChart_->addSeries(series);
  auto *angular = new QValueAxis();
  angular->setTickCount(9);
  angular->setLabelFormat("%d°");
  auto *radial = new QValueAxis();
  radial->setRange(0, 100);
  radarChart_->addAxis(angular, QPolarChart::PolarOrientationAngular);
  radarChart_->addAxis(radial, QPolarChart::PolarOrientationRadial);
  series->attachAxis(angular);
  series->attachAxis(radial);

  auto *view = new QChartView(radarChart_);
  view->setRenderHint(QPainter::Antialiasing);
  return view;
}

QChartView *PostMatchAnalysisWindow::buildStackedBar() {
  focusSet_ = new QBarSet(tr("关注度"));
  *focusSet_ << 5 << 7 << 3 << 9 << 4;

  auto *series = new QBarSeries();
  series->append(focusSet_);

  auto *chart = new QChart();
  chart->addSeries(series);
  chart->setTitle(tr("眼动注意力分配"));

  QStringList categories = {tr("小地图"), tr("技能栏"), tr("战场"), tr("资源"), tr("背包")};
  auto *axisX = new QBarCategoryAxis();
  axisX->append(categories);
  auto *axisY = new QValueAxis();
  axisY->setRange(0, 10);

  chart->addAxis(axisX, Qt::AlignBottom);
  chart->addAxis(axisY, Qt::AlignLeft);
  series->attachAxis(axisX);
  series->attachAxis(axisY);

  auto *view = new QChartView(chart);
  view->setRenderHint(QPainter::Antialiasing);
  return view;
}

QWidget *PostMatchAnalysisWindow::buildGazeHeatmap() {
  auto *box = new QGroupBox(tr("眼动热力图"));
  auto *layout = new QVBoxLayout(box);
  gazeHeatmap_ = new HeatmapWidget();
  layout->addWidget(gazeHeatmap_);
  return box;
}

QWidget *PostMatchAnalysisWindow::buildHeatmapChart() {
  auto *box = new QGroupBox(tr("键盘操作热力图"));
  auto *layout = new QVBoxLayout(box);
  keyboardHeatmap_ = new HeatmapWidget();
  layout->addWidget(keyboardHeatmap_);
  return box;
}

void PostMatchAnalysisWindow::simulateAnalysis() {
  // Append demo samples for charts
  const int nextIndex = heartRateSeries_->count();
  const int hr = 70 + QRandomGenerator::global()->bounded(60);
  const int wl = QRandomGenerator::global()->bounded(40, 120);

  heartRateSeries_->append(nextIndex, hr);
  workloadSeries_->append(nextIndex, wl);
  if (nextIndex > 100) {
    heartRateSeries_->removePoints(0, 1);
    workloadSeries_->removePoints(0, 1);
  }

  // Update radar chart using aggregated mouse trajectory samples
  auto *series = static_cast<QLineSeries *>(radarChart_->series().first());
  series->clear();
  for (int i = 0; i <= 360; i += 45) {
    const qreal radius = 20 + QRandomGenerator::global()->bounded(80);
    series->append(i, radius);
  }

  // Update attention bars
  for (int i = 0; i < focusSet_->count(); ++i) {
    (*focusSet_)[i] = QRandomGenerator::global()->bounded(2, 10);
  }

  // Update heatmaps with random data
  std::vector<std::vector<double>> gaze(6, std::vector<double>(8));
  std::vector<std::vector<double>> keyboard(4, std::vector<double>(10));
  for (auto &row : gaze) {
    for (auto &cell : row) cell = QRandomGenerator::global()->bounded(100) / 100.0;
  }
  for (auto &row : keyboard) {
    for (auto &cell : row) cell = QRandomGenerator::global()->bounded(100) / 100.0;
  }
  gazeHeatmap_->setData(gaze);
  keyboardHeatmap_->setData(keyboard);

  // Update textual report
  QString summary = hr > 110 ? tr("总体评价: 后期压力较大") : tr("总体评价: 控制良好");
  summaryLabel_->setText(summary);
  advantageBox_->setPlainText(tr("情绪稳定：情绪曲线平稳，心率波动可控。\n操作高效：常用键位使用集中。"));
  issueBox_->setPlainText(tr("抗压能力：失利后心率上升速度快。\n注意力分配：需要更多关注战场全局信息。"));
  adviceBox_->setPlainText(tr("呼吸调节训练与高压模拟对抗。\n开展视野扩展练习，提升多目标监控能力。"));
}

void PostMatchAnalysisWindow::setHeartRateSeries(const QVector<QPointF> &points) {
  heartRateSeries_->replace(points);
}

void PostMatchAnalysisWindow::setWorkloadSeries(const QVector<QPointF> &points) {
  workloadSeries_->replace(points);
}

void PostMatchAnalysisWindow::setMouseTrajectory(const QVector<QPointF> &path,
                                                 const QVector<QPointF> &clicks) {
  // path draws as radar spokes length = density; here we map to radar chart series
  auto *series = static_cast<QLineSeries *>(radarChart_->series().first());
  series->clear();
  for (const auto &p : path) {
    series->append(p);
  }
  // clicks are handled via histogram (focusSet) approximation
  for (int i = 0; i < focusSet_->count() && i < clicks.size(); ++i) {
    (*focusSet_)[i] = clicks[i].y();
  }
}

void PostMatchAnalysisWindow::setGazeHeatmap(const std::vector<std::vector<double>> &heatmap) {
  gazeHeatmap_->setData(heatmap);
}

void PostMatchAnalysisWindow::setKeyboardHeatmap(
    const std::vector<std::vector<double>> &heatmap) {
  keyboardHeatmap_->setData(heatmap);
}

void PostMatchAnalysisWindow::setReport(const QString &summary, const QString &advantages,
                                        const QString &issues, const QString &advice) {
  summaryLabel_->setText(summary);
  advantageBox_->setPlainText(advantages);
  issueBox_->setPlainText(issues);
  adviceBox_->setPlainText(advice);
}

