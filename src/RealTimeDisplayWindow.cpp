#include "RealTimeDisplayWindow.h"

#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QScatterSeries>
#include <QBrush>
#include <QGraphicsDropShadowEffect>
#include <QGridLayout>
#include <QRadialGradient>
#include <QRandomGenerator>

namespace {
QColor emotionColor(const QString &emotion) {
  if (emotion == "平静") return QColor(173, 216, 230);
  if (emotion == "开心") return QColor(255, 223, 0);
  if (emotion == "悲伤") return QColor(96, 125, 139);
  if (emotion == "惊讶") return QColor(255, 152, 0);
  if (emotion == "恐惧") return QColor(156, 39, 176);
  if (emotion == "愤怒") return QColor(229, 57, 53);
  if (emotion == "轻蔑") return QColor(143, 188, 143);
  return QColor(85, 107, 47);
}

QString randomEmotion() {
  static QStringList emotions = {"平静", "开心", "悲伤", "惊讶", "恐惧", "愤怒", "轻蔑", "厌恶"};
  return emotions.at(QRandomGenerator::global()->bounded(emotions.size()));
}

QString randomWorkload() {
  switch (QRandomGenerator::global()->bounded(3)) {
  case 0:
    return "低";
  case 1:
    return "中";
  default:
    return "高";
  }
}

QColor workloadColor(const QString &level) {
  if (level == "低") return QColor(76, 175, 80);
  if (level == "中") return QColor(255, 193, 7);
  return QColor(244, 67, 54);
}
} // namespace

RealTimeDisplayWindow::RealTimeDisplayWindow(QWidget *parent)
    : QWidget(parent) {
  auto *rootLayout = new QVBoxLayout(this);
  auto *topLayout = new QHBoxLayout();

  topLayout->addWidget(buildPlayerPanel(), 4);
  topLayout->addWidget(buildRightPanel(), 6);

  rootLayout->addLayout(topLayout, 7);
  rootLayout->addWidget(buildKeyboardHeatmap(), 3);

  connect(&dataTimer_, &QTimer::timeout, this, &RealTimeDisplayWindow::simulateDataUpdate);
  dataTimer_.start(1200);

  connect(&keyboardTimer_, &QTimer::timeout, this, &RealTimeDisplayWindow::flashRandomKey);
  keyboardTimer_.start(300);
}

QWidget *RealTimeDisplayWindow::buildPlayerPanel() {
  auto *panel = new QGroupBox(tr("选手状态监控"));
  auto *layout = new QVBoxLayout(panel);

  // 视频与身份
  auto *videoArea = new QLabel(tr("实时视频流\n(摄像头占位)"));
  videoArea->setMinimumHeight(200);
  videoArea->setAlignment(Qt::AlignCenter);
  videoArea->setStyleSheet("background: #0b1a2e; color: #7dd; border: 1px solid #1e90ff;");

  identityLabel_ = new QLabel(tr("ID: 未知选手"));
  identityLabel_->setStyleSheet("color: #9cf; font-weight: bold;");

  faceAlertLabel_ = new QLabel(tr("人脸检测: 未检测"));
  faceAlertLabel_->setStyleSheet("color: #f44336; font-weight: bold;");

  auto *videoLayout = new QVBoxLayout();
  videoLayout->addWidget(videoArea);
  videoLayout->addWidget(identityLabel_);
  videoLayout->addWidget(faceAlertLabel_);

  auto *videoFrame = new QWidget;
  videoFrame->setLayout(videoLayout);

  // 指标仪表盘
  auto *metrics = new QWidget;
  auto *grid = new QGridLayout(metrics);

  emotionLabel_ = new QLabel(tr("情绪: 平静"));
  heartRateLabel_ = new QLabel(tr("心率: 72 bpm"));
  workloadLabel_ = new QLabel(tr("负荷: 中"));

  grid->addWidget(emotionLabel_, 0, 0);
  grid->addWidget(heartRateLabel_, 1, 0);
  grid->addWidget(workloadLabel_, 2, 0);

  layout->addWidget(videoFrame);
  layout->addWidget(metrics);
  return panel;
}

QChartView *RealTimeDisplayWindow::buildMousePathChart() {
  auto *chart = new QChart();
  chart->setTitle(tr("鼠标轨迹与注视点"));
  chart->legend()->setVisible(true);

  mousePathSeries_ = new QLineSeries();
  mousePathSeries_->setName(tr("鼠标移动"));

  gazeSeries_ = new QLineSeries();
  gazeSeries_->setName(tr("注视点"));
  gazeSeries_->setPen(QPen(QColor(30, 144, 255), 3, Qt::DotLine));

  chart->addSeries(mousePathSeries_);
  chart->addSeries(gazeSeries_);
  chart->createDefaultAxes();
  chart->axes(Qt::Horizontal).first()->setRange(0, 100);
  chart->axes(Qt::Vertical).first()->setRange(0, 100);

  auto *view = new QChartView(chart);
  view->setRenderHint(QPainter::Antialiasing);
  return view;
}

