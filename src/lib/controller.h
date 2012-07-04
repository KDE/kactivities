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
#include <QWidget>
#include <QString>
#include <QStringList>

#include "consumer.h"

#include <kurl.h>
#include "kactivities_export.h"

namespace KActivities {

class ControllerPrivate;

/**
 * This class provides methods for controlling and managing
 * the activities.
 *
 * @see Consumer for info about activities
 *
 * @since 4.5
 */
class KACTIVITIES_EXPORT Controller: public Consumer
{
    Q_OBJECT

    Q_PROPERTY(QString currentActivity READ currentActivity WRITE setCurrentActivity)

public:
    explicit Controller(QObject * parent = 0 /*nullptr*/);

    ~Controller();

    /**
     * Sets the name of the specified activity
     * @param id id of the activity
     * @param name name to be set
     */
    void setActivityName(const QString & id, const QString & name);

    /**
     * Sets the icon of the specified activity
     * @param id id of the activity
     * @param icon icon to be set - freedesktop.org name or file path
     */
    void setActivityIcon(const QString & id, const QString & icon);

    /**
     * Sets whether the activity should be encrypted
     * @param id id of the activity
     * @param encrypted should the activity be encrypted
     */
    KDE_DEPRECATED
    void setActivityEncrypted(const QString & id, bool encrypted);

    /**
     * Sets the current activity
     * @param id id of the activity to make current
     * @returns true if successful
     */
    bool setCurrentActivity(const QString & id);

    /**
     * Adds a new activity
     * @param name name of the activity
     * @returns id of the newly created activity
     */
    QString addActivity(const QString & name);

    /**
     * Removes the specified activity
     * @param id id of the activity to delete
     */
    void removeActivity(const QString & id);

    /**
     * Stops the activity
     * @param id id of the activity to stop
     */
    void stopActivity(const QString & id);

    /**
     * Starts the activity
     * @param id id of the activity to start
     */
    void startActivity(const QString & id);

private:
    ControllerPrivate * const d;
};

} // namespace KActivities

#endif // ACTIVITIES_CONTROLLER_H

