#include <QApplication>
#include <QTabWidget>

#include "PostMatchAnalysisWindow.h"
#include "RealTimeDisplayWindow.h"

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);
  QTabWidget tabs;
  tabs.setWindowTitle(QObject::tr("人因信息展示与分析"));

  auto *realtime = new RealTimeDisplayWindow();
  auto *postMatch = new PostMatchAnalysisWindow();

  tabs.addTab(realtime, QObject::tr("实时展示"));
  tabs.addTab(postMatch, QObject::tr("赛后分析"));
  tabs.resize(1280, 720);
  tabs.show();

  return app.exec();
}

