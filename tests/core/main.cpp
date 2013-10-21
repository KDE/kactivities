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

#include "Process.h"
#include "OfflineTest.h"
#include "CleanOnlineTest.h"

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

    (runner
        << Process::exec(Process::Kill)

        // Running the tests for when the service is offline
        << new OfflineTest()

        // Running the offline tests again so that we are sure
        // nothing has changed -- no activities created, changed etc.
        << new OfflineTest()

        // Starting the manager
        << Process::exec(Process::Start)

        // Starting the online tests
        << new CleanOnlineTest()
        << new CleanOnlineSetup()
        << new OnlineTest()

        // Starting the manager
        << Process::exec(Process::Stop)

        << new OfflineTest()
        << new OfflineTest()

        << Process::exec(Process::Start)
        << new OnlineTest()

        << Process::exec(Process::Stop)

    ).start();


    return app.exec();
    // QTest::qExec(&tc, argc, argv);
}

