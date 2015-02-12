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

#include "QueryTest.h"

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QString>
#include <QDebug>
#include <QTest>

#include <query.h>
#include <common/test.h>

namespace KAStats = KActivities::Experimental::Stats;
using namespace KAStats;
using namespace KAStats::Terms;

QueryTest::QueryTest(QObject *parent)
    : Test(parent)
{
}

void QueryTest::testDefaults()
{
    TEST_CHUNK("Testing the term defaults");

    Query query;

    QCOMPARE(query.selection(),  AllResources);
    QCOMPARE(query.types(),      {":any"});
    QCOMPARE(query.agents(),     {":current"});
    QCOMPARE(query.activities(), {":current"});
    QCOMPARE(query.ordering(),   HighScoredFirst);
}

void QueryTest::testDebuggingOutput()
{
    TEST_CHUNK("Debugging output for a query");

    Query query;

    // Testing whether qDebug can be called (compilation check)
    qDebug() << "Writing out a query:" << query;
}

void QueryTest::testDerivationFromDefault()
{
    TEST_CHUNK("Testing query derivation from default")

    Query queryDefault;
    auto  queryDerived = queryDefault | LinkedResources;

    // queryDefault should not have been modified
    QCOMPARE(queryDefault.selection(),  AllResources);
    QCOMPARE(queryDerived.selection(),  LinkedResources);

    // Changing queryDerived back to AllResources, should be == to queryDefault
    queryDerived.setSelection(AllResources);
    QCOMPARE(queryDefault, queryDerived);
}

void QueryTest::testDerivationFromCustom()
{
    TEST_CHUNK("Testing query derivation from custom")

    Query queryCustom;
    auto  queryDerived = queryCustom | LinkedResources;

    // q1 should not have been modified
    QCOMPARE(queryCustom.selection(),  AllResources);
    QCOMPARE(queryDerived.selection(),  LinkedResources);

    // Changing queryDerived back to AllResources, should be == to queryDefault
    queryDerived.setSelection(AllResources);
    QCOMPARE(queryCustom, queryDerived);
}

void QueryTest::testNormalSyntaxAgentManipulation()
{
    TEST_CHUNK("Testing normal syntax manipulation: Agents")

    Query query;
    query.addAgents(QStringList() << "gvim" << "kate");

    QCOMPARE(query.agents(), QStringList() << "gvim" << "kate");

    query.addAgents(QStringList() << "kwrite");

    QCOMPARE(query.agents(), QStringList() << "gvim" << "kate" << "kwrite");

    query.clearAgents();

    QCOMPARE(query.agents(), QStringList() << ":current");
}

void QueryTest::testNormalSyntaxTypeManipulation()
{
    TEST_CHUNK("Testing normal syntax manipulation: Types")

    Query query;
    query.addTypes(QStringList() << "text/html" << "text/plain");

    QCOMPARE(query.types(), QStringList() << "text/html" << "text/plain");

    query.addTypes(QStringList() << "text/xml");

    QCOMPARE(query.types(), QStringList() << "text/html" << "text/plain" << "text/xml");

    query.clearTypes();

    QCOMPARE(query.types(), QStringList() << ":any");
}

void QueryTest::testNormalSyntaxActivityManipulation()
{
    TEST_CHUNK("Testing normal syntax manipulation: Activities")

    Query query;
    query.addActivities(QStringList() << "a1" << "a2");

    QCOMPARE(query.activities(), QStringList() << "a1" << "a2");

    query.addActivities(QStringList() << "a3");

    QCOMPARE(query.activities(), QStringList() << "a1" << "a2" << "a3");

    query.clearActivities();

    QCOMPARE(query.activities(), QStringList() << ":current");
}

void QueryTest::testNormalSyntaxOrderingManipulation()
{
    TEST_CHUNK("Testing normal syntax manipulation: Activities")

    Query query;

    QCOMPARE(query.ordering(), HighScoredFirst);

    query.setOrdering(RecentlyCreatedFirst);

    QCOMPARE(query.ordering(), RecentlyCreatedFirst);

    query.setOrdering(OrderByUrl);

    QCOMPARE(query.ordering(), OrderByUrl);
}

