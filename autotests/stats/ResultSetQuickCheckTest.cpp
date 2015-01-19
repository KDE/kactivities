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

#include "ResultSetQuickCheckTest.h"

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QString>
#include <QDebug>
#include <QUuid>
#include <QTest>
#include <QCoreApplication>
#include <QTemporaryDir>

#include <boost/range/algorithm.hpp>
#include <boost/range/numeric.hpp>
#include <boost/range/adaptor/filtered.hpp>
#include <boost/range/algorithm/sort.hpp>

#include <query.h>
#include <resultset.h>

#include <iostream>

#include <common/database/schema/ResourcesDatabaseSchema.h>
#include <common/database/Database.h>
#include <common/test.h>

#define NUMBER_ACTIVITIES 10
#define NUMBER_AGENTS     10
#define NUMBER_RESOURCES  50
#define NUMBER_CACHES    200

namespace KAStats = KActivities::Experimental::Stats;

ResultSetQuickCheckTest::ResultSetQuickCheckTest(QObject *parent)
    : Test(parent)
    , activities(new KActivities::Consumer())
{
}

namespace {

    // Convert any range to a vector {{{
    template <typename Range>
    std::vector<typename Range::value_type> to_vector(Range &&range)
    {
        return std::vector<typename Range::value_type>(range.begin(), range.end());
    }
    /// }}}

    // {{{ helpers to perform accumulate and generate a string
    struct ItemAccumulator {
        ItemAccumulator(bool showScore)
            : showScore(showScore)
        {
        }

        QString operator() (const QString &acc, ResourceScoreCache::Item item) const
        {
            return acc + item.targettedResource
                + (showScore ? ( '(' + QString::number(item.cachedScore) + ')' ) : QString())
                + ' ';
        }

        QString operator()(const QString &acc,
                           const KAStats::ResultSet::Result &item) const
        {
            return acc + item.uri
                + (showScore ? ( '(' + QString::number(item.score) + ')' ) : QString())
                + ' ';
        }

    private:
        bool showScore;
    };

    template <typename Range>
    inline
    QString concatAll(const Range &range, bool showScore)
    {
        return boost::accumulate(range, QStringLiteral(" "),
                ItemAccumulator(showScore));
    }
    // }}}

}

// {{{ Data init
void ResultSetQuickCheckTest::initTestCase()
{
    qsrand(time(NULL));

    QTemporaryDir dir(QDir::tempPath() + "/KActivitiesStatsTest_ResultSetQuickCheckTest_XXXXXX");
    dir.setAutoRemove(false);

    if (!dir.isValid()) {
        qFatal("Can not create a temporary directory");
    }

    const auto databaseFile = dir.path() + "/database";

    Common::ResourcesDatabaseSchema::overridePath(databaseFile);
    qDebug() << "Creating database in " << databaseFile;

    // Renaming the activity1 to the current acitivty
    while (activities->serviceStatus() == KActivities::Consumer::Unknown) {
        QCoreApplication::processEvents();
    }

    generateActivitiesList();
    generateAgentList();
    generateTypesList();
    generateResourcesList();

    generateResourcesInfos();
    generateResouceScoreCaches();

    pushToDatabase();

    qDebug() << "Init finished";
}

void ResultSetQuickCheckTest::generateActivitiesList()
{
    activitiesList = activities->activities();

    while (activitiesList.size() < NUMBER_ACTIVITIES) {
        activitiesList << QUuid::createUuid().toString().mid(1, 36);
    }
}

void ResultSetQuickCheckTest::generateAgentList()
{
    for (int i = 0; i < NUMBER_AGENTS; ++i) {
        agentsList << "Agent_" + QString::number(i);
    }
}

void ResultSetQuickCheckTest::generateTypesList()
{
    typesList
        << "application/postscript"
        << "application/pdf"
        << "image/x-psd"
        << "image/x-sgi"
        << "image/x-tga"
        << "image/x-xbitmap"
        << "image/x-xwindowdump"
        << "image/x-xcf"
        << "image/x-compressed-xcf"
        << "image/tiff"
        << "image/jpeg"
        << "image/x-psp"
        << "image/png"
        << "image/x-icon"
        << "image/x-xpixmap"
        << "image/svg+xml"
        << "application/pdf"
        << "image/x-wmf"
        << "image/jp2"
        << "image/jpeg2000"
        << "image/jpx"
        << "image/x-xcursor";
}

void ResultSetQuickCheckTest::generateResourcesList()
{
    for (int i = 0; i < NUMBER_RESOURCES; ++i) {
        resourcesList << (
                QStringLiteral("/r")
                + (i < 10 ? "0" : "")
                + QString::number(i)
            );
    }
}

void ResultSetQuickCheckTest::generateResourcesInfos()
{
    foreach (const QString &resource, resourcesList) {
        ResourceInfo::Item ri;
        ri.targettedResource = resource;
        ri.title = "Title_" + QString::number(qrand() % 100);
        ri.mimetype = randItem(typesList);

        resourceInfos.insert(ri);
    }
}

