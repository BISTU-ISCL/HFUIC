#pragma once

#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QScatterSeries>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QStringList>
#include <QTimer>
#include <QVBoxLayout>
#include <QVector>
#include <QWidget>

QT_CHARTS_USE_NAMESPACE

struct RealTimeSample {
  QString playerId;
  QString emotion;
  int heartRate{}; // bpm
  QString workloadLevel; // 低/中/高
  bool faceDetected{};
  QPointF mousePosition; // 0-100 normalized
  bool mouseClicked{};
  QPointF gazePoint; // 0-100 normalized
  QStringList pressedKeys;
};

class RealTimeDisplayWindow : public QWidget {
  Q_OBJECT

public:
  explicit RealTimeDisplayWindow(QWidget *parent = nullptr);
  void updateSample(const RealTimeSample &sample);
  void updateGazeHeatmap(const QVector<QVector<double>> &heatmap);

private slots:
  void simulateDataUpdate();
  void decayHeatmap();

private:
  QWidget *buildPlayerPanel();
  QWidget *buildRightPanel();
  QWidget *buildKeyboardHeatmap();
  QChartView *buildMousePathChart();
  QChartView *buildGazeQuadrantChart();
  void flashKeys(const QStringList &keys);
  QColor keyColor(int count) const;

  QLabel *identityLabel_{};
  QLabel *emotionLabel_{};
  QLabel *heartRateLabel_{};
  QLabel *workloadLabel_{};
  QLabel *faceAlertLabel_{};
  QLabel *keyboardGrid_[4][10]{};
  int keyCounts_[4][10]{};
  QLineSeries *mousePathSeries_{};
  QScatterSeries *mouseClickSeries_{};
  QLineSeries *gazeSeries_{};
  QScatterSeries *gazePointSeries_{};
  QScatterSeries *gazeQuadrantPoint_{};
  QTimer dataTimer_{};
  QTimer heatmapDecayTimer_{};
};

