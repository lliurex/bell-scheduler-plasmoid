#include "BellSchedulerIndicatorUtils.h"

#include <QFile>
#include <QDateTime>
#include <QFileInfo>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QDebug>
#include <QTextStream>
#include <QJsonObject>
#include <QList>
#include <QPointer>
#include <QThreadPool>

#include <QtConcurrent>

#include <n4d.hpp>

#include <tuple>
#include <sys/types.h>

using namespace edupals;
using namespace std;


BellSchedulerIndicatorUtils::BellSchedulerIndicatorUtils(QObject *parent)
    : QObject(parent)
       
{
    user=qgetenv("USER");
  
} 

void BellSchedulerIndicatorUtils::startWidget(){

    QPointer<BellSchedulerIndicatorUtils>safeThis(this);

    QThreadPool::globalInstance()->start([safeThis](){

        if (!safeThis){
            return;
        }

        bool startOk=false;
        bool initWorker=false;

        try{
            safeThis->cleanCache();
            safeThis->client=n4d::Client("https://127.0.0.1:9779");
            if (!QFileInfo::exists(safeThis->refPath)){
                QDir basePath("/tmp/");
                basePath.mkdir(".BellScheduler");
            }else{
                initWorker=true;
            }
            startOk=true;
        }catch (std::exception& e){
            qDebug()<<"[BELL-SCHEDULER-INDICATOR]: Error creating n4d client: " <<e.what();
        } 

        if (safeThis){
            emit safeThis->startWidgetFinished(startOk,initWorker);
        }

    });
}

void BellSchedulerIndicatorUtils::cleanCache(){

    qDebug()<<"[BELL-SCHEDULER-INDICATOR]: Clean cache";
    QFile CURRENT_VERSION_TOKEN;
    QDir cacheDir("/home/"+user+"/.cache/plasmashell/qmlcache");
    QString currentVersion="";
    bool clear=false;

    CURRENT_VERSION_TOKEN.setFileName("/home/"+user+"/.config/bell-scheduler-widget.conf");
    QString installedVersion=getInstalledVersion();

    if (!CURRENT_VERSION_TOKEN.exists()){
        if (CURRENT_VERSION_TOKEN.open(QIODevice::WriteOnly)){
            QTextStream data(&CURRENT_VERSION_TOKEN);
            data<<installedVersion;
            CURRENT_VERSION_TOKEN.close();
            clear=true;
        }
    }else{
        if (CURRENT_VERSION_TOKEN.open(QIODevice::ReadOnly)){
            QTextStream content(&CURRENT_VERSION_TOKEN);
            currentVersion=content.readLine();
            CURRENT_VERSION_TOKEN.close();
        }
        if (currentVersion!=installedVersion){
            if (CURRENT_VERSION_TOKEN.open(QIODevice::WriteOnly)){
                QTextStream data(&CURRENT_VERSION_TOKEN);
                data<<installedVersion;
                CURRENT_VERSION_TOKEN.close();
                clear=true;
            }
        }
    } 
    if (clear){
        if (cacheDir.exists()){
            cacheDir.removeRecursively();
        }
    }   

}

QString BellSchedulerIndicatorUtils::getInstalledVersion(){

    QFile INSTALLED_VERSION_TOKEN;
    QString installedVersion="";
    
    INSTALLED_VERSION_TOKEN.setFileName("/var/lib/bell-scheduler-indicator/version");

    if (INSTALLED_VERSION_TOKEN.exists()){
        if (INSTALLED_VERSION_TOKEN.open(QIODevice::ReadOnly)){
            QTextStream content(&INSTALLED_VERSION_TOKEN);
            installedVersion=content.readLine();
            INSTALLED_VERSION_TOKEN.close();
        }
    }
    return installedVersion;

}

void BellSchedulerIndicatorUtils::readBellToken(){

    QPointer<BellSchedulerIndicatorUtils>safeThis(this);

    QThreadPool::globalInstance()->start([safeThis](){

        if (!safeThis){
            return;
        }
        safeThis->readToken();
        if (safeThis){
            emit safeThis->readBellTokenFinished();
        }
    });

} 

