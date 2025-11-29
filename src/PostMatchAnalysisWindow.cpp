#include "PostMatchAnalysisWindow.h"

#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>
#include <QtCharts/QPieSeries>
#include <QtCharts/QPolarChart>
#include <QtCharts/QValueAxis>
#include <QGridLayout>
#include <QLegend>
#include <QRandomGenerator>

PostMatchAnalysisWindow::PostMatchAnalysisWindow(QWidget *parent)
    : QWidget(parent) {
  auto *root = new QVBoxLayout(this);
  auto *topLayout = new QHBoxLayout();

  topLayout->addWidget(buildIdentityPanel(), 3);
  topLayout->addWidget(buildLineChart(), 5);
  topLayout->addWidget(buildRadarChart(), 4);

  auto *bottomLayout = new QHBoxLayout();
  bottomLayout->addWidget(buildStackedBar(), 4);
  bottomLayout->addWidget(buildHeatmapChart(), 6);

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
  for (int i = 0; i <= 360; i += 45) {
    const qreal radius = 20 + QRandomGenerator::global()->bounded(80);
    series->append(i, radius);
  }

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

QChartView *PostMatchAnalysisWindow::buildHeatmapChart() {
  heatmapChart_ = new QChart();
  heatmapChart_->setTitle(tr("键盘操作热力图"));

  // Represent heatmap via stacked bars for simplicity.
  auto *series = new QBarSeries();
  for (int i = 0; i < 4; ++i) {
    auto *row = new QBarSet(QString("R%1").arg(i + 1));
    for (int j = 0; j < 8; ++j) {
      row->append(QRandomGenerator::global()->bounded(10));
    }
    series->append(row);
  }

  heatmapChart_->addSeries(series);
  auto *axisX = new QBarCategoryAxis();
  axisX->append({"1", "2", "3", "4", "5", "6", "7", "8"});
  auto *axisY = new QValueAxis();
  axisY->setRange(0, 10);

  heatmapChart_->addAxis(axisX, Qt::AlignBottom);
  heatmapChart_->addAxis(axisY, Qt::AlignLeft);
  series->attachAxis(axisX);
  series->attachAxis(axisY);

  auto *view = new QChartView(heatmapChart_);
  view->setRenderHint(QPainter::Antialiasing);
  return view;
}

void PostMatchAnalysisWindow::simulateAnalysis() {
  const int nextIndex = heartRateSeries_->count();
  const int hr = 70 + QRandomGenerator::global()->bounded(60);
  const int wl = QRandomGenerator::global()->bounded(40, 120);

  heartRateSeries_->append(nextIndex, hr);
  workloadSeries_->append(nextIndex, wl);
  if (nextIndex > 100) {
    heartRateSeries_->removePoints(0, 1);
    workloadSeries_->removePoints(0, 1);
  }

  // Update radar chart
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

  // Update textual report
  QString summary = hr > 110 ? tr("总体评价: 后期压力较大") : tr("总体评价: 控制良好");
  summaryLabel_->setText(summary);
  advantageBox_->setPlainText(tr("情绪稳定：情绪曲线平稳，心率波动可控。\n操作高效：常用键位使用集中。"));
  issueBox_->setPlainText(tr("抗压能力：失利后心率上升速度快。\n注意力分配：需要更多关注战场全局信息。"));
  adviceBox_->setPlainText(tr("呼吸调节训练与高压模拟对抗。\n开展视野扩展练习，提升多目标监控能力。"));
}

