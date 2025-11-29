#pragma once

#include <QtCharts/QBarSet>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QPolarChart>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QTextEdit>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

QT_CHARTS_USE_NAMESPACE

class PostMatchAnalysisWindow : public QWidget {
  Q_OBJECT

public:
  explicit PostMatchAnalysisWindow(QWidget *parent = nullptr);

private slots:
  void simulateAnalysis();

private:
  QWidget *buildIdentityPanel();
  QChartView *buildLineChart();
  QChartView *buildRadarChart();
  QChartView *buildStackedBar();
  QChartView *buildHeatmapChart();

  QLabel *summaryLabel_{};
  QTextEdit *advantageBox_{};
  QTextEdit *issueBox_{};
  QTextEdit *adviceBox_{};

  QLineSeries *heartRateSeries_{};
  QLineSeries *workloadSeries_{};
  QPolarChart *radarChart_{};
  QBarSet *focusSet_{};
  QChart *heatmapChart_{};
};

