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

#include <QtTest/QtTest>
#include <QCoreApplication>
#include <QList>

#include <common/test.h>

#include "QueryTest.h"
#include "ResultSetTest.h"
#include "ResultSetQuickCheckTest.h"
#include "ResultWatcherTest.h"

class TestRunner: public QObject {
public:
    TestRunner()
        : m_nextToStart(0)
    {
    }

    TestRunner &addTest(Test *test)
    {
        if (m_nextToStart == 0)
            m_tests << test;
        return *this;
    }

    TestRunner &operator<<(Test *test)
    {
        addTest(test);
        return *this;
    }

    void start()
    {
        if (m_nextToStart)
            return;

        if (m_tests.size() == 0) {
            // We do not have a QCoreApplication here, calling system exit
            ::exit(0);
            return;
        }

        next();
    }

private:
    void next()
    {
        if (m_nextToStart >= m_tests.size()) {
            QCoreApplication::exit(0);
            return;
        }

        Test *test = m_tests[m_nextToStart++];

        QObject::connect(test, &Test::testFinished,
                this, &TestRunner::next,
                Qt::QueuedConnection);

        QTest::qExec(test);

    }

private:
    QList<Test*> m_tests;
    int m_nextToStart;

};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    TestRunner &runner = *(new TestRunner());

    qDebug() << app.arguments();

    bool all = (app.arguments().size() <= 1);

#define ADD_TEST(TestName)                                                     \
    qDebug() << "Test " << #TestName << " is enabled "                         \
             << (all || app.arguments().contains(#TestName));                  \
    if (all || app.arguments().contains(#TestName)) {                          \
        runner << new TestName##Test();                                        \
    }

    ADD_TEST(Query)
    ADD_TEST(ResultSet)
    ADD_TEST(ResultSetQuickCheck)
    ADD_TEST(ResultWatcher)

    runner.start();

#undef ADD_TEST
    return app.exec();
    // QTest::qExec(&tc, argc, argv);
}

