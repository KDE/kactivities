/*
 *   Copyright (C) 2012 Ivan Cukic <ivan.cukic(at)kde.org>
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

#ifndef RESOURCE_MANAGER_H_
#define RESOURCE_MANAGER_H_

#include <QString>
#include <QStringList>

#include <Module.h>
#include <Event.h>

#include <utils/d_ptr.h>
#include <utils/nullptr.h>

/**
 * Resources
 */
class Resources: public Module {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.ActivityManager.Resources")

public:
    Resources(QObject * parent = nullptr);
    virtual ~Resources();

public Q_SLOTS:
    /**
     * Registers a new event
     * @param application the name of application that sent the event. Ignored if the event is not of type Opened
     * @param windowId ID of the window that displays the resource. Ignored if the event is of type Accessed
     * @param uri URI of the resource on which the event happened
     * @param event type of the event
     * @param reason reason for opening the resource
     */
    void RegisterResourceEvent(QString application, uint windowId, const QString & uri, uint event, uint reason);

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
     * Unlinks the specified resource from the activity
     * @param uri URI of the resource
     * @param uri activity id of the activity to unlink from. If empty, the resource
     *     is unlinked from the current activity
     */
    void UnlinkResourceFromActivity(const QString & uri, const QString & activity = QString());

    /**
     * @returns whether the resource is linked to the activity
     * @param uri URI of the resource
     * @param uri activity id
     */
    bool IsResourceLinkedToActivity(const QString & uri, const QString & activity = QString()) const;

    /**
     * @returns the list of resources linked to the specified activity
     */
    QStringList ResourcesLinkedToActivity(const QString & activity = QString()) const;

Q_SIGNALS:
    void RegisteredResourceEvent(const Event & event);
    void ProcessedResourceEvents(const EventList & events);
    void RegisteredResourceMimeType(const QString & uri, const QString & mimetype);
    void RegisteredResourceTitle(const QString & uri, const QString & title);
    void LinkedResourceToActivity(const QString & uri, const QString & activity);
    void UnlinkedResourceFromActivity(const QString & uri, const QString & activity);

public:
    virtual bool isFeatureOperational(const QStringList & feature) const _override;
    virtual bool isFeatureEnabled(const QStringList & feature) const _override;
    virtual void setFeatureEnabled(const QStringList & feature, bool value) _override;
    virtual QStringList listFeatures(const QStringList & feature) const _override;

private:
    D_PTR;
};

#endif // RESOURCE_MANAGER_H_

