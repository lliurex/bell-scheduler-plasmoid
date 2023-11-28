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

#include <n4d.hpp>
#include <variant.hpp>
#include <json.hpp>

#include <tuple>
#include <sys/types.h>
#include <QDebug>

using namespace edupals;
using namespace std;


BellSchedulerIndicatorUtils::BellSchedulerIndicatorUtils(QObject *parent)
    : QObject(parent)
       
{
    n4d::Client client;
    client=n4d::Client("https://127.0.0.1:9779");
    BELLS_TOKEN.setFileName("tmp/.BellScheduler/bellscheduler-token");
  
}    

string BellSchedulerIndicatorUtils::getFormatHour(int hour,int minute){

    std::stringstream shour;
    shour<<hour;
    std::stringstream sminute;
    sminute<<minute;
    string format_hour="";
    string format_minute="";
  
    if (hour<10){
        format_hour='0'+shour.str();
    }else{
        format_hour=shour.str();

    }

    if (minute<10){
        format_minute='0'+sminute.str();
    }else{
        format_minute=sminute.str();

    }

   string format_time=format_hour+":"+format_minute;
   
   return format_time;

}

variant::Variant BellSchedulerIndicatorUtils::readToken(){

    bool error=false;
    variant::Variant tmp =variant::Variant::create_array(0);
 
    QFile file("/tmp/.BellScheduler/bellscheduler-token");

    file.open(QIODevice::ReadOnly);
    QString s;
    QTextStream s1(&file);
 
    QString line;
    QStringList bells_id;
    while (s1.readLineInto(&line)) {
        bells_id.push_back(line);
    } 
    file.close();
    
    try{
        variant::Variant bell_info = client.call("BellSchedulerManager","read_conf");

       
        for(int i=0 ; i < bells_id.length() ; i++){
            try{    
                variant::Variant info=variant::Variant::create_struct();  
                string bellId=bells_id[i].toStdString();
                int hour=bell_info["data"][bellId]["hour"].get_int32();
                int minute=bell_info["data"][bellId]["minute"].get_int32();
                string format_hour=getFormatHour(hour,minute);
                int duration=bell_info["data"][bellId]["play"]["duration"];

                info["bellId"]=bellId;
                info["name"]=bell_info["data"][bellId]["name"].get_string();
                info["hour"]=format_hour;
                info["duration"]=duration;
                info["bellPID"]="";
                tmp.append(info);

            }catch(...){
                error=true;

            }    
                
        }
       
    }catch (...){
        error=true;

    } 

    if (error) {
        variant::Variant empty =variant::Variant::create_array(0);
        tmp=empty;
        for(int i=0 ; i < bells_id.length() ; i++){
            variant::Variant info=variant::Variant::create_struct();  
            string bellId=bells_id[i].toStdString();
            info["bellId"]=bellId;
            info["name"]="";
            info["hour"]="";
            info["duration"]=999;
            info["bellPID"]="";
            tmp.append(info);
        }         

    }      
    
   return tmp;


}

void BellSchedulerIndicatorUtils::getBellInfo(){

    variant::Variant token_content = readToken();

    if (bellsInfo.count()>0){
        for (int i=0;i<bellsInfo.count();i++){
            if (!bellsId.contains(QString::fromStdString(bellsInfo[i]["bellId"]))){
                bellsId.push_back(QString::fromStdString(bellsInfo[i]["bellId"]));
            }    
        }
    } 
    
    for (int i=0;i<token_content.count();i++){
        QString id=QString::fromStdString(token_content[i]["bellId"]);
        if (!bellsId.contains(id)){
            bellsInfo.append(token_content[i]);
            
        }   
    } 

}