void BellSchedulerIndicatorUtils::readToken(){

    qDebug()<<"[BELL-SCHEDULER-INDICATOR]: Read Token";

    QFile tokenFile(tokenPath);

    if (tokenFile.exists() && tokenFile.open(QIODevice::ReadOnly)) {
        QTextStream content(&tokenFile);
        
        while (!content.atEnd()) {
            QString tmpLine = content.readLine().trimmed();
            
            if (!tmpLine.isEmpty()) {
                QStringList tmpBell = tmpLine.split("###");
                QString bellId = tmpBell[0];
                if (!bellId.isEmpty()){
                   if (!bellsInfo.contains(bellId)) {
                        QVariantMap info;
                        if (tmpBell.size() == 4) {
                            info["name"] = tmpBell[1];
                            info["hour"] = tmpBell[2];
                            info["duration"] = tmpBell[3].toInt();
                            info["bellPID"] = "";
                        }else{
                            info["name"] = "";
                            info["hour"] = "";
                            info["duration"] = 999;
                            info["bellPID"] = "";
                        }
                        bellsInfo.insert(bellId, info);
                    }
                }
            }
        }
        tokenFile.close();
    }
}

void BellSchedulerIndicatorUtils::getRunningBells(){

    if (m_process && m_process->state() != QProcess::NotRunning) {
        return;
    }

    if (!m_process) {
        m_process = new QProcess(this);
    }
    m_process->disconnect();
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                this, [this](int exitCode, QProcess::ExitStatus exitStatus) {

        QList<QJsonObject> pidInfo;
        QStringList bellsPid;

        if (exitStatus == QProcess::NormalExit) {
            QString output = m_process->readAllStandardOutput();
            QStringList lines = output.split('\n', Qt::SkipEmptyParts);

            for (const QString &line : lines) {
                QStringList parts = line.split(' ', Qt::SkipEmptyParts);
                if (parts.size() >= 2) {
                    QString bPID = parts[0];
                    QString bID = parts.last();

                    bool exists = std::any_of(pidInfo.begin(), pidInfo.end(), [&](const QJsonObject &obj){
                        return obj["bellId"].toString() == bID;
                    });
                    if (!exists) {
                        pidInfo.append(QJsonObject{{"bellId", bID}, {"bellPID", bPID}});
                    }
                    if (!bellsPid.contains(bPID)) bellsPid.append(bPID);
                }
            }   
        }
        emit getRunningBellsFinished(pidInfo, bellsPid);
        m_process->deleteLater();
        m_process=nullptr;
    });

    m_process->start("pgrep", QStringList() << "-fa" << "BellSchedulerPlayer");
}

void BellSchedulerIndicatorUtils::syncBellInfo(QList<QJsonObject> pidInfo, QStringList bellsPid){

   for (const QJsonObject &obj : pidInfo) {
        QString bID = obj["bellId"].toString();
        QString bPID = obj["bellPID"].toString();

        if (bellsInfo.contains(bID)) {
            bellsInfo[bID]["PID"] = bPID;
         }   
    }
    auto it=bellsInfo.begin();
    while (it != bellsInfo.end()) {
        QString bellId = it.key();
        QVariantMap &bellData = it.value();
        QString pidInData = bellData.value("PID").toString().trimmed();

        if (!bellsPid.contains(pidInData)) {
            emit requestCloseNotification("end", bellId);
            it = bellsInfo.erase(it);
        } else {
            ++it;
        }
    }

}

void BellSchedulerIndicatorUtils::stopBell(){

    qDebug()<<"[BELL-SCHEDULER-INDICATOR]: Stopping bell";

    QPointer<BellSchedulerIndicatorUtils>safeThis(this);

    QThreadPool::globalInstance()->start([safeThis](){

        if (!safeThis){
            return;
        }
        safeThis->readToken();
        try{
            safeThis->client.call("BellSchedulerManager","stop_bell");
        }catch(std::exception&e){
            qDebug()<<"[BELL-SCHEDULER-INDICATOR]: Stopping bell. Error: "<<e.what();
        }
        emit safeThis->stopBellFinished();
    });
       
}

QStringList BellSchedulerIndicatorUtils::getBellData(QString bellId){

    QStringList bellData;
    QString hour;
    QString bell;
    QString duration;

    QVariantMap currentBell = bellsInfo.value(bellId);
    int durationVal = currentBell["duration"].toInt();

    if (durationVal == 999) {
        hour = "";
        bell = "";
        duration = "error";
    }else {
        hour = currentBell["hour"].toString();
        bell = currentBell["name"].toString();
    
        if (durationVal == 0) {
            duration = "full";
        }else {
            duration = QString::number(durationVal);
        }
    }
    
    bellData.append(hour);
    bellData.append(bell);
    bellData.append(duration);
    
    return bellData;  

}
