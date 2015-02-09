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

#include "ResultSetTest.h"

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QString>
#include <QDebug>
#include <QTest>
#include <QCoreApplication>
#include <QTemporaryDir>

#include <boost/range/algorithm.hpp>
#include <boost/range/numeric.hpp>
#include <boost/range/adaptor/transformed.hpp>

#include <query.h>
#include <resultset.h>

#include <common/database/schema/ResourcesDatabaseSchema.h>
#include <common/database/Database.h>
#include <common/test.h>

namespace KAStats = KActivities::Experimental::Stats;

ResultSetTest::ResultSetTest(QObject *parent)
    : Test(parent)
{
}

namespace {
    QString getBarredUri(const KAStats::ResultSet::Result &result)
    {
        return result.resource + "|";
    }

    QString concatenateResults(const KAStats::ResultSet &results)
    {
        using boost::accumulate;
        using boost::adaptors::transformed;

        return accumulate(
                results | transformed(getBarredUri),
                QStringLiteral("|")
            );
    }
}

void ResultSetTest::testLinkedResources()
{
    using namespace KAStats;
    using namespace KAStats::Terms;

    // TEST_CHUNK("Getting the linked resources alphabetically")
    // {
    //     ResultSet result(LinkedResources
    //                         | Agent { "gvim" }
    //                         | Activity { "860d9ec8-87f9-8e96-1558-1faf54b98e97" }
    //                         | OrderAlphabetically
    //                     );
    //
    //     QCOMPARE(result.at(0).resource, QStringLiteral("/path/mid1_a1"));
    //     QCOMPARE(result.at(1).resource, QStringLiteral("/path/mid2_a1"));
    // }

}

void ResultSetTest::testUsedResources()
{
    using namespace KAStats;
    using namespace KAStats::Terms;

    qDebug() << "Agent: " << QCoreApplication::instance()->applicationName();

    TEST_CHUNK("Getting the used resources by the highest score, default query")
    {
        ResultSet result(UsedResources);

        qDebug() << "-----------------------------";
        for (const auto &item: result) {
            qDebug() << "Item: " << item.resource;
        }
        qDebug() << "-----------------------------";

        QCOMPARE(result.at(0).resource, QStringLiteral("/path/high5_act1_kast"));
        QCOMPARE(result.at(1).resource, QStringLiteral("/path/high7_act1_kast"));
        QCOMPARE(result.at(2).resource, QStringLiteral("/path/high8_act1_kast"));

        // END!
        QCOMPARE(result.at(3).resource, QString());

        // Testing whether range works
        QCOMPARE(QStringLiteral("|/path/high5_act1_kast|/path/high7_act1_kast|/path/high8_act1_kast|"),
                 concatenateResults(result));
    }

    TEST_CHUNK("Getting the used resources by the highest score, gvim")
    {
        ResultSet result(UsedResources
                        | HighScoredFirst
                        | Agent{"gvim"}
                        );

        QCOMPARE(result.at(0).resource, QStringLiteral("/path/high1_act1_gvim"));
        QCOMPARE(result.at(1).resource, QStringLiteral("/path/high4_act1_gvim"));
    }

    TEST_CHUNK("Getting the used resources by the highest score, global agent")
    {
        ResultSet result(UsedResources
                        | HighScoredFirst
                        | Agent::global()
                        );

        QCOMPARE(result.at(0).resource, QStringLiteral("/path/mid6_act1_glob"));
        QCOMPARE(result.at(1).resource, QStringLiteral("/path/mid7_act1_glob"));
        QCOMPARE(result.at(2).resource, QStringLiteral("/path/mid8_act1_glob"));
    }

    TEST_CHUNK("Getting the used resources by the highest score, any agent")
    {
        ResultSet result(UsedResources
                        | HighScoredFirst
                        | Agent::any()
                        | Activity::any()
                        );

        QCOMPARE(result.at(0).resource, QStringLiteral("/path/high1_act1_gvim"));
        QCOMPARE(result.at(1).resource, QStringLiteral("/path/high2_act2_kate"));
        QCOMPARE(result.at(2).resource, QStringLiteral("/path/high3_act1_kate"));
    }
}

