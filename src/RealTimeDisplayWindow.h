#pragma once

#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

QT_CHARTS_USE_NAMESPACE

class RealTimeDisplayWindow : public QWidget {
  Q_OBJECT

public:
  explicit RealTimeDisplayWindow(QWidget *parent = nullptr);

private slots:
  void simulateDataUpdate();
  void flashRandomKey();

private:
  QWidget *buildPlayerPanel();
  QWidget *buildRightPanel();
  QWidget *buildKeyboardHeatmap();
  QChartView *buildMousePathChart();

  QLabel *identityLabel_{};
  QLabel *emotionLabel_{};
  QLabel *heartRateLabel_{};
  QLabel *workloadLabel_{};
  QLabel *faceAlertLabel_{};
  QLabel *keyboardGrid_[4][10]{};
  QLineSeries *mousePathSeries_{};
  QLineSeries *gazeSeries_{};
  QTimer dataTimer_{};
  QTimer keyboardTimer_{};
};

