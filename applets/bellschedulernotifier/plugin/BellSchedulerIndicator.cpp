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
#include <QFileSystemWatcher>
#include <QThread>
#include <QtCore/QStringList>
#include <QJsonObject>

#include <variant.hpp>
#include <json.hpp>
#include <QDebug>

using namespace edupals;
using namespace std;



BellSchedulerIndicator::BellSchedulerIndicator(QObject *parent)
    : QObject(parent)
    /*, m_timer(new QTimer(this))*/
    , m_timer_run(new QTimer(this))
    , m_utils(new BellSchedulerIndicatorUtils(this))
    
{

    TARGET_FILE.setFileName("/tmp/.BellScheduler/bellscheduler-token");
 
    /*
    connect(m_timer, &QTimer::timeout, this, &BellSchedulerIndicator::worker);
    m_timer->start(2000);
    worker();
    */
    initWatcher();

}    

void BellSchedulerIndicator::initWatcher(){

    QDir TARGET_DIR(refPath);

	if (!TARGET_DIR.exists()){
		QDir basePath("/tmp/");
		basePath.mkdir(".BellScheduler");
	}else{
		worker();
	}
	watcher=new QFileSystemWatcher(this);
    connect(watcher,SIGNAL(directoryChanged(QString)),this,SLOT(worker()));
	watcher->addPath(refPath);

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

	
	if (notType=="start"){
		notificationStartTitle=i18n("Playing the bell:");
		setNotificationBody(index);
		m_bellPlayingNotification = KNotification::event(QStringLiteral("Run"),notificationStartTitle,notificationBody,"bell-scheduler-indicator", nullptr, KNotification::CloseOnTimeout , QStringLiteral("bellschedulernotifier"));
	    QString name = i18n("Stop now");
	    m_bellPlayingNotification->setDefaultAction(name);
	    m_bellPlayingNotification->setActions({name});
	    connect(m_bellPlayingNotification, QOverload<unsigned int>::of(&KNotification::activated), this, &BellSchedulerIndicator::stopBell);
	}else{
		notificationEndTitle=i18n("The bell has ended:");
		setNotificationBody(index);
		m_bellPlayingNotification = KNotification::event(QStringLiteral("Run"),notificationEndTitle,notificationBody, "bell-scheduler-indicator", nullptr, KNotification::CloseOnTimeout , QStringLiteral("bellschedulernotifier"));
	}    

}

void BellSchedulerIndicator::getBellInfo(){

    is_working=true;
    m_utils->getBellInfo();

    for (int i=0;i<m_utils->bellsInfo.count();i++){
    	if (!bellsnotification.contains(QString::fromStdString(m_utils->bellsInfo[i]["bellId"]))){
    		bellsnotification.append(QString::fromStdString(m_utils->bellsInfo[i]["bellId"]));
 	  		showNotification("start",i);
 	  		runningBells=runningBells+1;
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
					runningBells=runningBells-1;
				}else{
					tmpList.append(m_utils->bellsInfo[i]);
					setNotificationBody(i);
					setSubToolTip(notificationStartTitle+"\n"+notificationBody);
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
		if (runningBells>1){
   	        setIconName("bellschedulernotifier-error");
        	setWarningSubToolTip();
        }else{
        	setIconName("bellschedulernotifier");
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
		runningBells=0;
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
        if (runningBells>1){
        	setIconName("bellschedulernotifier-error");
        	setWarningSubToolTip();
        }else{
   	        setIconName("bellschedulernotifier");
        	setSubToolTip(notificationStartTitle+"\n"+notificationBody);
  	
        }
    }else{
        setStatus(PassiveStatus);
    }

}

void BellSchedulerIndicator::setNotificationBody(int bellId){

	QString hour;
	QString bell;
	QString duration;
	QString duration_label=i18n("Duration: ");

	auto bellData=m_utils->getBellData(bellId);

	duration=bellData[2];
	if (duration=="error"){
		duration=i18n("Error. Not Available");
		hour=duration;
		bell="";
	}else{
		hour=bellData[0];
		bell=bellData[1];
		if (duration=="full"){
			duration=i18n("Full reproduction");
		}else{
			QString label=i18n(" seconds");
			duration=duration+label;
		}
	}
	notificationBody="- "+hour+ " "+bell+"\n- "+duration_label+duration;

}

void BellSchedulerIndicator::setWarningSubToolTip(){

	QString warning=i18n("WARNING: 2 or more bells are played simultaneously");
	setSubToolTip(notificationStartTitle+"\n"+warning);
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
