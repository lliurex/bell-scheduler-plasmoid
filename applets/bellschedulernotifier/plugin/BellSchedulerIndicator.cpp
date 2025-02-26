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
    , m_timer_run(new QTimer(this))
    , m_utils(new BellSchedulerIndicatorUtils(this))
    
{

    QString titleInitHead=i18n("No bell was played");
    setSubToolTip(titleInitHead);
    setPlaceHolderText(titleInitHead);
    TARGET_FILE.setFileName(tokenPath);
    initWatcher();

}    

void BellSchedulerIndicator::initWatcher(){

    QDir TARGET_DIR(refPath);
    bool initWorker=false;

	if (!TARGET_DIR.exists()){
		QDir basePath("/tmp/");
		basePath.mkdir(".BellScheduler");
	}else{
		initWorker=true;
	}
	watcher=new QFileSystemWatcher(this);
	connect(watcher,SIGNAL(directoryChanged(QString)),this,SLOT(worker()));
	watcher->addPath(refPath);
	if (initWorker){
		worker();
	}

}

void BellSchedulerIndicator::worker(){

    if (BellSchedulerIndicator::TARGET_FILE.exists() ) {
        connect(watcher,SIGNAL(fileChanged(QString)),this,SLOT(tokenChanged()));
        watcher->addPath(tokenPath);
        if (!is_working){
            getBellInfo();
            isAlive();
        }
    }        
     
}

void BellSchedulerIndicator::tokenChanged(){

	if (BellSchedulerIndicator::TARGET_FILE.exists()){
		getBellInfo();
		changeTryIconState(0);	
	}
}    

void BellSchedulerIndicator::showNotification(QString notType,int index){

	if (notType=="start"){
		titleStartHead=i18n("Playing the bell");
		notificationStartTitle=titleStartHead+":";
		setNotificationBody(index,"start");
		m_bellPlayingNotification = KNotification::event(QStringLiteral("Run"),notificationStartTitle,notificationStartBody,"bell-scheduler-indicator", nullptr, KNotification::CloseOnTimeout , QStringLiteral("bellschedulernotifier"));
	    QString name = i18n("Stop now");
	    m_bellPlayingNotification->setDefaultAction(name);
	    m_bellPlayingNotification->setActions({name});
	    connect(m_bellPlayingNotification, QOverload<unsigned int>::of(&KNotification::activated), this, &BellSchedulerIndicator::stopBell);
	}else{
		notificationEndTitle=i18n("The bell has ended:");
		setNotificationBody(index,"end");
		m_bellPlayingNotification = KNotification::event(QStringLiteral("Run"),notificationEndTitle,notificationEndBody, "bell-scheduler-indicator", nullptr, KNotification::CloseOnTimeout , QStringLiteral("bellschedulernotifier"));
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
					setNotificationBody(i,"start");
					setSubToolTip(notificationStartTitle+"\n"+notificationStartBody);
					setPlaceHolderText(titleStartHead);
					setPlaceHolderExplanation(placeHolderExplanationStart);
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
		if (runningBells>1){
   	        setIconName("bellschedulernotifier-error");
        	setWarningSubToolTip();
        }else{
        	setIconName("bellschedulernotifier");
        }

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
        setCanStopBell(true);
        if (runningBells>1){
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
        QString titleEndHead=i18n("Last bell played");
        QString warningEndExplanation=i18n("WARNING: 2 or more bells have played simultaneously");
        notificationStartTitle=titleEndHead+":";
        if (multipleBellsPlayed){
        	setSubToolTip(notificationStartTitle+"\n"+warningEndExplanation);
        	setPlaceHolderExplanation(warningEndExplanation);
        }else{
        	setSubToolTip(notificationStartTitle+"\n"+notificationStartBody);
        }
        setPlaceHolderText(titleEndHead);
    }

}

void BellSchedulerIndicator::setNotificationBody(int bellId,QString action){

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
	if (action=="start"){
		notificationStartBody="- "+hour+ " "+bell+"\n- "+duration_label+duration;
		placeHolderExplanationStart=hour+ " "+bell+"\n"+duration_label+duration;
	}else{
		notificationEndBody="- "+hour+ " "+bell+"\n- "+duration_label+duration;
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
