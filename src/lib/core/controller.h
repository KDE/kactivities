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

#ifndef ACTIVITIES_CONTROLLER_H
#define ACTIVITIES_CONTROLLER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QFuture>

#include "consumer.h"

#include "kactivities_export.h"

namespace KActivities {

class ControllerPrivate;

/**
 * This class provides methods for controlling and managing
 * the activities.
 *
 * @note The QFuture objects returned by these methods are not thread-based,
 * you can not call synchronous methods like waitForFinished, cancel, pause on
 * them. You need either to register watchers to check when those have finished,
 * or to check whether they are ready from time to time manually.
 *
 * @see Consumer for info about activities
 *
 * @since 5.0
 */
class KACTIVITIES_EXPORT Controller : public Consumer {
    Q_OBJECT

    Q_PROPERTY(QString currentActivity READ currentActivity WRITE setCurrentActivity)

public:
    explicit Controller(QObject *parent = Q_NULLPTR);

    ~Controller();

    /**
     * Sets the name of the specified activity
     * @param id id of the activity
     * @param name name to be set
     */
    QFuture<void> setActivityName(const QString &id, const QString &name);

    /**
     * Sets the icon of the specified activity
     * @param id id of the activity
     * @param icon icon to be set - freedesktop.org name or file path
     */
    QFuture<void> setActivityIcon(const QString &id, const QString &icon);

    /**
     * Sets the current activity
     * @param id id of the activity to make current
     * @returns true if successful
     */
    QFuture<bool> setCurrentActivity(const QString &id);

    /**
     * Adds a new activity
     * @param name name of the activity
     * @returns id of the newly created activity
     */
    QFuture<QString> addActivity(const QString &name);

    /**
     * Removes the specified activity
     * @param id id of the activity to delete
     */
    QFuture<void> removeActivity(const QString &id);

    /**
     * Stops the activity
     * @param id id of the activity to stop
     */
    QFuture<void> stopActivity(const QString &id);

    /**
     * Starts the activity
     * @param id id of the activity to start
     */
    QFuture<void> startActivity(const QString &id);

private:
    // const QScopedPointer<ControllerPrivate> d;
};

} // namespace KActivities

#endif // ACTIVITIES_CONTROLLER_H
