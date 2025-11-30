#pragma once

#include <QtCharts/QBarSet>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QPolarChart>
#include <QPointF>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QTextEdit>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>
#include <QVector>
#include <vector>

QT_CHARTS_USE_NAMESPACE

class HeatmapWidget : public QWidget {
  Q_OBJECT

public:
  explicit HeatmapWidget(QWidget *parent = nullptr);
  void setData(const std::vector<std::vector<double>> &data);

protected:
  void paintEvent(QPaintEvent *event) override;

private:
  std::vector<std::vector<double>> data_{};
};

class PostMatchAnalysisWindow : public QWidget {
  Q_OBJECT

public:
  explicit PostMatchAnalysisWindow(QWidget *parent = nullptr);

  void setHeartRateSeries(const QVector<QPointF> &points);
  void setWorkloadSeries(const QVector<QPointF> &points);
  void setMouseTrajectory(const QVector<QPointF> &path, const QVector<QPointF> &clicks);
  void setGazeHeatmap(const std::vector<std::vector<double>> &heatmap);
  void setKeyboardHeatmap(const std::vector<std::vector<double>> &heatmap);
  void setReport(const QString &summary, const QString &advantages, const QString &issues,
                 const QString &advice);

private slots:
  void simulateAnalysis();

private:
  QWidget *buildIdentityPanel();
  QChartView *buildLineChart();
  QChartView *buildRadarChart();
  QChartView *buildStackedBar();
  QWidget *buildHeatmapChart();
  QWidget *buildGazeHeatmap();

  QLabel *summaryLabel_{};
  QTextEdit *advantageBox_{};
  QTextEdit *issueBox_{};
  QTextEdit *adviceBox_{};

  QLineSeries *heartRateSeries_{};
  QLineSeries *workloadSeries_{};
  QPolarChart *radarChart_{};
  QBarSet *focusSet_{};
  HeatmapWidget *gazeHeatmap_{};
  HeatmapWidget *keyboardHeatmap_{};
};

