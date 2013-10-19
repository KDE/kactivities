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

#ifndef COMMON_TEST_H
#define COMMON_TEST_H

#include <QObject>
#include <QFuture>
#include <QFutureWatcher>

class Test: public QObject {
    Q_OBJECT
public:
    Test(QObject *parent = nullptr);

protected:
    enum WhenToFail {
        DontFail = 0,
        FailIfTrue = 1,
        FailIfFalse = 2
    };

    template <typename _ReturnType, typename _Continuation>
    void continue_future(const QFuture<_ReturnType> &future,
                         _Continuation &&continuation)
    {
        if (!future.isFinished()) {
            auto watcher = new QFutureWatcher<decltype(future.result())>();
            QObject::connect(watcher, &QFutureWatcherBase::finished,
                [=] {
                    continuation(watcher->result());
                    watcher->deleteLater();
                }
            );

            watcher->setFuture(future);

        } else {
            continuation(future.result());

        }
    }

    template <typename _Continuation>
    void continue_future(const QFuture<void> &future,
                         _Continuation &&continuation)
    {
        if (!future.isFinished()) {
            auto watcher = new QFutureWatcher<void>();
            QObject::connect(watcher, &QFutureWatcherBase::finished,
                [=] {
                    continuation();
                    watcher->deleteLater();
                }
            );

            watcher->setFuture(future);

        } else {
            continuation();

        }
    }


    template <typename T>
    static bool check(T what, WhenToFail wtf = DontFail,
                      const char *msg = nullptr)
    {
        bool result = what();

        if (
            (wtf == FailIfTrue && result) ||
            (wtf == FailIfFalse && !result)
        ) {
            qFatal(
                "\n"
                "\n"
                "!!! > \n"
                "!!! > %s\n"
                "!!! > \n"
                ,  msg);
        }

        return result;
    }

    static bool inEmptySession();
    static bool isActivityManagerRunning();

Q_SIGNALS:
    void testFinished();
};

#define CHECK_CONDITION(A, B) check(A, B, #A " raised " #B)

#endif /* TEST_H */