void ResultSetQuickCheckTest::generateResouceScoreCaches()
{
    for (int i = 0; i < NUMBER_CACHES; ++i) {
        ResourceScoreCache::Item rsc;

        rsc.usedActivity      = randItem(activitiesList);
        rsc.initiatingAgent   = randItem(agentsList);
        rsc.targettedResource = randItem(resourcesList);

        rsc.cachedScore       = qrand() % 1000;
        rsc.firstUpdate       = qrand();
        rsc.lastUpdate        = qrand();

        resourceScoreCaches.insert(rsc);
    }
}

void ResultSetQuickCheckTest::pushToDatabase()
{
    // Creating the database, and pushing some dummy data into it
    auto database = Common::Database::instance(Common::Database::ResourcesDatabase,
                                               Common::Database::ReadWrite);

    Common::ResourcesDatabaseSchema::initSchema(*database);

    int i;

    // Inserting resource score caches
    qDebug() << "Inserting" << resourceScoreCaches.size() << "items into ResourceScoreCache";
    i = 0;

    foreach (const auto &rsc, resourceScoreCaches) {
        std::cerr << '.';

        if (++i % 10 == 0) std::cerr << i;

        database->execQuery(QStringLiteral(
            "INSERT INTO ResourceScoreCache ("
                "  usedActivity"
                ", initiatingAgent"
                ", targettedResource"
                ", scoreType"
                ", cachedScore"
                ", firstUpdate"
                ", lastUpdate"
            ") VALUES ("
                "  '%1'" // usedActivity
                ", '%2'" // initiatingAgent
                ", '%3'" // targettedResource
                ",   0 " // scoreType
                ",  %4 " // cachedScore
                ",  %5 " // firstUpdate
                ",  %6 " // lastUpdate
            ")"
        )
            .arg(rsc.usedActivity)
            .arg(rsc.initiatingAgent)
            .arg(rsc.targettedResource)
            .arg(rsc.cachedScore)
            .arg(rsc.firstUpdate)
            .arg(rsc.lastUpdate)
        );

    }
    std::cerr << std::endl;

    // Inserting resource infos
    qDebug() << "Inserting" << resourceInfos.size() << "items into ResourceInfo";
    i = 0;

    foreach (const auto &ri, resourceInfos) {
        std::cerr << '.';

        if (++i % 10 == 0) std::cerr << i;

        database->execQuery(QStringLiteral(
            "INSERT INTO ResourceInfo ("
                "  targettedResource"
                ", title"
                ", mimetype"
                ", autoTitle"
                ", autoMimetype"
            ") VALUES ("
                "  '%1' " // targettedResource
                ", '%2' " // title
                ", '%3' " // mimetype
                ",   1  " // autoTitle
                ",   1  " // autoMimetype
            ")"
        )
            .arg(ri.targettedResource)
            .arg(ri.title)
            .arg(ri.mimetype)
        );

    }
    std::cerr << std::endl;
}

void ResultSetQuickCheckTest::cleanupTestCase()
{
    emit testFinished();
}

QString ResultSetQuickCheckTest::randItem(const QStringList &choices) const
{
    return choices[qrand() % choices.size()];
}
// Data init }}}

void ResultSetQuickCheckTest::testUsedResourcesForAgents()
{
    using namespace KAStats;
    using namespace KAStats::Terms;
    using boost::sort;
    using boost::adaptors::filtered;

    TEST_CHUNK("Filtering by a specific agent, alphabetical order by URL")

    foreach (const auto &agent, agentsList) {

        auto memItems = to_vector(
                resourceScoreCaches
                | filtered(ResourceScoreCache::initiatingAgent() == agent)
            );

        // Testing when sorting by URL
        sort(memItems, ResourceScoreCache::targettedResource().asc());

        ResultSet dbItems = UsedResources
                                | Agent{agent}
                                | Activity::any()
                                | OrderAlphabetically
                                ;

        QCOMPARE(concatAll(memItems, false), concatAll(dbItems, false));

    }

    TEST_CHUNK("Filtering by a specific agent, highest scores first")

    foreach (const auto &agent, agentsList) {

        auto memItems = to_vector(
                resourceScoreCaches
                | filtered(ResourceScoreCache::initiatingAgent() == agent)
            );

        // Testing when sorting by URL
        sort(memItems, ResourceScoreCache::cachedScore().desc());

        ResultSet dbItems = UsedResources
                                | Agent{agent}
                                | Activity::any()
                                | HighScoredFirst
                                ;

        QCOMPARE(concatAll(memItems, true), concatAll(dbItems, true));

    }
}

void ResultSetQuickCheckTest::testUsedResourcesForActivities()
{
}

// vim: set foldmethod=marker:
