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

#include <utils/qsqlquery_iterator.h>

#define NUMBER_ACTIVITIES 10
#define NUMBER_AGENTS     10
#define NUMBER_RESOURCES  50
#define NUMBER_CACHES    200

namespace KAStats = KActivities::Experimental::Stats;

static ResultSetQuickCheckTest * instance;

ResultSetQuickCheckTest::ResultSetQuickCheckTest(QObject *parent)
    : Test(parent)
    , activities(new KActivities::Consumer())
{
    instance = this;
}

namespace {
    QString resourceTitle(const QString &resource)
    {
        // We need to find the title
        ResourceInfo::Item key;
        key.targettedResource = resource;

        auto &infos = instance->resourceInfos;

        auto ri = infos.lower_bound(key);

        return
            (ri != infos.cend() && ri->targettedResource == resource) ?
            ri->title : resource;
    }

    QString toQString(const ResourceScoreCache::Item &item)
    {
        return
            item.targettedResource
            + ':' + resourceTitle(item.targettedResource)
            + '(' + QString::number(item.cachedScore) + ')';
    }

    QString toQString(const ResourceLink::Item &item)
    {
        return
            item.targettedResource
            + ':' + resourceTitle(item.targettedResource)
            // + '(' + QString::number(0/* item.score */) + ')'
            ;
    }

    QString toQString(const KAStats::ResultSet::Result &item)
    {
        return
            item.resource
            + ':' + item.title
            + '(' + QString::number(item.score) + ')';
    }

    bool operator==(const ResourceScoreCache::Item &left,
                    const KAStats::ResultSet::Result &right)
    {
        return left.targettedResource == right.resource
               && resourceTitle(left.targettedResource) == right.title
               && qFuzzyCompare(left.cachedScore, right.score);
    }

    bool operator==(const ResourceLink::Item &left,
                    const KAStats::ResultSet::Result &right)
    {
        return left.targettedResource == right.resource
               && resourceTitle(left.targettedResource) == right.title;
               // && qFuzzyCompare(left.cachedScore, right.score);
    }

    template<typename Left>
    void assert_range_equal(const Left &left, const KAStats::ResultSet &right,
            const char * file, int line)
    {
        auto leftIt  = left.cbegin();
        auto rightIt = right.cbegin();
        auto leftEnd  = left.cend();
        auto rightEnd = right.cend();

        bool equal = true;

        QString leftLine;
        QString rightLine;

        for (; leftIt != leftEnd && rightIt != rightEnd; ++leftIt, ++rightIt) {
            auto leftString = toQString(*leftIt);
            auto rightString = toQString(*rightIt);

            if (*leftIt == *rightIt) {
                rightString.fill('.');

            } else {
                equal = false;
            }

            int longer  = qMax(leftString.length(), rightString.length());
            leftString  = leftString.leftJustified(longer);
            rightString = rightString.leftJustified(longer, '.');

            leftLine += " " + leftString;
            rightLine += " " + rightString;
        }

        if (equal) {
            // So far, we are equal, but do we have the same number
            // of elements - did we reach the end of both ranges?
            if (leftIt != leftEnd) {
                QTest::qFail("The left range is longer than the right one", file, line);
            }

            if (rightIt != rightEnd) {
                QTest::qFail("The right range is longer than the left one", file, line);
            }

        } else {

            qDebug() << "Ranges differ:\n"
                     << "MEM: " << leftLine << '\n'
                     << "LIB: " << rightLine;
            QTest::qFail("Results do not match", file, line);
        }
    }

#define ASSERT_RANGE_EQUAL(L, R) assert_range_equal(L, R, __FILE__, __LINE__)
}

// {{{ Data init
void ResultSetQuickCheckTest::initTestCase()
{
    qsrand(time(NULL));

    QString databaseFile;

    int dbArgIndex = QCoreApplication::arguments().indexOf("--ResultSetQuickCheckDatabase");
    if (dbArgIndex > 0) {
        databaseFile = QCoreApplication::arguments()[dbArgIndex+1];

        qDebug() << "Using an existing database: " + databaseFile;
        Common::ResourcesDatabaseSchema::overridePath(databaseFile);

        pullFromDatabase();

    } else {

        QTemporaryDir dir(QDir::tempPath() + "/KActivitiesStatsTest_ResultSetQuickCheckTest_XXXXXX");
        dir.setAutoRemove(false);

        if (!dir.isValid()) {
            qFatal("Can not create a temporary directory");
        }

        databaseFile = dir.path() + "/database";

        qDebug() << "Creating database in " << databaseFile;
        Common::ResourcesDatabaseSchema::overridePath(databaseFile);

        while (activities->serviceStatus() == KActivities::Consumer::Unknown) {
            QCoreApplication::processEvents();
        }

        generateActivitiesList();
        generateAgentsList();
        generateTypesList();
        generateResourcesList();

        generateResourceInfos();
        generateResourceScoreCaches();
        generateResourceLinks();

        pushToDatabase();
    }


    qDebug() << "Init finished";
}

void ResultSetQuickCheckTest::generateActivitiesList()
{
    activitiesList = activities->activities();

    while (activitiesList.size() < NUMBER_ACTIVITIES) {
        activitiesList << QUuid::createUuid().toString().mid(1, 36);
    }

    qDebug() << "Generated/Activities:" << activitiesList;
}