std::tuple<QList<QJsonObject>, QStringList> BellSchedulerIndicatorUtils::getBellPid(){

    QStringList bellPid;
    QList<QJsonObject>pidInfo;

    QProcess process;
    QString cmd="ps -ef | grep '/usr/bin/BellSchedulerPlayer' | grep -v 'grep'";
    
    process.start("/bin/sh", QStringList()<< "-c" 
                       << cmd);
    process.waitForFinished(-1);
    QString stdout=process.readAllStandardOutput();
    QString stderr=process.readAllStandardError();
    QStringList lst=stdout.split("\n");

    if (lst.length()>0){
        for (int i=0;i<lst.length();i++){
            QStringList tmp_list;
            QJsonObject tmp_pid{
                {"bellId",""},
                {"pidParent",""},
                {"bellPID",""}
            };
            int cont=0;
            QStringList processed_line=lst[i].split(" ");
            if (processed_line.length()>=10){
                for (int i=0;i<processed_line.length();i++){
                    if (processed_line[i].compare("")!=0){
                        tmp_list.push_back(processed_line[i]);
                    }
                }    
                processed_line=tmp_list;
                tmp_pid["bellId"]=processed_line[9];
                tmp_pid["pidParent"]=processed_line[2];
                tmp_pid["bellPID"]=processed_line[1];
                if (pidInfo.length()>0){
                    for (int i=0;i<pidInfo.length();i++){
                        if (pidInfo[i]["bellId"]==tmp_pid["bellId"]){
                            cont=cont+1;
                        }
                    }
                }
                if (cont==0){
                    pidInfo.append(tmp_pid);
                }
                if (!bellPid.contains(processed_line[1])){
                    bellPid.append(processed_line[1]);

                } 

               
          }
        }
    }    
    
    return {pidInfo,bellPid};

}

std::tuple<bool,QStringList> BellSchedulerIndicatorUtils::areBellsLive(){

   auto[pidInfo,bellPid]=getBellPid();

   bool bellsLive=false;
   QStringList removeBells;

   if (bellPid.size()>0){
        bellsLive=true;
        for (int i=0;i<bellsInfo.count();i++){
            QString bellPID=QString::fromStdString(bellsInfo[i]["bellPID"]);
            if (bellPID!=""){
                if (!bellPid.contains(bellPID)){
                    removeBells.push_back(QString::fromStdString(bellsInfo[i]["bellId"]));
                } 
            }

        }
    }    
    
    return {bellsLive,removeBells};    

}

void BellSchedulerIndicatorUtils::linkBellPid(){

    int cont=0;
    auto[pidInfo,bellPid]=getBellPid();

    for (int i=0;i<bellsInfo.count();i++){
       QString bellPID=QString::fromStdString(bellsInfo[i]["bellPID"]);
       if (bellPID==""){
            cont=cont+1;
       }
    }

    if (cont>0){
        for (int i=0; i<pidInfo.size();i++){
            for (int j=0;j<bellsInfo.count();j++){
                QString bellID=QString::fromStdString(bellsInfo[j]["bellId"]);
                QString bellID2=pidInfo[i]["bellId"].toString();
                if (bellID.compare(bellID2)==0){
                    QString bellPID=pidInfo[i]["bellPID"].toString();
                    bellsInfo[j]["bellPID"]=bellPID.toStdString();
                }

            }
            
        }

    }

}

bool BellSchedulerIndicatorUtils::isTokenUpdated(){

    tokenUpdated=false;

    QDateTime currentTime=QDateTime::currentDateTime();
    QDateTime lastModification=QFileInfo(BELLS_TOKEN).lastModified();    

    qint64 millisecondsDiff = lastModification.msecsTo(currentTime);


    if (millisecondsDiff<MOD_FRECUENCY){
        tokenUpdated=true;
    }
    return tokenUpdated;

}

void BellSchedulerIndicatorUtils::stopBell(){

    client.call("BellSchedulerManager","stop_bell");
    
}

QStringList BellSchedulerIndicatorUtils::getBellData(int bellId){

     QStringList bellData;
     QString hour;
     QString bell;
     QString duration;

     if (bellsInfo[bellId]["duration"].get_int32()==999){
        hour="";
        bell="";
        duration="error";
    }else{
        hour=QString::fromStdString(bellsInfo[bellId]["hour"]);
        bell=QString::fromStdString(bellsInfo[bellId]["name"]);
        if (bellsInfo[bellId]["duration"].get_int32()==0){
            duration="full";
        }else{
            QString s = QString::number(bellsInfo[bellId]["duration"].get_int32());
            duration=s;

        }   

    } 
    bellData.append(hour);
    bellData.append(bell);
    bellData.append(duration);
    bellData.append(QString::number(bellPid.count()));
    
    return bellData;  

}
