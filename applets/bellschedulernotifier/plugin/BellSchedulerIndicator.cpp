/*
 * Copyright (C) 2015 Dominik Haumann <dhaumann@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
#include "BellSchedulerIndicator.h"
#include "BellSchedulerIndicatorUtils.h"

#include <KLocalizedString>
#include <KFormat>
#include <KNotification>
#include <KRun>
#include <QTimer>
#include <QStandardPaths>
#include <QDebug>
#include <QFile>
#include <QThread>
#include <QtCore/QStringList>
#include <QJsonObject>

#include <variant.hpp>
#include <json.hpp>

using namespace edupals;
using namespace std;



BellSchedulerIndicator::BellSchedulerIndicator(QObject *parent)
    : QObject(parent)
    , m_timer(new QTimer(this))
    , m_timer_run(new QTimer(this))
    , m_utils(new BellSchedulerIndicatorUtils(this))
    
{
    

    TARGET_FILE.setFileName("/tmp/.BellScheduler/bellscheduler-token");
   	
    connect(m_timer, &QTimer::timeout, this, &BellSchedulerIndicator::worker);
    m_timer->start(2000);
    worker();
}    


void BellSchedulerIndicator::worker(){


    if (!is_working){
        if (BellSchedulerIndicator::TARGET_FILE.exists() ) {
            getBellInfo();
            isAlive();
            
        }
    }        

     
}    

void BellSchedulerIndicator::showNotification(QString notType,int index){

	QString duration_label=i18n("Duration: ");
	QString bell;
	QString hour;
	QString duration;
		
	if (m_utils->bellsInfo[index]["duration"].get_int32()==999){
		QString error=i18n("Error. Not Available");
		hour=error;
		bell="";
		duration=error;
	}else{
		hour=QString::fromStdString(m_utils->bellsInfo[index]["hour"]);
		bell=QString::fromStdString(m_utils->bellsInfo[index]["name"]);
		if (m_utils->bellsInfo[index]["duration"].get_int32()==0){
			duration=i18n("Full reproduction");
		}else{
			QString label=i18n(" seconds");
			QString s = QString::number(m_utils->bellsInfo[index]["duration"].get_int32());
			duration=s+label;

		}	

	}	
	if (notType=="start"){
		notificationTitle=i18n("Playing the bell:");
		notificationBody="- "+hour+ " "+bell+"\n- "+duration_label+duration;
		m_bellPlayingNotification = KNotification::event(QStringLiteral("Run"),notificationTitle,notificationBody,"bell-scheduler-indicator", nullptr, KNotification::CloseOnTimeout , QStringLiteral("bellschedulernotifier"));
	    QString name = i18n("Stop now");
	    m_bellPlayingNotification->setDefaultAction(name);
	    m_bellPlayingNotification->setActions({name});
	    connect(m_bellPlayingNotification, QOverload<unsigned int>::of(&KNotification::activated), this, &BellSchedulerIndicator::stopBell);
	}else{
		notificationTitle=i18n("The bell has ended:");
		notificationBody="- "+hour+" "+bell+"\n- "+duration_label+duration;
		m_bellPlayingNotification = KNotification::event(QStringLiteral("Run"),notificationTitle,notificationBody, "bell-scheduler-indicator", nullptr, KNotification::CloseOnTimeout , QStringLiteral("bellschedulernotifier"));
	}    

}

void BellSchedulerIndicator::getBellInfo(){

    is_working=true;
    m_utils->getBellInfo();

    for (int i=0;i<m_utils->bellsInfo.count();i++){
    	if (!bellsnotification.contains(QString::fromStdString(m_utils->bellsInfo[i]["bellId"]))){
    		bellsnotification.push_back(QString::fromStdString(m_utils->bellsInfo[i]["bellId"]));
 	  		showNotification("start",i);
 	  	}	
    }
   
}

void BellSchedulerIndicator::isAlive(){

	bellToken=false;
	changeTryIconState(0);
	connect(m_timer_run, &QTimer::timeout, this, &BellSchedulerIndicator::checkStatus);
    m_timer_run->start(1000);
    checkStatus();


}

bool BellSchedulerIndicator::areBellsLive(){


	auto[bellsLive,removeBells]=m_utils->areBellsLive();
	variant::Variant tmpList=variant::Variant::create_array(0);
	if (bellsLive){
		if (removeBells.size()>0){
			for (int i=0;i<m_utils->bellsInfo.count();i++){
				if (removeBells.contains(QString::fromStdString(m_utils->bellsInfo[i]["bellId"]))){
					showNotification("end",i);

				}else{
					tmpList.append(m_utils->bellsInfo[i]);

				}
			}
			m_utils->bellsInfo=tmpList;
		}
	}

	return bellsLive;


}



void BellSchedulerIndicator::checkStatus(){

	if (BellSchedulerIndicator::TARGET_FILE.exists() ) { 
		if (!bellToken){
			bellToken=true;
			
		}
	}else{
		if (areBellsLive()){
			bellToken=true;
		}else{
			bellToken=false;
		}	

	}	

	if (bellToken){

		if (BellSchedulerIndicator::TARGET_FILE.exists()){ 
			if (checkToken>2){
				if (m_utils->isTokenUpdated()){
					checkToken=0;
					getBellInfo();	
				}
			}	

		}	
		checkToken=checkToken+1;
		m_utils->linkBellPid();

	}else{
		for (int i=0;i<m_utils->bellsInfo.count();i++){
			showNotification("end",i);
		}	
		m_utils->bellsInfo=variant::Variant::create_array(0);			
		m_timer_run->stop();
		changeTryIconState(1);
		is_working=false;
		QStringList emptyList;
		m_utils->bellsId=emptyList;
		bellsnotification=emptyList;
		checkToken=0;
	}
	
}


BellSchedulerIndicator::TrayStatus BellSchedulerIndicator::status() const
{
    return m_status;
}



void BellSchedulerIndicator::changeTryIconState(int state){

    const QString tooltip(i18n("Bell Scheduler"));

    if (state==0){
        setStatus(ActiveStatus);
        setToolTip(tooltip);
        setSubToolTip(notificationTitle+"\n"+notificationBody);
        setIconName("bellschedulernotifier");
       

    }else{
        setStatus(PassiveStatus);
    }
    


}

void BellSchedulerIndicator::stopBell(){

    m_utils->stopBell();
}


void BellSchedulerIndicator::setStatus(BellSchedulerIndicator::TrayStatus status)
{

    if (m_status != status) {
        m_status = status;
        emit statusChanged();
    }
}

QString BellSchedulerIndicator::iconName() const
{
    return m_iconName;
}

void BellSchedulerIndicator::setIconName(const QString &name)
{
    if (m_iconName != name) {
        m_iconName = name;
        emit iconNameChanged();
    }
}

QString BellSchedulerIndicator::toolTip() const
{
    return m_toolTip;
}

void BellSchedulerIndicator::setToolTip(const QString &toolTip)
{
    if (m_toolTip != toolTip) {
        m_toolTip = toolTip;
        emit toolTipChanged();
    }
}

QString BellSchedulerIndicator::subToolTip() const
{
    return m_subToolTip;
}

void BellSchedulerIndicator::setSubToolTip(const QString &subToolTip)
{
    if (m_subToolTip != subToolTip) {
        m_subToolTip = subToolTip;
        emit subToolTipChanged();
    }
}
