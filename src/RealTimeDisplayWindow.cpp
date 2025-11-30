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
#include <algorithm>
#include <cmath>

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

QColor gradientHeat(double ratio) {
  ratio = std::clamp(ratio, 0.0, 1.0);
  const int r = 30 + static_cast<int>(ratio * 200);
  const int g = 80 + static_cast<int>((1 - ratio) * 120);
  const int b = 140 + static_cast<int>((1 - ratio) * 60);
  return QColor(r, g, b);
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

  connect(&heatmapDecayTimer_, &QTimer::timeout, this, &RealTimeDisplayWindow::decayHeatmap);
  heatmapDecayTimer_.start(800);
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
  chart->setTitle(tr("鼠标轨迹与注视"));
  chart->legend()->setVisible(true);

  mousePathSeries_ = new QLineSeries();
  mousePathSeries_->setName(tr("鼠标移动"));

  mouseClickSeries_ = new QScatterSeries();
  mouseClickSeries_->setName(tr("点击"));
  mouseClickSeries_->setMarkerSize(10);
  mouseClickSeries_->setColor(Qt::red);

  gazeSeries_ = new QLineSeries();
  gazeSeries_->setName(tr("注视轨迹"));
  gazeSeries_->setPen(QPen(QColor(30, 144, 255), 3, Qt::DotLine));

  gazePointSeries_ = new QScatterSeries();
  gazePointSeries_->setName(tr("当前注视点"));
  gazePointSeries_->setColor(QColor(0, 191, 255));
  gazePointSeries_->setMarkerSize(12);

  chart->addSeries(mousePathSeries_);
  chart->addSeries(mouseClickSeries_);
  chart->addSeries(gazeSeries_);
  chart->addSeries(gazePointSeries_);
  chart->createDefaultAxes();
  chart->axes(Qt::Horizontal).first()->setRange(0, 100);
  chart->axes(Qt::Vertical).first()->setRange(0, 100);

  auto *view = new QChartView(chart);
  view->setRenderHint(QPainter::Antialiasing);
  return view;
}

