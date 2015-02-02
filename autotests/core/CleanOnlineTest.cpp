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

#include "CleanOnlineTest.h"

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QString>
#include <QDebug>
#include <QTest>

QString CleanOnlineSetup::id1;
QString CleanOnlineSetup::id2;

CleanOnlineTest::CleanOnlineTest(QObject *parent)
    : Test(parent)
{
    activities.reset(new KActivities::Controller());
}

CleanOnlineSetup::CleanOnlineSetup(QObject *parent)
    : Test(parent)
{
    activities.reset(new KActivities::Controller());
}

OnlineTest::OnlineTest(QObject *parent)
    : Test(parent)
{
    activities.reset(new KActivities::Controller());
}

void CleanOnlineTest::testCleanOnlineActivityListing()
{
    // Waiting for the service to start, and for us to sync
    TEST_WAIT_UNTIL(activities->serviceStatus()
                    == KActivities::Consumer::Running);

    QCOMPARE(activities->activities(), QStringList());
    QCOMPARE(activities->serviceStatus(), KActivities::Consumer::Running);
    QCOMPARE(activities->currentActivity(), QString());

    QCOMPARE(activities->activities(), QStringList());
    QCOMPARE(activities->activities(KActivities::Info::Running), QStringList());
    QCOMPARE(activities->activities(KActivities::Info::Stopped), QStringList());
}

void CleanOnlineSetup::testCleanOnlineActivityControl()
{
    // Waiting for the service to start, and for us to sync
    TEST_WAIT_UNTIL(activities->serviceStatus()
                    == KActivities::Consumer::Running);

    auto activity1 = activities->addActivity(QStringLiteral("The first one"));
    TEST_WAIT_UNTIL(activity1.isFinished());
    id1 = activity1.result();

    auto activity2 = activities->addActivity(QStringLiteral("The second one"));
    TEST_WAIT_UNTIL(activity2.isFinished());
    id2 = activity2.result();

    TEST_WAIT_UNTIL(activities->currentActivity() == id1);

    auto f1 = activities->setCurrentActivity(activity2.result());
    TEST_WAIT_UNTIL(f1.isFinished());
    TEST_WAIT_UNTIL(activities->currentActivity() == id2);

    auto f2 = activities->setCurrentActivity(id1);
    TEST_WAIT_UNTIL(f2.isFinished());
    TEST_WAIT_UNTIL(activities->currentActivity() == id1);

    auto f3 = activities->setActivityName(id1, QStringLiteral("Renamed activity"));
    TEST_WAIT_UNTIL(f3.isFinished());

    KActivities::Info ac(id1);
    TEST_WAIT_UNTIL(ac.name() == QStringLiteral("Renamed activity"));

    TEST_WAIT_UNTIL(activities->activities().size() == 2);
}

void OnlineTest::testOnlineActivityListing()
{
    // Waiting for the service to start, and for us to sync
    TEST_WAIT_UNTIL(activities->serviceStatus()
                    == KActivities::Consumer::Running);

    KActivities::Info i1(CleanOnlineSetup::id1);
    KActivities::Info i2(CleanOnlineSetup::id2);

    TEST_WAIT_UNTIL(i1.name() == QStringLiteral("Renamed activity"));
    TEST_WAIT_UNTIL(i2.name() == QStringLiteral("The second one"));

    qDebug() << CleanOnlineSetup::id1 << i1.name();
    qDebug() << CleanOnlineSetup::id2 << i2.name();
}

void CleanOnlineTest::cleanupTestCase()
{
    emit testFinished();
}

void CleanOnlineSetup::cleanupTestCase()
{
    emit testFinished();
}

void OnlineTest::cleanupTestCase()
{
    emit testFinished();
}