void ResultSetQuickCheckTest::generateAgentsList()
{
    for (int i = 0; i < NUMBER_AGENTS; ++i) {
        agentsList << "Agent_" + QString::number(i);
    }

    qDebug() << "Generated/Agents:" << agentsList;
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

void ResultSetQuickCheckTest::generateResourceInfos()
{
    foreach (const QString &resource, resourcesList) {
        // We want every n-th or so to be without the title
        if (qrand() % 3) continue;

        ResourceInfo::Item ri;
        ri.targettedResource = resource;
        ri.title = "Title_" + QString::number(qrand() % 100);
        ri.mimetype = randItem(typesList);

        resourceInfos.insert(ri);
    }
}

void ResultSetQuickCheckTest::generateResourceScoreCaches()
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

void ResultSetQuickCheckTest::generateResourceLinks()
{
    foreach (const QString &resource, resourcesList) {
        // We don't want all the resources to be linked
        // to something
        if (qrand() % 2) continue;

        ResourceLink::Item rl;

        rl.targettedResource = resource;
        rl.usedActivity      = randItem(activitiesList);
        rl.initiatingAgent   = randItem(agentsList);

        resourceLinks.insert(rl);
    }
}

void ResultSetQuickCheckTest::pushToDatabase()
{
    // Creating the database, and pushing some dummy data into it
    auto database = Common::Database::instance(Common::Database::ResourcesDatabase,
                                               Common::Database::ReadWrite);

    Common::ResourcesDatabaseSchema::initSchema(*database);

    // Inserting resource score caches
    qDebug() << "Inserting" << resourceScoreCaches.size() << "items into ResourceScoreCache";
    int i = 0;

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

    // Inserting resource links
    qDebug() << "Inserting" << resourceLinks.size() << "items into ResourceLink";
    i = 0;

    foreach (const auto &rl, resourceLinks) {
        std::cerr << '.';

        if (++i % 10 == 0) std::cerr << i;

        database->execQuery(QStringLiteral(
            "INSERT INTO ResourceLink ("
                "  targettedResource"
                ", usedActivity"
                ", initiatingAgent"
            ") VALUES ("
                "  '%1' " // targettedResource
                ", '%2' " // usedActivity
                ", '%3' " // initiatingAgent
            ")"
        )
            .arg(rl.targettedResource)
            .arg(rl.usedActivity)
            .arg(rl.initiatingAgent)
        );

    }
    std::cerr << std::endl;
}


void ResultSetQuickCheckTest::pullFromDatabase()
{
    auto database = Common::Database::instance(Common::Database::ResourcesDatabase,
                                               Common::Database::ReadWrite);

    auto rscQuery = database->execQuery("SELECT * FROM ResourceScoreCache");

    for (const auto &rsc: rscQuery) {
        ResourceScoreCache::Item item;
        item.usedActivity      = rsc["usedActivity"].toString();
        item.initiatingAgent   = rsc["initiatingAgent"].toString();
        item.targettedResource = rsc["targettedResource"].toString();
        item.cachedScore       = rsc["cachedScore"].toDouble();
        item.firstUpdate       = rsc["firstUpdate"].toInt();
        item.lastUpdate        = rsc["lastUpdate"].toInt();
        resourceScoreCaches.insert(item);
    }

    auto riQuery = database->execQuery("SELECT * FROM ResourceInfo");

    for (const auto& ri: riQuery) {
        ResourceInfo::Item item;
        item.targettedResource = ri["targettedResource"].toString();
        item.title             = ri["title"].toString();
        item.mimetype          = ri["mimetype"].toString();
        resourceInfos.insert(item);
    }
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

    foreach (const auto &agent, agentsList) {
        auto memItems = ResourceScoreCache::groupByResource(
                resourceScoreCaches
                | filtered(ResourceScoreCache::initiatingAgent() == agent)
            );

        auto baseTerm = UsedResources | Agent{agent} | Activity::any();

        #define ORDERING_TEST(Column, Dir, OrderFlag)                          \
        {                                                                      \
            sort(memItems, ResourceScoreCache::Column().Dir()                  \
                           | ResourceScoreCache::targettedResource().asc());   \
            ResultSet dbItems = baseTerm | OrderFlag;                          \
            ASSERT_RANGE_EQUAL(memItems, dbItems);                             \
        }

        ORDERING_TEST(targettedResource, asc,  OrderByUrl)
        ORDERING_TEST(cachedScore,       desc, HighScoredFirst);
        ORDERING_TEST(lastUpdate,        desc, RecentlyUsedFirst);
        ORDERING_TEST(firstUpdate,       desc, RecentlyCreatedFirst);

        #undef ORDERING_TEST

    }

}

void ResultSetQuickCheckTest::testUsedResourcesForActivities()
{
}

void ResultSetQuickCheckTest::testLinkedResourcesForAgents()
{
    using namespace KAStats;
    using namespace KAStats::Terms;
    using boost::sort;
    using boost::adaptors::filtered;

    foreach (const auto &agent, agentsList) {
        auto memItems = ResourceLink::groupByResource(
                resourceLinks
                | filtered(ResourceLink::initiatingAgent() == agent)
            );

        auto baseTerm = LinkedResources | Agent{agent} | Activity::any();

        #define ORDERING_TEST(Column, Dir, OrderFlag)                          \
        {                                                                      \
            sort(memItems, ResourceLink::Column().Dir()                        \
                           | ResourceLink::targettedResource().asc());         \
            ResultSet dbItems = baseTerm | OrderFlag;                          \
            ASSERT_RANGE_EQUAL(memItems, dbItems);                             \
        }

        ORDERING_TEST(targettedResource, asc,  OrderByUrl)
        // ORDERING_TEST(cachedScore,       desc, HighScoredFirst);
        // ORDERING_TEST(lastUpdate,        desc, RecentlyUsedFirst);
        // ORDERING_TEST(firstUpdate,       desc, RecentlyCreatedFirst);

        #undef ORDERING_TEST

    }

}

// vim: set foldmethod=marker:
