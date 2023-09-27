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
   	
   	cout << "#######BELL INFO######" << endl;
    cout << bellsInfo << endl;

    connect(m_timer, &QTimer::timeout, this, &BellSchedulerIndicator::worker);
    m_timer->start(5000);
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

void BellSchedulerIndicator::getBellInfo(){

    is_working=true;
    variant::Variant tmp = m_utils->readToken();
    cout << "#############" << endl;
    cout << tmp << endl;
    QStringList bellsId;

   
   	qDebug()<<"vacia";
    if (bellsInfo.size()>0){
    	qDebug()<<"1"<<bellsInfo.size();
    	int bellsInfoLength=sizeof(bellsInfo)/sizeof(bellsInfo[0]);
    	if (bellsInfoLength>0){
    		qDebug()<<"2"<<bellsInfoLength;
	    	for (int i=0;i<bellsInfoLength+1;i++){
	    		qDebug()<<"3";
	    		bellsId.push_back(QString::fromStdString(bellsInfo[i]["bellId"]));

	    	}
		} 
	}

    int tmpLength=tmp.count();
    qDebug()<<"4A SIZE"<<tmp.size();
    qDebug()<<"4 LENGTH"<<tmpLength;
    for (int i=0;i<tmpLength;i++){
    	qDebug()<<"5";
    	QString id=QString::fromStdString(tmp[i]["bellId"]);
    	if (!bellsId.contains(id)){
    		qDebug()<<"6";
    		bellsInfo.append(tmp[i]);

			QString title=i18n("Playing the scheduled bell:");
			QString hour=QString::fromStdString(tmp[i]["hour"]);
			QString bell=QString::fromStdString(tmp[i]["name"]);
			QString duration_label=i18n("Duration: ");
			QString duration="";
			if (tmp[i]["duration"].get_int32()==0){
				duration=i18n("Full reproduction");
			}else{
				QString label=i18n(" seconds");
				QString s = QString::number(tmp[i]["duration"].get_int32());
				duration=s+label;
			}
			qDebug()<<"7";
			QString subtooltip=title+"\n-"+hour+" "+bell+"\n-"+duration_label+duration;
			m_bellPlayingNotification = KNotification::event(QStringLiteral("Run"), subtooltip, {}, "bell-scheduler-indicator", nullptr, KNotification::CloseOnTimeout , QStringLiteral("bellschedulernotifier"));
	        QString name = i18n("Stop now");
	        m_bellPlayingNotification->setDefaultAction(name);
	        m_bellPlayingNotification->setActions({name});
	        connect(m_bellPlayingNotification, QOverload<unsigned int>::of(&KNotification::activated), this, &BellSchedulerIndicator::stopBell);
			qDebug()<<"8";
		}   
	}	
	qDebug()<<"9";
	cout << bellsInfo << endl;

}

void BellSchedulerIndicator::isAlive(){

	qDebug()<<"Esta vivo";
	bellToken=false;
	changeTryIconState(0);
	connect(m_timer_run, &QTimer::timeout, this, &BellSchedulerIndicator::checkStatus);
    m_timer_run->start(10000);
    checkStatus();


}

bool BellSchedulerIndicator::areBellsLive(){

	bool bellsLive=false;
	auto[pidInfo,bellPid]=m_utils->getBellPid();

	qDebug()<<"VIENDO SI HAY ALARMAS VIVAS";
	qDebug()<<"NUMERO DE PIDS"<<bellPid.size();
	variant::Variant tmpInfo =variant::Variant::create_array(0);
	if (bellPid.size()>0){
		
		int bellsInfoLength=bellsInfo.count();
		qDebug()<<"NUMERO DE ALARMAS VIVAS"<<bellsInfoLength;
		for (int i=bellsInfoLength-1;i>=0;i--){
			qDebug()<<"VIENDO SI HAY ALARMAS MUERTAS";
			QString bellPID=QString::fromStdString(bellsInfo[i]["bellPID"]);
			qDebug()<<"ARE LIVE??"<<bellPID;
			if (bellPID!=""){
				if (!bellPid.contains(bellPID)){
					qDebug()<<"BELL TERMINADA"<<bellPID;
					QString title=i18n("The scheduled bell has ended");
					QString hour=QString::fromStdString(bellsInfo[i]["hour"]);
					QString bell=QString::fromStdString(bellsInfo[i]["name"]);
					QString duration_label=i18n("Duration: ");
					QString duration="";
					if (bellsInfo[i]["duration"].get_int32()==0){
						duration=i18n("Full reproduction");
					}else{
						QString label=i18n(" seconds");
						QString s = QString::number(bellsInfo[i]["duration"].get_int32());
						duration=s+label;
					}
					qDebug()<<"73";
					QString subtooltip=title+"\n-"+hour+" "+bell+"\n-"+duration_label+duration;
					m_bellStopNotification = KNotification::event(QStringLiteral("Run"), subtooltip, {}, "bell-scheduler-indicator", nullptr, KNotification::CloseOnTimeout , QStringLiteral("bellschedulernotifier"));
	       	 		//QString name = i18n("Stop now");
	        		//m_bellPlayingNotification->setDefaultAction(name);
	        		//m_bellPlayingNotification->setActions({name});
	        		connect(m_bellStopNotification, QOverload<unsigned int>::of(&KNotification::activated), this, &BellSchedulerIndicator::stopBell);
					qDebug()<<"238";
					
				}else{
					tmpInfo.append(bellsInfo[i]);
				}

			}

		}
		qDebug()<<"LISTA DE ALARMAS VIVAS";
		cout << tmpInfo << endl;
		qDebug()<<"LISTA DE ALARMAS ACTUALIZADA";
		bellsInfo=tmpInfo;
		cout << bellsInfo << endl;
		qDebug()<<"HAY VIVAS";
		bellsLive=true;
	}else{
		qDebug()<<"NO HAY";
	}
	return bellsLive;

}

