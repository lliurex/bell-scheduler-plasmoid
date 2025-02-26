#ifndef PLASMA_BELL_SCHEDULER_INDICATOR_H
#define PLASMA_BELL_SCHEDULER_INDICATOR_H

#include <QObject>
#include <QProcess>
#include <QPointer>
#include <KNotification>
#include <QDir>
#include <QFile>
#include <QThread>
#include <QFileSystemWatcher>

#include <variant.hpp>

#include "BellSchedulerIndicatorUtils.h"

class QTimer;
class KNotification;
class AsyncDbus;


class BellSchedulerIndicator : public QObject
{
    Q_OBJECT


    Q_PROPERTY(TrayStatus status READ status NOTIFY statusChanged)
    Q_PROPERTY(QString toolTip READ toolTip NOTIFY toolTipChanged)
    Q_PROPERTY(QString subToolTip READ subToolTip NOTIFY subToolTipChanged)
    Q_PROPERTY(QString placeHolderText READ placeHolderText NOTIFY placeHolderTextChanged)
    Q_PROPERTY(QString placeHolderExplanation READ placeHolderExplanation NOTIFY placeHolderExplanationChanged)
    Q_PROPERTY(QString iconName READ iconName NOTIFY iconNameChanged)
    Q_PROPERTY (bool canStopBell READ canStopBell NOTIFY canStopBellChanged)
    Q_ENUMS(TrayStatus)

public:
    /**
     * System tray icon states.
     */
    enum TrayStatus {
        ActiveStatus=0,
        PassiveStatus
    };

    BellSchedulerIndicator(QObject *parent = nullptr);

    TrayStatus status() const;
    void changeTryIconState (int state);
    void setStatus(TrayStatus status);

    QString toolTip() const;
    void setToolTip(const QString &toolTip);

    QString subToolTip() const;
    void setSubToolTip(const QString &subToolTip);

    QString placeHolderText() const;
    void setPlaceHolderText(const QString &placeHolderText);

    QString placeHolderExplanation() const;
    void setPlaceHolderExplanation(const QString &placeHolderExplanation);

    QString iconName() const;
    void setIconName(const QString &name);

    bool canStopBell();
    void setCanStopBell(bool);

    void isAlive();


public slots:
    
    void worker();
    void stopBell();
    void tokenChanged();
  
signals:
   
    void statusChanged();
    void toolTipChanged();
    void subToolTipChanged();
    void placeHolderTextChanged();
    void placeHolderExplanationChanged();
    void iconNameChanged();
    void canStopBellChanged();

private:

    void initWatcher();
    void getBellInfo();
    void checkStatus();
    bool areBellsLive();
    void linkBellPid();
    void showNotification(QString notType, int index);
    void setNotificationBody(int bellId,QString action);
    void setWarningSubToolTip();

    QTimer *m_timer_run=nullptr;
    TrayStatus m_status = PassiveStatus;
    QStringList bellsnotification;
    QString m_iconName = QStringLiteral("bellschedulernotifier");
    QString m_toolTip;
    QString m_subToolTip;
    QString m_placeHolderText;
    QString m_placeHolderExplanation;
    bool m_canStopBell=false;
    QString titleStartHead;
    QString notificationStartTitle;
    QString notificationEndTitle;
    QString notificationStartBody;
    QString placeHolderExplanationStart;
    QString notificationEndBody;
    QFile TARGET_FILE;
    bool is_working=false;
    bool bellToken=false;
    BellSchedulerIndicatorUtils* m_utils;
    QPointer<KNotification> m_bellPlayingNotification;
    int runningBells=0;
    bool multipleBellsPlayed=false;
    QFileSystemWatcher *watcher = nullptr;
    QString refPath="/tmp/.BellScheduler";
    QString tokenPath="/tmp/.BellScheduler/bellscheduler-token";
     
};


#endif // PLASMA_LLIUREX_DISK_QUOTA_H
