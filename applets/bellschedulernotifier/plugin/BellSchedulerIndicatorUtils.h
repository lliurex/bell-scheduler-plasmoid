#ifndef PLASMA_BELL_SCHEDULER_INDICATOR_UTILS_H
#define PLASMA_BELL_SCHEDULER_INDICATOR_UTILS_H

#include <QObject>
#include <QProcess>
#include <QFile>

#include <n4d.hpp>
#include <variant.hpp>

using namespace std;
using namespace edupals;


class BellSchedulerIndicatorUtils : public QObject
{
    Q_OBJECT


public:
   

   BellSchedulerIndicatorUtils(QObject *parent = nullptr);

   void getBellInfo();
   void linkBellPid();
   void stopBell();
   std::tuple<bool, QStringList> areBellsLive();
   bool isTokenUpdated();
   QStringList getBellData(int bellId);

   QStringList bellsId;
   variant::Variant bellsInfo =variant::Variant::create_array(0);
   //bool areBellsLive();
   bool tokenUpdated=false;

private:    
     
	 variant::Variant readToken();
    std::tuple<QList<QJsonObject>, QStringList> getBellPid();	
    string  getFormatHour(int hour,int minute);
    n4d::Client client;
    QFile BELLS_TOKEN;
    qint64 MOD_FRECUENCY=2000;
    QStringList bellPid;
    
     
};



#endif // PLASMA_LLIUREX_UP_INDICATOR_UTILS_H
