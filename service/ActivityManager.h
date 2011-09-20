/*
 *   Copyright (C) 2010 Ivan Cukic <ivan.cukic(at)kde.org>
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

#ifndef ACTIVITY_MANAGER_H_
#define ACTIVITY_MANAGER_H_

#define ActivityManagerServicePath "org.kde.ActivityManager"

#include <QString>
#include <QStringList>

#include <KUniqueApplication>

class ActivityManagerPrivate;

/**
 * Service for tracking the user actions and managing the
 * activities
 */
class ActivityManager: public KUniqueApplication {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.ActivityManager")

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
     * Creates new ActivityManager
     */
    ActivityManager();

    /**
     * Destroys this interface
     */
    virtual ~ActivityManager();

    static ActivityManager* self();

// service control methods
public Q_SLOTS:
    /**
     * Does nothing. If the service is not running, the D-Bus daemon
     * will automatically create it
     */
    void Start();

    /**
     * Stops the service
     */
    void Stop();



// workspace activities control
public Q_SLOTS:
    /**
     * @returns the id of the current activity, empty string if none
     */
    QString CurrentActivity() const;

    /**
     * Sets the current activity
     * @param id id of the activity to make current
     */
    bool SetCurrentActivity(const QString & id);

    /**
     * Adds a new activity
     * @param name name of the activity
     * @returns id of the newly created activity
     */
    QString AddActivity(const QString & name);

    /**
     * Starts the specified activity
     * @param id id of the activity to stash
     */
    void StartActivity(const QString & id);

    /**
     * Stops the specified activity
     * @param id id of the activity to stash
     */
    void StopActivity(const QString & id);

    /**
     * @returns the state of the activity
     * @param id id of the activity
     */
    int ActivityState(const QString & id) const;

    /**
     * Removes the specified activity
     * @param id id of the activity to delete
     */
    void RemoveActivity(const QString & id);

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
     * @param id id of the activity
     */
    QString ActivityName(const QString & id) const;

    /**
     * Sets the name of the specified activity
     * @param id id of the activity
     * @param name name to be set
     */
    void SetActivityName(const QString & id, const QString & name);

    /**
     * @returns the description of the specified activity
     * @param id id of the activity
     */
    QString ActivityDescription(const QString & id) const;

    /**
     * Sets the description of the specified activity
     * @param id id of the activity
     * @param description description to be set
     */
    void SetActivityDescription(const QString & id, const QString & description);

    /**
     * @returns the icon of the specified activity
     * @param id id of the activity
     */
    QString ActivityIcon(const QString & id) const;

    /**
     * Sets the icon of the specified activity
     * @param id id of the activity
     * @param icon icon to be set
     */
    void SetActivityIcon(const QString & id, const QString & icon);


    /**
     * @returns whether the backstore (Nepomuk) is available
     */
    bool IsBackstoreAvailable() const;

Q_SIGNALS:
    /**
     * This signal is emitted when the global
     * activity is changed
     * @param id id of the new current activity
     */
    void CurrentActivityChanged(const QString & id);

    /**
     * This signal is emitted when a new activity is created
     * @param id id of the activity
     */
    void ActivityAdded(const QString & id);

    /**
     * This signal is emitted when an activity is started
     * @param id id of the activity
     */
    void ActivityStarted(const QString & id);

    /**
     * This signal is emitted when an activity is stashed
     * @param id id of the activity
     */
    void ActivityStopped(const QString & id);

    /**
     * This signal is emitted when an activity is deleted
     * @param id id of the activity
     */
    void ActivityRemoved(const QString & id);

    /**
     * Emitted when an activity name, description and/or icon are changed
     * @param id id of the changed activity
     */
    void ActivityChanged(const QString & id);

    /**
     * Emitted when the state of activity is changed
     */
    void ActivityStateChanged(const QString & id, int state);

// Resource related mothods
public Q_SLOTS:

    /**
     * Registers a new event
     * @param application the name of application that sent the event. Ignored if the event is not of type Opened
     * @param windowId ID of the window that displays the resource. Ignored if the event is of type Accessed
     * @param uri URI of the resource on which the event happened
     * @param event type of the event
     * @param reason reason for opening the resource
     */
    void RegisterResourceEvent(const QString & application, uint windowId, const QString & uri, uint event, uint reason);

    /**
     * Registers resource's mimetype. If not manually specified, it will
     * be retrieved if needed from Nepomuk
     *
     * Note that this will be forgotten when the resource in question is closed.
     * @param uri URI of the resource
     */
    void RegisterResourceMimeType(const QString & uri, const QString & mimetype);

    /**
     * Registers resource's title. If not manually specified, it will be a shortened
     * version of the uri
     *
     * Note that this will be forgotten when the resource in question is closed.
     * @param uri URI of the resource
     */
    void RegisterResourceTitle(const QString & uri, const QString & title);

    /**
     * Links the specified resource to the activity
     * @param uri URI of the resource
     * @param uri activity id of the activity to link to. If empty, the resource
     *     is linked to the current activity
     */
    void LinkResourceToActivity(const QString & uri, const QString & activity = QString());

    /**
     * Links the specified resource to the activity
     * @param uri URI of the resource
     * @param uri activity id of the activity to link to. If empty, the resource
     *     is linked to the current activity
     */
    // void UnlinkResourceFromActivity(const QString & uri, const QString & activity = QString());


private:
    friend class ActivityManagerPrivate;
    class ActivityManagerPrivate * const d;
};

#endif // ACTIVITY_MANAGER_H_
