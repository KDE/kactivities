/*
 *   Copyright (C) 2011 - 2016 by Ivan Cukic <ivan.cukic(at)kde.org>
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

#include "resourceinstance.h"
#include "manager_p.h"

#include <QCoreApplication>
#include "debug_p.h"

namespace KActivities {

class ResourceInstancePrivate {
public:
    quintptr wid;
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

    static void registerResourceEvent(const QString &application, quintptr wid, const QUrl &uri, Type event)
    {
        Q_ASSERT_X(!application.isEmpty(), "ResourceInstance::event",
                   "The application id must not be empty");

        if (uri.isEmpty()) {
            return;
        }

        Manager::resources()->RegisterResourceEvent(application, wid, uri.toString(), uint(event));
    }
};

void ResourceInstancePrivate::closeResource()
{
    registerResourceEvent(application, wid, uri, Closed);
}

void ResourceInstancePrivate::openResource()
{
    registerResourceEvent(application, wid, uri, Opened);
}

ResourceInstance::ResourceInstance(quintptr wid, QObject *parent)
    : QObject(parent)
    , d(new ResourceInstancePrivate())
{
    qCDebug(KAMD_CORELIB) << "Creating ResourceInstance: empty for now";
    d->wid = wid;
    d->application = QCoreApplication::instance()->applicationName();
}

ResourceInstance::ResourceInstance(quintptr wid, const QString &application, QObject *parent)
    : QObject(parent)
    , d(new ResourceInstancePrivate())
{
    qCDebug(KAMD_CORELIB) << "Creating ResourceInstance: empty for now";
    d->wid = wid;
    d->application = application.isEmpty() ? QCoreApplication::instance()->applicationName() : application;
}

ResourceInstance::ResourceInstance(quintptr wid, QUrl resourceUri, const QString &mimetype,
                                   const QString &title, const QString &application, QObject *parent)
    : QObject(parent)
    , d(new ResourceInstancePrivate())
{
    qCDebug(KAMD_CORELIB) << "Creating ResourceInstance: " << resourceUri;
    d->wid = wid;
    d->uri = resourceUri.adjusted(QUrl::StripTrailingSlash);
    d->application = application.isEmpty() ? QCoreApplication::instance()->applicationName() : application;

    d->openResource();

    setTitle(title);
    setMimetype(mimetype);
}

ResourceInstance::~ResourceInstance()
{
    d->closeResource();
}

void ResourceInstance::notifyModified()
{
    d->registerResourceEvent(d->application, d->wid, d->uri, ResourceInstancePrivate::Modified);
}

void ResourceInstance::notifyFocusedIn()
{
    d->registerResourceEvent(d->application, d->wid, d->uri, ResourceInstancePrivate::FocusedIn);
}

void ResourceInstance::notifyFocusedOut()
{
    d->registerResourceEvent(d->application, d->wid, d->uri, ResourceInstancePrivate::FocusedOut);
}

void ResourceInstance::setUri(const QUrl &newUri)
{
    if (d->uri == newUri) {
        return;
    }

    if (!d->uri.isEmpty()) {
        d->closeResource();
    }

    d->uri = newUri.adjusted(QUrl::StripTrailingSlash);

    d->openResource();
}

void ResourceInstance::setMimetype(const QString &mimetype)
{
    if (mimetype.isEmpty()) {
        return;
    }

    d->mimetype = mimetype;
    // TODO: update the service info
    Manager::resources()->RegisterResourceMimetype(d->uri.toString(), mimetype);
}

void ResourceInstance::setTitle(const QString &title)
{
    qCDebug(KAMD_CORELIB) << "Setting the title: " << title;
    if (title.isEmpty()) {
        return;
    }

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

void ResourceInstance::notifyAccessed(const QUrl &uri, const QString &application)
{
    ResourceInstancePrivate::registerResourceEvent(
        application.isEmpty() ? QCoreApplication::instance()->applicationName() : application,
        0, uri, ResourceInstancePrivate::Accessed);
}

} // namespace KActivities