QWidget *RealTimeDisplayWindow::buildRightPanel() {
  auto *panel = new QWidget;
  auto *layout = new QVBoxLayout(panel);

  auto *screenArea = new QLabel(tr("比赛画面 (屏幕占位)"));
  screenArea->setMinimumHeight(180);
  screenArea->setAlignment(Qt::AlignCenter);
  screenArea->setStyleSheet("background: #07111f; color: #9cf; border: 1px solid #1e90ff;");

  auto *chartView = buildMousePathChart();

  layout->addWidget(screenArea, 4);
  layout->addWidget(chartView, 5);
  return panel;
}

QWidget *RealTimeDisplayWindow::buildKeyboardHeatmap() {
  auto *panel = new QGroupBox(tr("键盘操作热力图"));
  auto *grid = new QGridLayout(panel);
  QString keys[4][10] = {{"ESC", "1", "2", "3", "4", "5", "6", "7", "8", "9"},
                         {"TAB", "Q", "W", "E", "R", "T", "Y", "U", "I", "O"},
                         {"CAPS", "A", "S", "D", "F", "G", "H", "J", "K", "L"},
                         {"SHIFT", "Z", "X", "C", "V", "B", "N", "M", ",", "."}};

  for (int r = 0; r < 4; ++r) {
    for (int c = 0; c < 10; ++c) {
      auto *label = new QLabel(keys[r][c]);
      label->setAlignment(Qt::AlignCenter);
      label->setStyleSheet("background: #0f213a; color: #9cf; border: 1px solid #1e90ff; padding: 6px;");
      grid->addWidget(label, r, c);
      keyboardGrid_[r][c] = label;
    }
  }
  return panel;
}

void RealTimeDisplayWindow::simulateDataUpdate() {
  const QString emotion = randomEmotion();
  const QString workload = randomWorkload();
  const int heartRate = QRandomGenerator::global()->bounded(60, 140);

  identityLabel_->setText(tr("ID: 选手A"));
  const bool faceDetected = QRandomGenerator::global()->bounded(10) > 1;
  faceAlertLabel_->setText(faceDetected ? tr("人脸检测: 正常") : tr("人脸检测: 异常"));
  faceAlertLabel_->setStyleSheet(QString("color:%1;font-weight:bold;") + (faceDetected ? "#8bc34a" : "#f44336"));

  emotionLabel_->setText(tr("情绪: %1").arg(emotion));
  emotionLabel_->setStyleSheet(QString("color:#021a2f; font-weight:bold; background:%1; padding:6px;")
                                   .arg(emotionColor(emotion).name()));

  heartRateLabel_->setText(tr("心率: %1 bpm").arg(heartRate));
  QString hrColor = heartRate < 90 ? "#4caf50" : (heartRate < 110 ? "#ffc107" : "#f44336");
  heartRateLabel_->setStyleSheet(QString("color:%1; font-weight:bold;").arg(hrColor));

  workloadLabel_->setText(tr("负荷: %1").arg(workload));
  workloadLabel_->setStyleSheet(QString("color:#021a2f; font-weight:bold; background:%1; padding:6px;")
                                    .arg(workloadColor(workload).name()));

  // Update mouse and gaze traces
  const int nextX = QRandomGenerator::global()->bounded(0, 100);
  const int nextY = QRandomGenerator::global()->bounded(0, 100);
  mousePathSeries_->append(nextX, nextY);
  gazeSeries_->append((nextX + 10) % 100, (nextY + 15) % 100);

  if (mousePathSeries_->count() > 50) {
    mousePathSeries_->removePoints(0, 1);
    gazeSeries_->removePoints(0, 1);
  }
}

void RealTimeDisplayWindow::flashRandomKey() {
  const int r = QRandomGenerator::global()->bounded(4);
  const int c = QRandomGenerator::global()->bounded(10);
  auto *label = keyboardGrid_[r][c];
  label->setStyleSheet("background:#f44336; color:white; border:1px solid #1e90ff; padding:6px;");
  QTimer::singleShot(250, this, [label]() {
    label->setStyleSheet("background:#0f213a; color:#9cf; border:1px solid #1e90ff; padding:6px;");
  });
}

