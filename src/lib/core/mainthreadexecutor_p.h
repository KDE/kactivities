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

#ifndef ACTIVITIES_MAINTHREADEXECUTOR_P
#define ACTIVITIES_MAINTHREADEXECUTOR_P

#include <functional>

#include <QObject>

namespace KActivities {

namespace detail {
    class MainThreadExecutor: public QObject {
        Q_OBJECT

    public:
        MainThreadExecutor(std::function<void()> &&f);

        Q_INVOKABLE void start();

    private:
        std::function<void()> m_function;
    };
} // namespace detail

void runInMainThread(std::function<void()> &&f);

} // namespace KActivities

#endif // ACTIVITIES_MAINTHREADEXECUTOR_P
