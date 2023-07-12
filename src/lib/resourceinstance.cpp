/*
    SPDX-FileCopyrightText: 2011-2016 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "resourceinstance.h"
#include "manager_p.h"

#include "debug_p.h"
#include <QCoreApplication>
#include <QWindow>

namespace KActivities
{
class ResourceInstancePrivate
{
public:
    QWindow *window = nullptr;
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
        FocusedOut = 5,
    };

    static void registerResourceEvent(const QString &application, QWindow *window, const QUrl &uri, Type event)
    {
        Q_ASSERT_X(!application.isEmpty(), "ResourceInstance::event", "The application id must not be empty");

        if (uri.isEmpty()) {
            return;
        }

        Manager::resources()->RegisterResourceEvent(application, window ? window->winId() : 0, uri.toString(), uint(event));
    }
};

void ResourceInstancePrivate::closeResource()
{
    registerResourceEvent(application, window, uri, Closed);
}

void ResourceInstancePrivate::openResource()
{
    registerResourceEvent(application, window, uri, Opened);
}

ResourceInstance::ResourceInstance(QWindow *window, QObject *parent)
    : QObject(parent)
    , d(new ResourceInstancePrivate())
{
    qCDebug(KAMD_CORELIB) << "Creating ResourceInstance: empty for now";
    d->window = window;
    d->application = QCoreApplication::instance()->applicationName();
}

ResourceInstance::ResourceInstance(QWindow *window, const QString &application, QObject *parent)
    : QObject(parent)
    , d(new ResourceInstancePrivate())
{
    qCDebug(KAMD_CORELIB) << "Creating ResourceInstance: empty for now";
    d->window = window;
    d->application = application.isEmpty() ? QCoreApplication::instance()->applicationName() : application;
}

ResourceInstance::ResourceInstance(QWindow *window,
                                   QUrl resourceUri,
                                   const QString &mimetype,
                                   const QString &title,
                                   const QString &application,
                                   QObject *parent)
    : QObject(parent)
    , d(new ResourceInstancePrivate())
{
    qCDebug(KAMD_CORELIB) << "Creating ResourceInstance:" << resourceUri;
    d->window = window;
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
    d->registerResourceEvent(d->application, d->window, d->uri, ResourceInstancePrivate::Modified);
}

void ResourceInstance::notifyFocusedIn()
{
    d->registerResourceEvent(d->application, d->window, d->uri, ResourceInstancePrivate::FocusedIn);
}

void ResourceInstance::notifyFocusedOut()
{
    d->registerResourceEvent(d->application, d->window, d->uri, ResourceInstancePrivate::FocusedOut);
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
    qCDebug(KAMD_CORELIB) << "Setting the title:" << title;
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

QWindow *ResourceInstance::window() const
{
    return d->window;
}

void ResourceInstance::notifyAccessed(const QUrl &uri, const QString &application)
{
    ResourceInstancePrivate::registerResourceEvent(application.isEmpty() ? QCoreApplication::instance()->applicationName() : application,
                                                   nullptr,
                                                   uri,
                                                   ResourceInstancePrivate::Accessed);
}

} // namespace KActivities

#include "moc_resourceinstance.cpp"
