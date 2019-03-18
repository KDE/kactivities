/*
 *   Copyright (C) 2013, 2014, 2015 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "test.h"

#include <QDBusConnection>
#include <QDBusConnectionInterface>

#include "common/dbus/common.h"

Test::Test(QObject *parent)
    : QObject(parent)
{

}

bool Test::inEmptySession()
{
    const QStringList services =
        QDBusConnection::sessionBus().interface()->registeredServiceNames();

    for (const QString & service : services) {
        bool kdeServiceAndNotKAMD =
            service.startsWith(QStringLiteral("org.kde")) &&
            service != KAMD_DBUS_SERVICE;

        if (kdeServiceAndNotKAMD) {
            return false;
        }
    }

    return true;
}

bool Test::isActivityManagerRunning()
{
    return QDBusConnection::sessionBus().interface()->isServiceRegistered(
        KAMD_DBUS_SERVICE);
}

