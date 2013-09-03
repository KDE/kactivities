/*
 *   Copyright (C) 2011, 2012 Ivan Cukic <ivan.cukic(at)kde.org>
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

#include "resourceinstance.h"
#include "manager_p.h"

#include <QCoreApplication>
#include <QDebug>

namespace KActivities {

class ResourceInstancePrivate {
public:
    quintptr wid;
    ResourceInstance::AccessReason reason;
    QUrl uri;
    QString mimetype;
    QString title;
    QString application;

    void closeResource();
    void openResource();

    enum Type {
        Accessed = 0,
        Opened = 1,
        Modified = 2,
        Closed = 3,
        FocusedIn = 4,
        FocusedOut = 5
    };

    static void registerResourceEvent(const QString &application, quintptr wid, const QUrl &uri, Type event, ResourceInstance::AccessReason reason)
    {
        Manager::resources()->RegisterResourceEvent(application, wid, uri.toString(), uint(event), uint(reason));
    }
};

void ResourceInstancePrivate::closeResource()
{
    registerResourceEvent(application, wid, uri, Closed, reason);
}

void ResourceInstancePrivate::openResource()
{
    registerResourceEvent(application, wid, uri, Opened, reason);
}

ResourceInstance::ResourceInstance(quintptr wid, QObject *parent)
    : QObject(parent), d(new ResourceInstancePrivate())
{
    qDebug() << "Creating ResourceInstance: empty for now";
    d->wid = wid;
    d->reason = User;
    d->application = QCoreApplication::instance()->applicationName();
}

ResourceInstance::ResourceInstance(quintptr wid, AccessReason reason, const QString &application, QObject *parent)
    : QObject(parent), d(new ResourceInstancePrivate())
{
    qDebug() << "Creating ResourceInstance: empty for now";
    d->wid = wid;
    d->reason = reason;
    d->application = application.isEmpty() ? QCoreApplication::instance()->applicationName() : application;
}

ResourceInstance::ResourceInstance(quintptr wid, QUrl resourceUri, const QString &mimetype,
        const QString &title, AccessReason reason, const QString &application, QObject *parent)
    : QObject(parent), d(new ResourceInstancePrivate())
{
    qDebug() << "Creating ResourceInstance: " << resourceUri;
    d->wid = wid;
    d->reason = reason;
    d->uri = resourceUri;
    d->application = application.isEmpty() ? QCoreApplication::instance()->applicationName() : application;

    d->openResource();

    setTitle(title);
    setMimetype(mimetype);
}

ResourceInstance::~ResourceInstance()
{
    d->closeResource();
    delete d;
}

void ResourceInstance::notifyModified()
{
    d->registerResourceEvent(d->application, d->wid, d->uri, ResourceInstancePrivate::Modified, d->reason);
}

void ResourceInstance::notifyFocusedIn()
{
    d->registerResourceEvent(d->application, d->wid, d->uri, ResourceInstancePrivate::FocusedIn, d->reason);
}

void ResourceInstance::notifyFocusedOut()
{
    d->registerResourceEvent(d->application, d->wid, d->uri, ResourceInstancePrivate::FocusedOut, d->reason);
}

void ResourceInstance::setUri(const QUrl &newUri)
{
    if (d->uri == newUri)
        return;

    if (!d->uri.isEmpty()) {
        d->closeResource();
    }

    d->uri = newUri;

    d->openResource();
}

void ResourceInstance::setMimetype(const QString &mimetype)
{
    if (mimetype.isEmpty()) return;

    d->mimetype = mimetype;
    // TODO: update the service info
    Manager::resources()->RegisterResourceMimeType(d->uri.toString(), mimetype);
}

void ResourceInstance::setTitle(const QString &title)
{
    qDebug() << "Setting the title: " << title;
    if (title.isEmpty()) return;

    d->title = title;
    // TODO: update the service info
    Manager::resources()->RegisterResourceTitle(d->uri.toString(), title);
}

QUrl ResourceInstance::uri() const
{
    return d->uri;
}

QString ResourceInstance::mimetype() const
{
    return d->mimetype;
}

QString ResourceInstance::title() const
{
    return d->title;
}

quintptr ResourceInstance::winId() const
{
    return d->wid;
}

ResourceInstance::AccessReason ResourceInstance::accessReason() const
{
    return d->reason;
}

void ResourceInstance::notifyAccessed(const QUrl &uri, const QString &application)
{
    ResourceInstancePrivate::registerResourceEvent(
            application.isEmpty() ? QCoreApplication::instance()->applicationName() : application,
            0, uri, ResourceInstancePrivate::Accessed, User);
}

} // namespace KActivities
