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
    Q_PROPERTY(QString iconName READ iconName NOTIFY iconNameChanged)
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

    QString iconName() const;
    void setIconName(const QString &name);

    void isAlive();


public slots:
    
    void worker();
    void stopBell();
  
signals:
   
    void statusChanged();
    void toolTipChanged();
    void subToolTipChanged();
    void iconNameChanged();

private:

    void initWatcher();
    void getBellInfo();
    void checkStatus();
    bool areBellsLive();
    void linkBellPid();
    void showNotification(QString notType, int index);
    void setNotificationBody(int bellId,QString action);
    void setWarningSubToolTip();

    /*QTimer *m_timer = nullptr;*/
    QTimer *m_timer_run=nullptr;
    TrayStatus m_status = PassiveStatus;
    QStringList bellsnotification;
    QString m_iconName = QStringLiteral("bellschedulernotifier");
    QString m_toolTip;
    QString m_subToolTip;
    QString notificationStartTitle;
    QString notificationEndTitle;
    QString notificationStartBody;
    QString notificationEndBody;
    QFile TARGET_FILE;
    bool is_working=false;
    bool bellToken=false;
    int checkToken=0;
    BellSchedulerIndicatorUtils* m_utils;
    QPointer<KNotification> m_bellPlayingNotification;
    int runningBells=0;
    QFileSystemWatcher *watcher = nullptr;
    QString refPath="/tmp/.BellScheduler";
     
};


#endif // PLASMA_LLIUREX_DISK_QUOTA_H
