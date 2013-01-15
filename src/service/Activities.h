/*
 *   Copyright (C) 2010, 2011, 2012 Ivan Cukic <ivan.cukic(at)kde.org>
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

#ifndef ACTIVITIES_H
#define ACTIVITIES_H

#include <QString>
#include <QStringList>

#include <Module.h>

#include <utils/d_ptr.h>
#include <utils/nullptr.h>

#include <common/dbus/org.kde.ActivityManager.Activities.h>

/**
 * Service for tracking the user actions and managing the
 * activities
 */
class Activities: public Module {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.ActivityManager.Activities")
    Q_PROPERTY(QString CurrentActivity READ CurrentActivity WRITE SetCurrentActivity NOTIFY CurrentActivityChanged)

public:
    /**
     * Activity state
     * @note: Do not change the values, needed for bit-operations
     */
    enum State {
        Invalid  = 0,
        Running  = 2,
        Starting = 3,
        Stopped  = 4,
        Stopping = 5
    };

    /**
     * The event type
     */
    enum EventType {
        Accessed = 1,
        Opened = 2,
        Modified = 3,
        Closed = 4,
        FocussedIn = 5,
        FocussedOut = 6
    };

    /**
     * Creates new Activities object
     */
    Activities(QObject * parent = nullptr);

    /**
     * Destroys this interface
     */
    virtual ~Activities();

// workspace activities control
public Q_SLOTS:
    /**
     * @returns the id of the current activity, empty string if none
     */
    QString CurrentActivity() const;

    /**
     * Sets the current activity
     * @param activity id of the activity to make current
     */
    bool SetCurrentActivity(const QString & activity);

    /**
     * Adds a new activity
     * @param name name of the activity
     * @returns id of the newly created activity
     */
    QString AddActivity(const QString & name);

    /**
     * Starts the specified activity
     * @param activity id of the activity to stash
     */
    void StartActivity(const QString & activity);

    /**
     * Stops the specified activity
     * @param activity id of the activity to stash
     */
    void StopActivity(const QString & activity);

    /**
     * @returns the state of the activity
     * @param activity id of the activity
     */
    int ActivityState(const QString & activity) const;

    /**
     * Removes the specified activity
     * @param activity id of the activity to delete
     */
    void RemoveActivity(const QString & activity);

    /**
     * @returns the list of all existing activities
     */
    QStringList ListActivities() const;

    /**
     * @returns the list of activities with the specified state
     * @param state state
     */
    QStringList ListActivities(int state) const;

    /**
     * @returns the name of the specified activity
     * @param activity id of the activity
     */
    QString ActivityName(const QString & activity) const;

    /**
     * Sets the name of the specified activity
     * @param activity id of the activity
     * @param name name to be set
     */
    void SetActivityName(const QString & activity, const QString & name);

    /**
     * @returns the icon of the specified activity
     * @param activity id of the activity
     */
    QString ActivityIcon(const QString & activity) const;

    /**
     * Sets the icon of the specified activity
     * @param activity id of the activity
     * @param icon icon to be set
     */
    void SetActivityIcon(const QString & activity, const QString & icon);

public Q_SLOTS:
    /**
     * @returns a list of activities with basic info about them
     */
    ActivityInfoList ListActivitiesWithInformation() const;

    /**
     * @returns the info about an activity
     */
    ActivityInfo ActivityInformation(const QString & activity) const;

Q_SIGNALS:
    /**
     * This signal is emitted when the global
     * activity is changed
     * @param activity id of the new current activity
     */
    void CurrentActivityChanged(const QString & activity);

    /**
     * This signal is emitted when a new activity is created
     * @param activity id of the activity
     */
    void ActivityAdded(const QString & activity);

    /**
     * This signal is emitted when an activity is started
     * @param activity id of the activity
     */
    void ActivityStarted(const QString & activity);

    /**
     * This signal is emitted when an activity is stashed
     * @param activity id of the activity
     */
    void ActivityStopped(const QString & activity);

    /**
     * This signal is emitted when an activity is deleted
     * @param activity id of the activity
     */
    void ActivityRemoved(const QString & activity);

    /**
     * Emitted when an activity name is changed
     * @param activity id of the changed activity
     * @param name name of the changed activity
     */
    void ActivityNameChanged(const QString & activity, const QString & name);

    /**
     * Emitted when an activity icon is changed
     * @param activity id of the changed activity
     * @param icon name of the changed activity
     */
    void ActivityIconChanged(const QString & activity, const QString & icon);

    /**
     * Emitted when an activity is changed (name, icon, or some other property)
     * @param activity id of the changed activity
     */
    void ActivityChanged(const QString & activity);

    /**
     * Emitted when the state of activity is changed
     */
    void ActivityStateChanged(const QString & activity, int state);


public:
    virtual bool isFeatureOperational(const QStringList & feature) const _override;
    virtual bool isFeatureEnabled(const QStringList & feature) const _override;
    virtual void setFeatureEnabled(const QStringList & feature, bool value) _override;
    virtual QStringList listFeatures(const QStringList & feature) const _override;

private:
    D_PTR;
};


#endif // ACTIVITIES_H