QChartView *RealTimeDisplayWindow::buildGazeQuadrantChart() {
  auto *chart = new QChart();
  chart->setTitle(tr("注视点象限"));

  gazeQuadrantPoint_ = new QScatterSeries();
  gazeQuadrantPoint_->setName(tr("注视点"));
  gazeQuadrantPoint_->setMarkerSize(12);
  gazeQuadrantPoint_->setColor(QColor(0, 191, 255));
  chart->addSeries(gazeQuadrantPoint_);
  chart->createDefaultAxes();
  chart->axes(Qt::Horizontal).first()->setRange(0, 100);
  chart->axes(Qt::Vertical).first()->setRange(0, 100);

  auto *view = new QChartView(chart);
  view->setRenderHint(QPainter::Antialiasing);
  view->setStyleSheet("background:#081423; border:1px solid #1e90ff;");
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
  auto *gazeQuadrant = buildGazeQuadrantChart();

  layout->addWidget(screenArea, 4);
  layout->addWidget(chartView, 4);
  layout->addWidget(gazeQuadrant, 3);
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

void RealTimeDisplayWindow::updateSample(const RealTimeSample &sample) {
  identityLabel_->setText(tr("ID: %1").arg(sample.playerId));
  faceAlertLabel_->setText(sample.faceDetected ? tr("人脸检测: 正常") : tr("人脸检测: 异常"));
  faceAlertLabel_->setStyleSheet(QString("color:%1;font-weight:bold;")
                                     .arg(sample.faceDetected ? "#8bc34a" : "#f44336"));

  emotionLabel_->setText(tr("情绪: %1").arg(sample.emotion));
  emotionLabel_->setStyleSheet(QString("color:#021a2f; font-weight:bold; background:%1; padding:6px;")
                                   .arg(emotionColor(sample.emotion).name()));

  heartRateLabel_->setText(tr("心率: %1 bpm").arg(sample.heartRate));
  QString hrColor = sample.heartRate < 90 ? "#4caf50" : (sample.heartRate < 110 ? "#ffc107" : "#f44336");
  heartRateLabel_->setStyleSheet(QString("color:%1; font-weight:bold;").arg(hrColor));

  workloadLabel_->setText(tr("负荷: %1").arg(sample.workloadLevel));
  workloadLabel_->setStyleSheet(QString("color:#021a2f; font-weight:bold; background:%1; padding:6px;")
                                    .arg(workloadColor(sample.workloadLevel).name()));

  mousePathSeries_->append(sample.mousePosition);
  if (mousePathSeries_->count() > 120) {
    mousePathSeries_->removePoints(0, mousePathSeries_->count() - 120);
  }

  if (sample.mouseClicked) {
    mouseClickSeries_->append(sample.mousePosition);
    if (mouseClickSeries_->count() > 60) {
      mouseClickSeries_->removePoints(0, mouseClickSeries_->count() - 60);
    }
  }

  gazeSeries_->append(sample.gazePoint);
  gazeQuadrantPoint_->clear();
  gazeQuadrantPoint_->append(sample.gazePoint);
  gazePointSeries_->clear();
  gazePointSeries_->append(sample.gazePoint);
  if (gazeSeries_->count() > 120) {
    gazeSeries_->removePoints(0, gazeSeries_->count() - 120);
  }

  flashKeys(sample.pressedKeys);
}

void RealTimeDisplayWindow::flashKeys(const QStringList &keys) {
  for (const auto &key : keys) {
    for (int r = 0; r < 4; ++r) {
      for (int c = 0; c < 10; ++c) {
        if (keyboardGrid_[r][c]->text().compare(key, Qt::CaseInsensitive) == 0) {
          keyCounts_[r][c] = std::min(50, keyCounts_[r][c] + 5);
          keyboardGrid_[r][c]->setStyleSheet(QString("background:%1; color:white; border:1px solid #1e90ff; padding:6px;")
                                                 .arg(keyColor(keyCounts_[r][c]).name()));
        }
      }
    }
  }
}

QColor RealTimeDisplayWindow::keyColor(int count) const {
  const double ratio = std::clamp(count / 50.0, 0.0, 1.0);
  return gradientHeat(ratio);
}

void RealTimeDisplayWindow::updateGazeHeatmap(const QVector<QVector<double>> &heatmap) {
  for (int r = 0; r < 4; ++r) {
    for (int c = 0; c < 10; ++c) {
      if (r < heatmap.size() && c < heatmap[r].size()) {
        const double ratio = heatmap[r][c];
        keyboardGrid_[r][c]->setStyleSheet(QString("background:%1; color:white; border:1px solid #1e90ff; padding:6px;")
                                               .arg(gradientHeat(ratio).name()));
      }
    }
  }
}

void RealTimeDisplayWindow::simulateDataUpdate() {
  RealTimeSample sample;
  sample.playerId = tr("选手A");
  sample.emotion = randomEmotion();
  sample.heartRate = QRandomGenerator::global()->bounded(60, 140);
  sample.workloadLevel = randomWorkload();
  sample.faceDetected = QRandomGenerator::global()->bounded(10) > 1;
  sample.mousePosition = QPointF(QRandomGenerator::global()->bounded(0, 100),
                                 QRandomGenerator::global()->bounded(0, 100));
  sample.mouseClicked = QRandomGenerator::global()->bounded(3) == 0;
  sample.gazePoint = QPointF(fmod(sample.mousePosition.x() + 10, 100),
                             fmod(sample.mousePosition.y() + 15, 100));
  if (QRandomGenerator::global()->bounded(2) == 0) sample.pressedKeys << "Q" << "E";

  updateSample(sample);
}

void RealTimeDisplayWindow::decayHeatmap() {
  for (int r = 0; r < 4; ++r) {
    for (int c = 0; c < 10; ++c) {
      keyCounts_[r][c] = std::max(0, keyCounts_[r][c] - 1);
      if (keyCounts_[r][c] == 0) {
        keyboardGrid_[r][c]->setStyleSheet("background:#0f213a; color:#9cf; border:1px solid #1e90ff; padding:6px;");
      } else {
        keyboardGrid_[r][c]->setStyleSheet(QString("background:%1; color:white; border:1px solid #1e90ff; padding:6px;")
                                               .arg(keyColor(keyCounts_[r][c]).name()));
      }
    }
  }
}
