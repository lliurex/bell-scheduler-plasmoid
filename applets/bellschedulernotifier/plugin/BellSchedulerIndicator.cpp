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

using namespace edupals;
using namespace std;

BellSchedulerIndicator::BellSchedulerIndicator(QObject *parent)
    : QObject(parent)
    , m_timer_run(new QTimer(this))
    , m_utils(new BellSchedulerIndicatorUtils(this))
    
{

    TARGET_FILE.setFile(m_utils->tokenPath);
    connect(m_utils,&BellSchedulerIndicatorUtils::startWidgetFinished,this,&BellSchedulerIndicator::handleStartFinished);
    connect(m_utils,&BellSchedulerIndicatorUtils::readBellTokenFinished,this,&BellSchedulerIndicator::handleBellTokenFinished);
    connect(m_utils,&BellSchedulerIndicatorUtils::getRunningBellsFinished,this,&BellSchedulerIndicator::handleGetRunningBellsFinished);
    connect(m_utils,&BellSchedulerIndicatorUtils::requestCloseNotification,this,&BellSchedulerIndicator::showNotification);
    connect(m_utils,&BellSchedulerIndicatorUtils::stopBellFinished,this,&BellSchedulerIndicator::handleStopBellFinished);
    connect(m_timer_run, &QTimer::timeout, this, &BellSchedulerIndicator::checkRunningBells);

    QTimer::singleShot(0,this,[this](){
    	m_utils->startWidget();
    });
    
} 

void BellSchedulerIndicator::handleStartFinished(bool startOk, bool initWorker){

    if (startOk){
    	titleStartHead=i18n("No bell was played");
    	setSubToolTip(titleStartHead);
    	setPlaceHolderText(titleStartHead);
	    QDir TARGET_DIR(m_utils->refPath);
	    if (!watcher) {
	    	watcher=new QFileSystemWatcher(this);
	    	connect(watcher,&QFileSystemWatcher::directoryChanged,this,&BellSchedulerIndicator::worker);
	    	connect(watcher,&QFileSystemWatcher::fileChanged,this,&BellSchedulerIndicator::tokenChanged,Qt::UniqueConnection);
	    }
	    if (!watcher->directories().contains(m_utils->refPath)){
	    	watcher->addPath(m_utils->refPath);
	    }
	    if (initWorker){
	    	worker();
	    }
	}else{
		setStatus(HiddenStatus);
	}

}

void BellSchedulerIndicator::worker(){

    TARGET_FILE.refresh();
    if (TARGET_FILE.exists()) {
    	if (!watcher->files().contains(TARGET_FILE.absoluteFilePath())){
    		watcher->addPath(TARGET_FILE.absoluteFilePath());
    	}
        if (!isWorking){
            getBellInfo();
        }
    }    
     
}

void BellSchedulerIndicator::tokenChanged(){

	TARGET_FILE.refresh();
	if (TARGET_FILE.exists()){
		if (!isWorking){
			getBellInfo();
		}
	}
}    

void BellSchedulerIndicator::showNotification(QString notType,QString bellId){

	if (notType=="start"){
		titleStartHead=i18n("Playing the bell:");
		notificationStartTitle=titleStartHead;
		setNotificationBody(bellId,"start");
		m_bellPlayingNotification = KNotification::event(QStringLiteral("Run"),notificationStartTitle,notificationStartBody,"bell-scheduler-indicator", nullptr, KNotification::CloseOnTimeout , QStringLiteral("bellschedulernotifier"));
	    QString name = i18n("Stop now");
	    m_bellPlayingNotification->setDefaultAction(name);
	    m_bellPlayingNotification->setActions({name});
	    connect(m_bellPlayingNotification, QOverload<unsigned int>::of(&KNotification::activated), this, &BellSchedulerIndicator::stopBell);
	}else{
		notificationEndTitle=i18n("The bell has ended:");
		setNotificationBody(bellId,"end");
		m_bellPlayingNotification = KNotification::event(QStringLiteral("Run"),notificationEndTitle,notificationEndBody, "bell-scheduler-indicator", nullptr, KNotification::CloseOnTimeout , QStringLiteral("bellschedulernotifier"));
	}    

}

void BellSchedulerIndicator::getBellInfo(){

   	isWorking=true;
   	m_utils->readBellToken();
}

void BellSchedulerIndicator::handleBellTokenFinished(){

    QStringList keys = m_utils->bellsInfo.keys();
    for (const QString &bellId : keys) {
        if (!bellId.isEmpty()) {
            if (!bellsnotification.contains(bellId)){
   				bellsnotification.append(bellId);
   				showNotification("start",bellId);
  				runningBells=runningBells+1;
  			}
        }
    }
    isWorking=false;
    if (m_utils->bellsInfo.size()>0){
   		changeTryIconState(0);
   		if (!isAliveWorking){
   			isAliveWorking=true;
   			isAlive();
   		}
    }
}