void QueryTest::testFancySyntaxBasic()
{
    TEST_CHUNK("Testing the fancy syntax, non c++11")

    auto query = LinkedResources
                    | Type("text")
                    | Type("image")
                    | Agent("test")
                    | RecentlyCreatedFirst;

    QCOMPARE(query.selection(),  LinkedResources);
    QCOMPARE(query.types(),      QStringList() << "text" << "image");
    QCOMPARE(query.agents(),     QStringList() << "test");
    QCOMPARE(query.activities(), QStringList() << ":current");
    QCOMPARE(query.ordering(),   RecentlyCreatedFirst);

    #ifdef Q_COMPILER_INITIALIZER_LISTS
    TEST_CHUNK("Testing the fancy syntax, c++11")

    // Testing the fancy c++11 syntax
    auto queryCXX11 = LinkedResources
                | Type{"text", "image"}
                | Agent{"test"}
                | RecentlyCreatedFirst;

    QCOMPARE(query, queryCXX11);
    #endif
}

void QueryTest::testFancySyntaxAgentDefinition()
{
    TEST_CHUNK("Testing the fancy syntax, agent definition")

    {
        auto query = LinkedResources | OrderByUrl;
        QCOMPARE(query.agents(), QStringList() << ":current");
    }

    {
        auto query = LinkedResources | Agent("gvim");
        QCOMPARE(query.agents(), QStringList() << "gvim");
    }

    {
        auto query = LinkedResources | Agent("gvim") | Agent("kate");
        QCOMPARE(query.agents(), QStringList() << "gvim" << "kate");
    }

    {
        auto query = LinkedResources | Agent(QStringList() << "gvim" << "kate");
        QCOMPARE(query.agents(), QStringList() << "gvim" << "kate");
    }
}

void QueryTest::testFancySyntaxTypeDefinition()
{
    TEST_CHUNK("Testing the fancy syntax, type definition")

    {
        auto query = LinkedResources | OrderByUrl;
        QCOMPARE(query.types(), QStringList() << ":any");
    }

    {
        auto query = LinkedResources | Type("text/plain");
        QCOMPARE(query.types(), QStringList() << "text/plain");
    }

    {
        auto query = LinkedResources | Type("text/plain") | Type("text/html");
        QCOMPARE(query.types(), QStringList() << "text/plain" << "text/html");
    }

    {
        auto query = LinkedResources | Type(QStringList() << "text/plain" << "text/html");
        QCOMPARE(query.types(), QStringList() << "text/plain" << "text/html");
    }
}

void QueryTest::testFancySyntaxActivityDefinition()
{
    TEST_CHUNK("Testing the fancy syntax, activity definition")

    {
        auto query = LinkedResources | OrderByUrl;
        QCOMPARE(query.activities(), QStringList() << ":current");
    }

    {
        auto query = LinkedResources | Activity("gvim");
        QCOMPARE(query.activities(), QStringList() << "gvim");
    }

    {
        auto query = LinkedResources | Activity("gvim") | Activity("kate");
        QCOMPARE(query.activities(), QStringList() << "gvim" << "kate");
    }

    {
        auto query = LinkedResources | Activity(QStringList() << "gvim" << "kate");
        QCOMPARE(query.activities(), QStringList() << "gvim" << "kate");
    }
}

void QueryTest::testFancySyntaxOrderingDefinition()
{
    TEST_CHUNK("Testing the fancy syntax, activity definition")

    {
        auto query = LinkedResources | OrderByUrl;
        QCOMPARE(query.ordering(), OrderByUrl);
    }

    {
        auto query = LinkedResources | HighScoredFirst;
        QCOMPARE(query.ordering(), HighScoredFirst);
    }

    {
        auto query = LinkedResources | RecentlyCreatedFirst;
        QCOMPARE(query.ordering(), RecentlyCreatedFirst);
    }

    {
        auto query = LinkedResources | RecentlyCreatedFirst | OrderByUrl;
        QCOMPARE(query.ordering(), OrderByUrl);
    }

    {
        auto query = LinkedResources | RecentlyCreatedFirst | HighScoredFirst;
        QCOMPARE(query.ordering(), HighScoredFirst);
    }
}

void QueryTest::initTestCase()
{
    // CHECK_CONDITION(isActivityManagerRunning, FailIfTrue);
}

void QueryTest::cleanupTestCase()
{
    emit testFinished();
}

