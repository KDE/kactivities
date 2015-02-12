/*
 *   Copyright (C) 2015 Ivan Cukic <ivan.cukic(at)kde.org>
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

#include "ResultWatcherTest.h"

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QString>
#include <QDebug>
#include <QTest>
#include <QCoreApplication>
#include <QTemporaryDir>
#include <QTime>

#include <boost/range/algorithm.hpp>
#include <boost/range/numeric.hpp>
#include <boost/range/adaptor/transformed.hpp>

#include <resourceinstance.h>

#include <query.h>
#include <resultset.h>
#include <resultwatcher.h>

#include <common/database/schema/ResourcesDatabaseSchema.h>
#include <common/database/Database.h>
#include <common/test.h>

namespace KAStats = KActivities::Experimental::Stats;

ResultWatcherTest::ResultWatcherTest(QObject *parent)
    : Test(parent)
{
}

namespace {

inline void liveSleep(int seconds)
{
    qDebug() << "Sleeping for " << seconds << " seconds";
    auto start = QTime::currentTime();
    while (start.secsTo(QTime::currentTime()) < seconds) {
        QCoreApplication::processEvents();
    }
}

#define CHECK_SIGNAL_RESULT(OBJ, SIGN, SECS, TESTARGS, TESTBODY)               \
    {                                                                          \
        QObject context;                                                       \
        bool executed = false;                                                 \
                                                                               \
        QObject::connect(OBJ, SIGN, &context, [&] TESTARGS {                   \
            TESTBODY;                                                          \
            executed = true;                                                   \
            qDebug() << "Signal processed";                                    \
        });                                                                    \
                                                                               \
        qDebug() << "Waiting  for the signal at most " << SECS << " seconds";  \
        auto start = QTime::currentTime();                                     \
        while (start.secsTo(QTime::currentTime()) < SECS && !executed) {       \
            QCoreApplication::processEvents();                                 \
        }                                                                      \
        QCOMPARE(executed, true);                                              \
    }
}

void ResultWatcherTest::testLinkedResources()
{
    using namespace KAStats;
    using namespace KAStats::Terms;

    KAStats::ResultWatcher watcher(
            LinkedResources | Agent::global()
                            | Activity::any());

    watcher.linkToActivity(QUrl("test://link1"), Activity::current());

    // A signal should arrive soon, waiting for 5 seconds at most
    CHECK_SIGNAL_RESULT(&watcher, &KAStats::ResultWatcher::resultAdded, 5,
                        (const QString &uri, double),
                        QCOMPARE(QString("test://link1"), uri));

    watcher.unlinkFromActivity(QUrl("test://link1"), Activity::current());

    // A signal should arrive soon, waiting for 5 seconds at most
    CHECK_SIGNAL_RESULT(&watcher, &KAStats::ResultWatcher::resultRemoved, 5,
                        (const QString &uri),
                        QCOMPARE(QString("test://link1"), uri));
}

void ResultWatcherTest::testUsedResources()
{
    using namespace KAStats;
    using namespace KAStats::Terms;

    KAStats::ResultWatcher watcher(
            UsedResources | Agent::current()
                          | Activity::any());

    // Openning a resource for a few seconds
    {
        KActivities::ResourceInstance resource(0);
        resource.setUri(QUrl("test://test1"));

        liveSleep(3);
    }

    // A signal should arrive soon, waiting for 5 seconds at most
    CHECK_SIGNAL_RESULT(&watcher, &KAStats::ResultWatcher::resultAdded, 5,
                        (const QString &uri, double),
                        QCOMPARE(QString("test://test1"), uri));
}

void ResultWatcherTest::initTestCase()
{
}

void ResultWatcherTest::cleanupTestCase()
{
    emit testFinished();
}

