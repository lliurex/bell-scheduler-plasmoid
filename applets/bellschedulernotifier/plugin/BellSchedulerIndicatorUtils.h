#ifndef PLASMA_BELL_SCHEDULER_INDICATOR_UTILS_H
#define PLASMA_BELL_SCHEDULER_INDICATOR_UTILS_H

#include <QObject>
#include <QProcess>
#include <QFile>
#include <QMap>
#include <n4d.hpp>

using namespace std;
using namespace edupals;


class BellSchedulerIndicatorUtils : public QObject
{
    Q_OBJECT


public:
   

   BellSchedulerIndicatorUtils(QObject *parent = nullptr);

   QString refPath="/tmp/.BellScheduler";
   QString tokenPath="/tmp/.BellScheduler/newbellscheduler-token";
   QMap<QString, QVariantMap> bellsInfo;

   void startWidget();
   void readBellToken();
   void getRunningBells();   
   void syncBellInfo(QList<QJsonObject> pidInfo, QStringList bellsPid);
   void stopBell();
   QStringList getBellData(QString bellId);

  signals:
      void startWidgetFinished(bool startOk,bool initWorker);
      void readBellTokenFinished();
      void getRunningBellsFinished(QList<QJsonObject> pidInfo, QStringList bellsPid);
      void requestCloseNotification(QString notType, QString bellId);
      void stopBellFinished();

private:    
     
   QString user;
   n4d::Client client;
    
   QProcess *m_process = nullptr;
    
   void cleanCache();
   QString getInstalledVersion();
   void readToken();
   string  getFormatHour(int hour,int minute);
     
};

#endif // PLASMA_BELL_SCHEDULER_INDICATOR_UTILS
