/*
 *   Copyright (C) 2014 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser General Public License for more details
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "mainthreadexecutor_p.h"

#include <mutex>

#include <QCoreApplication>
#include <QDBusConnection>
#include <QMetaObject>
#include <QThread>

namespace KActivities {

namespace detail {

MainThreadExecutor::MainThreadExecutor(std::function<void()> &&f)
    : m_function(std::forward<std::function<void()>>(f))
{
}

void MainThreadExecutor::start()
{
    m_function();
    deleteLater();
}

} // namespace detail

void runInMainThread(std::function<void()> &&f)
{
    static auto mainThread = QCoreApplication::instance()->thread();

    if (QThread::currentThread() == mainThread) {
        f();

    } else {
        auto executor = new detail::MainThreadExecutor(std::forward<std::function<void()>>(f));

        executor->moveToThread(mainThread);

        QMetaObject::invokeMethod(executor, "start", Qt::BlockingQueuedConnection);
    }
}

} // namespace KActivities
