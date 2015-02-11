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

#ifndef RESOURCES_H
#define RESOURCES_H

// Qt
#include <QString>
#include <QStringList>

// Utils
#include <utils/d_ptr.h>

// Local
#include "Module.h"
#include "Event.h"


/**
 * Resources
 */
class Resources : public Module {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.ActivityManager.Resources")

public:
    Resources(QObject *parent = Q_NULLPTR);
    virtual ~Resources();

public Q_SLOTS:
    /**
     * Registers a new event
     * @param application the name of application that sent the event. Ignored
     *                    if the event is not of type Opened
     * @param windowId ID of the window that displays the resource. Ignored if
     *                 the event is of type Accessed
     * @param uri URI of the resource on which the event happened
     * @param event type of the event
     */
    void RegisterResourceEvent(QString application, uint windowId,
                               const QString &uri, uint event);

    /**
     * Registers resource's mimetype.
     * Note that this will be forgotten when the resource in question is closed.
     * @param uri URI of the resource
     */
    void RegisterResourceMimetype(const QString &uri, const QString &mimetype);

    /**
     * Registers resource's title. If not manually specified, it will be a
     * shortened version of the uri
     *
     * Note that this will be forgotten when the resource in question is closed.
     * @param uri URI of the resource
     */
    void RegisterResourceTitle(const QString &uri, const QString &title);

Q_SIGNALS:
    void RegisteredResourceEvent(const Event &event);
    void ProcessedResourceEvents(const EventList &events);
    void RegisteredResourceMimetype(const QString &uri, const QString &mimetype);
    void RegisteredResourceTitle(const QString &uri, const QString &title);

public:
    bool isFeatureOperational(const QStringList &feature) const Q_DECL_OVERRIDE;
    bool isFeatureEnabled(const QStringList &feature) const Q_DECL_OVERRIDE;
    void setFeatureEnabled(const QStringList &feature, bool value) Q_DECL_OVERRIDE;
    QStringList listFeatures(const QStringList &feature) const Q_DECL_OVERRIDE;

private:
    D_PTR;
};

#endif // RESOURCES_H
