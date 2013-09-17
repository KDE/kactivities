/*
 *   Copyright (C) 2013 Ivan Cukic <ivan.cukic(at)kde.org>
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

#include "ConsumerTest.h"

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QString>
#include <QDebug>
#include <QTest>

void ConsumerTest::testOfflineActivityListing()
{
    ensureRunningJailed();

    // The service is not running
    QString nulluuid = QStringLiteral("00000000-0000-0000-0000-000000000000");

    QCOMPARE(activities.currentActivity(), nulluuid);
    QCOMPARE(activities.activities(), QStringList() << nulluuid);

    



}

void ConsumerTest::ensureRunningJailed()
{
    QStringList services =
        QDBusConnection::sessionBus().interface()->registeredServiceNames();

    foreach (const QString & service, services) {
        bool kdeServiceAndNotKAMD =
            service.startsWith(QStringLiteral("org.kde")) &&
            service != QStringLiteral("org.kde.ActivityManager");

        if (kdeServiceAndNotKAMD) {
            qFatal(
                "\n\n"
                "  *******************************************************\n"
                "  * You need to run this test in an empty dbus session! *\n"
                "  *******************************************************\n"
            );
        }
    }
}

