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
#ifndef PLASMA_BELL_SCHEDULER_INDICATOR_UTILS_H
#define PLASMA_BELL_SCHEDULER_INDICATOR_UTILS_H

#include <QObject>
#include <QProcess>
#include <QFile>

#include <n4d.hpp>
#include <variant.hpp>

using namespace std;
using namespace edupals;


class BellSchedulerIndicatorUtils : public QObject
{
    Q_OBJECT


public:
   

   BellSchedulerIndicatorUtils(QObject *parent = nullptr);

   void getBellInfo();
   void linkBellPid();
   void stopBell();
   std::tuple<bool, QStringList> areBellsLive();
   bool isTokenUpdated();

   QStringList bellsId;
   variant::Variant bellsInfo =variant::Variant::create_array(0);
   //bool areBellsLive();
   bool tokenUpdated=false;

private:    
     
	 variant::Variant readToken();
    std::tuple<QList<QJsonObject>, QStringList> getBellPid();	
    string  getFormatHour(int hour,int minute);
    n4d::Client client;
    QFile BELLS_TOKEN;
    qint64 MOD_FRECUENCY=2000;
    
     
};



#endif // PLASMA_LLIUREX_UP_INDICATOR_UTILS_H
