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
#ifndef PLASMA_BELL_SCHEDULER_INDICATOR_H
#define PLASMA_BELL_SCHEDULER_INDICATOR_H

#include <QObject>
#include <QProcess>
#include <QPointer>
#include <KNotification>
#include <QDir>
#include <QFile>
#include <QThread>

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
    void getBellInfo();
    void checkStatus();
    void stopBell();

signals:
   
    void statusChanged();
    void toolTipChanged();
    void subToolTipChanged();
    void iconNameChanged();

private:

    bool areBellsLive();
    void linkBellPid();
    void showNotification(QString notType, int index);

    QTimer *m_timer = nullptr;
    QTimer *m_timer_run=nullptr;
    TrayStatus m_status = PassiveStatus;
    QStringList bellsnotification;
    QString m_iconName = QStringLiteral("bellschedulernotifier");
    QString m_toolTip;
    QString m_subToolTip;
    QFile TARGET_FILE;
    bool is_working=false;
    bool bellToken=false;
    int checkToken=0;
    BellSchedulerIndicatorUtils* m_utils;
    QPointer<KNotification> m_bellPlayingNotification;
     
};


#endif // PLASMA_LLIUREX_DISK_QUOTA_H
