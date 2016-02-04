/*
 *   Copyright (C) 2010 - 2016 by Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) version 3, or any
 *   later version accepted by the membership of KDE e.V. (or its
 *   successor approved by the membership of KDE e.V.), which shall
 *   act as a proxy defined in Section 6 of version 3 of the license.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library.
 *   If not, see <http://www.gnu.org/licenses/>.
 */

#include "controller.h"
#include "consumer_p.h"
#include "manager_p.h"

#include "utils/dbusfuture_p.h"

#include <QObject>

namespace KActivities {

Controller::Controller(QObject *parent)
    : Consumer(parent)
{
}

Controller::~Controller()
{
}

#define CREATE_SETTER(What)                                                    \
    QFuture<void> Controller::setActivity##What(const QString &id,             \
                                                const QString &value)          \
    {                                                                          \
        return Manager::isServiceRunning()                                     \
                   ? DBusFuture::asyncCall<void>(                              \
                         Manager::activities(),                                \
                         QString::fromLatin1("SetActivity" #What), id, value)  \
                   : DBusFuture::fromVoid();                                   \
    }

CREATE_SETTER(Name)
CREATE_SETTER(Description)
CREATE_SETTER(Icon)

#undef CREATE_SETTER

QFuture<bool> Controller::setCurrentActivity(const QString &id)
{
    // Q_ASSERT_X(activities().contains(id), "Controller::setCurrentActivity",
    //            "You can not set an non-existent activity to be the current");

    // return Manager::activities()->SetCurrentActivity(id);
    return Manager::isServiceRunning() ?
        DBusFuture::asyncCall<bool>(
            Manager::activities(), QStringLiteral("SetCurrentActivity"), id)
        :
        DBusFuture::fromValue(false);
}

QFuture<QString> Controller::addActivity(const QString &name)
{
    Q_ASSERT_X(!name.isEmpty(), "Controller::addActivity",
               "The activity name can not be an empty string");

    // return Manager::activities()->AddActivity(name);
    return Manager::isServiceRunning() ?
        DBusFuture::asyncCall<QString>(
            Manager::activities(), QStringLiteral("AddActivity"), name)
        :
        DBusFuture::fromValue(QString());
}

QFuture<void> Controller::removeActivity(const QString &id)
{
    // Q_ASSERT_X(activities().contains(id), "Controller::removeActivity",
    //            "You can not remove an non-existent activity");

    // Manager::activities()->RemoveActivity(id);
    return Manager::isServiceRunning() ?
        DBusFuture::asyncCall<void>(
            Manager::activities(), QStringLiteral("RemoveActivity"), id)
        :
        DBusFuture::fromVoid();
}

QFuture<void> Controller::stopActivity(const QString &id)
{
    // Q_ASSERT_X(activities().contains(id), "Controller::stopActivity",
    //            "You can not stop an non-existent activity");

    // Manager::activities()->StopActivity(id);
    return Manager::isServiceRunning() ?
        DBusFuture::asyncCall<void>(
            Manager::activities(), QStringLiteral("StopActivity"), id)
        :
        DBusFuture::fromVoid();
}

QFuture<void> Controller::startActivity(const QString &id)
{
    // Q_ASSERT_X(activities().contains(id), "Controller::startActivity",
    //            "You can not start an non-existent activity");

    // Manager::activities()->StartActivity(id);
    return Manager::isServiceRunning() ?
        DBusFuture::asyncCall<void>(
            Manager::activities(), QStringLiteral("StartActivity"), id)
        :
        DBusFuture::fromVoid();
}

} // namespace KActivities

// #include "controller.moc"