void BellSchedulerIndicator::linkBellPid(){

	qDebug()<<"ASOCIANDO PID";
	int cont=0;
	auto[pidInfo,bellPid]=m_utils->getBellPid();

	int bellsInfoLength=bellsInfo.count();
	qDebug()<<"NUMERO DE ALARMAS"<<bellsInfoLength;
	qDebug()<<"SIZE ALARMAS"<<bellsInfo.size();
	for (int i=0;i<bellsInfoLength;i++){
		QString bellPID=QString::fromStdString(bellsInfo[i]["bellPID"]);
		if (bellPID==""){
			qDebug()<<"contando";
			cont=cont+1;
		}
	}
	qDebug()<<"CONTADOR DE PID";
	qDebug()<<cont;
	if (cont>0){
		qDebug()<<"ASOCIANDO PID START";
		int pidInfoLength=sizeof(pidInfo)/sizeof(pidInfo[0]);
		qDebug()<<"NUMERO PID INFO"<<pidInfoLength;
		qDebug()<<"SIZE PID INFO"<<pidInfo.size();
		for (int i=0; i<pidInfo.size();i++){
			
				for (int j=0;j<bellsInfoLength;j++){
					QString bellID=QString::fromStdString(bellsInfo[j]["bellId"]);
					qDebug()<<"#######3BellID"<<bellID;
					QString bellID2=pidInfo[i]["bellId"].toString();
					qDebug()<<"#########3BellID2"<<bellID2;
					if (bellID.compare(bellID2)==0){
						qDebug()<<"matc";
						QString bellPID=pidInfo[i]["bellPID"].toString();
						bellsInfo[j]["bellPID"]=bellPID.toStdString();
					}

				}
			
		}

	}
	qDebug()<<"RESULTADO LINK";
	cout << bellsInfo << endl ;	

}

void BellSchedulerIndicator::checkStatus(){

	qDebug()<<"Checheando";
	if (BellSchedulerIndicator::TARGET_FILE.exists() ) { 
		if (!bellToken){
			bellToken=true;
		}
	}else{
		if (areBellsLive()){
			bellToken=true;
		}else{
			bellToken=false;
			//m_timer_run->stop();
			//changeTryIconState(1);
		}	

	}	

	if (bellToken){
		qDebug()<<"Asociando token";
		linkBellPid();

	}else{
		for (int i=0;i<bellsInfo.count();i++){
			QString title=i18n("The scheduled bell has ended");
			QString hour=QString::fromStdString(bellsInfo[i]["hour"]);
			QString bell=QString::fromStdString(bellsInfo[i]["name"]);
			QString duration_label=i18n("Duration: ");
			QString duration="";
			if (bellsInfo[i]["duration"].get_int32()==0){
				duration=i18n("Full reproduction");
			}else{
				QString label=i18n(" seconds");
				QString s = QString::number(bellsInfo[i]["duration"].get_int32());
				duration=s+label;
			}
			qDebug()<<"73";
			QString subtooltip=title+"\n-"+hour+" "+bell+"\n-"+duration_label+duration;
			m_bellStopNotification = KNotification::event(QStringLiteral("Run"), subtooltip, {}, "bell-scheduler-indicator", nullptr, KNotification::CloseOnTimeout , QStringLiteral("bellschedulernotifier"));
	       	 		//QString name = i18n("Stop now");
	        		//m_bellPlayingNotification->setDefaultAction(name);
	        		//m_bellPlayingNotification->setActions({name});
	        connect(m_bellStopNotification, QOverload<unsigned int>::of(&KNotification::activated), this, &BellSchedulerIndicator::stopBell);
			qDebug()<<"238";
		}	
		bellsInfo=variant::Variant::create_array(0);			
		m_timer_run->stop();
		changeTryIconState(1);
		is_working=false;


	}
	
	
}


BellSchedulerIndicator::TrayStatus BellSchedulerIndicator::status() const
{
    return m_status;
}



void BellSchedulerIndicator::changeTryIconState(int state){

	qDebug()<<"Cambio icono";
    const QString tooltip(i18n("Bell-Scheduler"));
    if (state==0){
    	qDebug()<<"Activando icono";
        setStatus(ActiveStatus);
        const QString subtooltip(i18n("Alarm is playing"));
        setToolTip(tooltip);
        setSubToolTip(subtooltip);
        setIconName("bellschedulernotifier");
        /*
        m_bellPlayingNotification = KNotification::event(QStringLiteral("Run"), subtooltip, {}, "bell-scheduler-indicator", nullptr, KNotification::CloseOnTimeout , QStringLiteral("bellschedulernotifier"));
        const QString name = i18n("Stop now");
        m_bellPlayingNotification->setDefaultAction(name);
        m_bellPlayingNotification->setActions({name});
        connect(m_bellPlayingNotification, QOverload<unsigned int>::of(&KNotification::activated), this, &BellSchedulerIndicator::stopBell);
		*/

    }else{
        setStatus(PassiveStatus);
    }
    


}

void BellSchedulerIndicator::stopBell(){

    qDebug()<<"Parando alarma";
}




void BellSchedulerIndicator::setStatus(BellSchedulerIndicator::TrayStatus status)
{
    
	qDebug()<<"m_status"<<m_status;
	qDebug()<<"status"<<status;

    if (m_status != status) {
    	qDebug()<<"Cambiando icono";
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