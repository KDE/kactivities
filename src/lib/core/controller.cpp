/*
 * Copyright (c) 2010, 2011, 2012 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "controller.h"
#include "consumer_p.h"
#include "manager_p.h"

#include "utils/dbusfuture_p.h"

#include <QObject>

namespace KActivities {

// class ControllerPrivate : public QObject {
// public:
//     ControllerPrivate(Controller *parent)
//     {
//     }
// };

Controller::Controller(QObject *parent)
    : Consumer(parent)
    // , d(new ControllerPrivate())
{
}

Controller::~Controller()
{
}

QFuture<void> Controller::setActivityName(const QString &id, const QString &name)
{
    // Manager::activities()->SetActivityName(id, name);
    return Manager::isServiceRunning() ?
        DBusFuture::asyncCall<void>(
            Manager::activities(), QStringLiteral("SetActivityName"), id, name)
        :
        DBusFuture::fromVoid();
}

QFuture<void> Controller::setActivityIcon(const QString &id,
                                          const QString &icon)
{
    // Manager::activities()->SetActivityIcon(id, icon);
    return Manager::isServiceRunning() ?
        DBusFuture::asyncCall<void>(
            Manager::activities(), QStringLiteral("SetActivityIcon"), id, icon)
        :
        DBusFuture::fromVoid();

}

QFuture<bool> Controller::setCurrentActivity(const QString &id)
{
    // return Manager::activities()->SetCurrentActivity(id);
    return Manager::isServiceRunning() ?
        DBusFuture::asyncCall<bool>(
            Manager::activities(), QStringLiteral("SetCurrentActivity"), id)
        :
        DBusFuture::fromValue(false);
}

QFuture<QString> Controller::addActivity(const QString &name)
{
    // return Manager::activities()->AddActivity(name);
    return Manager::isServiceRunning() ?
        DBusFuture::asyncCall<QString>(
            Manager::activities(), QStringLiteral("AddActivity"), name)
        :
        DBusFuture::fromValue(QString());
}

QFuture<void> Controller::removeActivity(const QString &id)
{
    // Manager::activities()->RemoveActivity(id);
    return Manager::isServiceRunning() ?
        DBusFuture::asyncCall<void>(
            Manager::activities(), QStringLiteral("RemoveActivity"), id)
        :
        DBusFuture::fromVoid();
}

QFuture<void> Controller::stopActivity(const QString &id)
{
    // Manager::activities()->StopActivity(id);
    return Manager::isServiceRunning() ?
        DBusFuture::asyncCall<void>(
            Manager::activities(), QStringLiteral("StopActivity"), id)
        :
        DBusFuture::fromVoid();
}

QFuture<void> Controller::startActivity(const QString &id)
{
    // Manager::activities()->StartActivity(id);
    return Manager::isServiceRunning() ?
        DBusFuture::asyncCall<void>(
            Manager::activities(), QStringLiteral("StartActivity"), id)
        :
        DBusFuture::fromVoid();
}

} // namespace KActivities

#include "controller.moc"