void BellSchedulerIndicator::isAlive(){

    qDebug()<<"[BELL-SCHEDULER-INDICATOR]: Checking for running bells";

	bellToken=false;
	changeTryIconState(0);
	m_timer_run->start(1000);
	checkRunningBells();

}

void BellSchedulerIndicator::checkRunningBells(){
	
	m_utils->getRunningBells();
	
}

void BellSchedulerIndicator::handleGetRunningBellsFinished(QList<QJsonObject> pidInfo, QStringList bellsPid){

	TARGET_FILE.refresh();
	if (TARGET_FILE.exists() ) { 
		if (!bellToken){
			bellToken=true;
		}
	}else{
		bool areBellsLive=!bellsPid.isEmpty();
		if (areBellsLive){
			bellToken=true;
		}else{
			bellToken=false;
		}
	}

	if (bellToken){
		runningBells=bellsPid.count();
		if (runningBells>1){
   	        setIconName("bellschedulernotifier-error");
        	setWarningSubToolTip();
        }else{
        	if (multipleBellsPlayed){
        		setIconName("bellschedulernotifier-error");
        	}else{
        		setIconName("bellschedulernotifier");
        	}
        }
        m_utils->syncBellInfo(pidInfo,bellsPid);

    }else{
    	QStringList keys = m_utils->bellsInfo.keys();
		for (const QString &bellId : keys) {
			showNotification("end",bellId);
		}
		m_utils->bellsInfo.clear();			
		m_timer_run->stop();
		changeTryIconState(1);
		isWorking=false;
		isAliveWorking=false;
		QStringList emptyList;
		bellsnotification=emptyList;
		runningBells=0;
		multipleBellsPlayed=false;
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
        setCanStopBell(true);
        if (multipleBellsPlayed){
        	setIconName("bellschedulernotifier-error");
        	setWarningSubToolTip();
        }else{
   	        setIconName("bellschedulernotifier");
        	setSubToolTip(notificationStartTitle+"\n"+notificationStartBody);
        	setPlaceHolderText(titleStartHead);
        	setPlaceHolderExplanation(placeHolderExplanationStart);

        }
    }else{
        setStatus(PassiveStatus);
        setCanStopBell(false);
        QString titleEndHead=i18n("Last bell played:");
        QString warningEndExplanation=i18n("WARNING: 2 or more bells have played simultaneously");
        notificationStartTitle=titleEndHead;
        if (multipleBellsPlayed){
        	setSubToolTip(notificationStartTitle+"\n"+warningEndExplanation);
        	setPlaceHolderExplanation(warningEndExplanation);
        }else{
        	setSubToolTip(notificationStartTitle+"\n"+notificationStartBody);
        }
        setPlaceHolderText(titleEndHead);
    }

}

void BellSchedulerIndicator::setNotificationBody(QString bellId,QString action){

	QString hour;
	QString bell;
	QString bellLabel=i18n("Name: ");
	QString duration;
	QString durationLabel=i18n("Duration: ");

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
	if (action=="start"){
		notificationStartBody="- "+bellLabel+hour+ " -  "+bell+"\n- "+durationLabel+duration;
		placeHolderExplanationStart=bellLabel+hour+ " -  "+bell+"\n"+durationLabel+duration;
	}else{
		notificationEndBody="- "+bellLabel+hour+ " "+bell+"\n- "+durationLabel+duration;
	}
}

void BellSchedulerIndicator::setWarningSubToolTip(){

	QString warning=i18n("WARNING: 2 or more bells are played simultaneously");
	setSubToolTip(notificationStartTitle+"\n"+warning);
	setPlaceHolderText(titleStartHead);
	setPlaceHolderExplanation(warning);
	multipleBellsPlayed=true;

}

void BellSchedulerIndicator::stopBell(){

    if (!stopBellLaunched){
    	stopBellLaunched=true;
    	m_utils->stopBell();
    }
}

void BellSchedulerIndicator::handleStopBellFinished(){

	stopBellLaunched=false;
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

QString BellSchedulerIndicator::placeHolderText() const
{
    return m_placeHolderText;
}

void BellSchedulerIndicator::setPlaceHolderText(const QString &placeHolderText)
{
    if (m_placeHolderText != placeHolderText) {
        m_placeHolderText = placeHolderText;
        emit placeHolderTextChanged();
    }
}

QString BellSchedulerIndicator::placeHolderExplanation() const
{
    return m_placeHolderExplanation;
}

void BellSchedulerIndicator::setPlaceHolderExplanation(const QString &placeHolderExplanation)
{
    if (m_placeHolderExplanation != placeHolderExplanation) {
        m_placeHolderExplanation = placeHolderExplanation;
        emit placeHolderExplanationChanged();
    }
}

bool BellSchedulerIndicator::canStopBell(){

	return m_canStopBell;
}

void BellSchedulerIndicator::setCanStopBell(bool canStopBell){

	if (m_canStopBell!=canStopBell){
		m_canStopBell=canStopBell;
		emit canStopBellChanged();	
	}
}