void ResultSetTest::initTestCase()
{
    QTemporaryDir dir(QDir::tempPath() + "/KActivitiesStatsTest_ResultSetTest_XXXXXX");
    dir.setAutoRemove(false);

    if (!dir.isValid()) {
        qFatal("Can not create a temporary directory");
    }

    const auto databaseFile = dir.path() + "/database";

    Common::ResourcesDatabaseSchema::overridePath(databaseFile);
    qDebug() << "Creating database in " << databaseFile;

    // Creating the database, and pushing some dummy data into it
    auto database = Common::Database::instance(Common::Database::ResourcesDatabase,
                                               Common::Database::ReadWrite);

    Common::ResourcesDatabaseSchema::initSchema(*database);

    database->execQuery(
            "INSERT INTO ResourceScoreCache (usedActivity, initiatingAgent, targettedResource, scoreType, cachedScore, firstUpdate, lastUpdate) VALUES "

            "   ('activity1' , 'gvim'                 , '/path/high1_act1_gvim' , '0' , '800' , '-1' , '1421446599')"
            " , ('activity2' , 'kate'                 , '/path/high2_act2_kate' , '0' , '700' , '-1' , '1421439442')"
            " , ('activity1' , 'kate'                 , '/path/high3_act1_kate' , '0' , '600' , '-1' , '1421439442')"
            " , ('activity1' , 'gvim'                 , '/path/high4_act1_gvim' , '0' , '500' , '-1' , '1421446488')"
            " , ('activity1' , 'KActivitiesStatsTest' , '/path/high5_act1_kast' , '0' , '400' , '-1' , '1421446599')"
            " , ('activity2' , 'KActivitiesStatsTest' , '/path/high6_act2_kast' , '0' , '300' , '-1' , '1421439442')"
            " , ('activity1' , 'KActivitiesStatsTest' , '/path/high7_act1_kast' , '0' , '200' , '-1' , '1421439442')"
            " , ('activity1' , 'KActivitiesStatsTest' , '/path/high8_act1_kast' , '0' , '100' , '-1' , '1421446488')"

            " , ('activity1' , 'gvim'                 , '/path/mid1_act1_gvim'  , '0' , '17'  , '-1' , '1421433419')"
            " , ('activity1' , 'gvim'                 , '/path/mid2_act1_gvim'  , '0' , '54'  , '-1' , '1421431630')"
            " , ('activity2' , 'gvim'                 , '/path/mid3_act2_gvim'  , '0' , '8'   , '-1' , '1421433172')"
            " , ('activity2' , 'gvim'                 , '/path/mid4_act2_gvim'  , '0' , '8'   , '-1' , '1421432545')"
            " , ('activity2' , 'gvim'                 , '/path/mid5_act2_gvim'  , '0' , '79'  , '-1' , '1421439118')"
            " , ('activity1' , ':global'              , '/path/mid6_act1_glob'  , '0' , '20'  , '-1' , '1421439331')"
            " , ('activity1' , ':global'              , '/path/mid7_act1_glob'  , '0' , '8'   , '-1' , '0')"
            " , ('activity1' , ':global'              , '/path/mid8_act1_glob'  , '0' , '7'   , '-1' , '1421432617')"

            " , ('activity1' , 'gvim'                 , '/path/low3_act1_gvim'  , '0' , '6'   , '-1' , '1421434704')"
            " , ('activity1' , 'kate'                 , '/path/low2_act1_kate'  , '0' , '3'   , '-1' , '1421433266')"
            " , ('activity1' , 'kate'                 , '/path/low1_act1_kate'  , '0' , '2'   , '-1' , '1421433254')"

        );

    // Renaming the activity1 to the current acitivty
    KActivities::Consumer kamd;

    while (kamd.serviceStatus() == KActivities::Consumer::Unknown) {
        QCoreApplication::processEvents();
    }

    database->execQuery(
            "UPDATE ResourceScoreCache SET usedActivity = '"
                + kamd.currentActivity()
                + "' WHERE usedActivity = 'activity1'");


    database->execQuery(
            "INSERT INTO ResourceLink (usedActivity, initiatingAgent, targettedResource) VALUES "

              "('activity1' , 'gvim' , '/path/mid1_a1')"
            ", ('activity1' , 'gvim' , '/path/mid2_a1')"
            ", ('activity2' , 'gvim' , '/path/mid3_a2')"
            ", ('activity2' , 'gvim' , '/path/mid4_a2')"
            ", ('activity2' , 'gvim' , '/path/link5_a2')"
            ", ('activity1' , 'kate' , '/path/link6_a1')"
            ", ('activity1' , 'kate' , '/path/link7_a1')"
            ", ('activity1' , 'kate' , '/path/link8_a1')"

        );
}

void ResultSetTest::cleanupTestCase()
{
    emit testFinished();
}

