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

#include "Resources.h"
#include "Resources_p.h"
#include "resourcesadaptor.h"

#include <QDBusConnection>
#include <QThread>
#include <QMutex>
#include <QMutexLocker>
#include <QDebug>

#include <KUrl>
#include <KWindowSystem>
#include <kdbusconnectionpool.h>

#include <Application.h>
#include <Activities.h>

#include <time.h>

#include "common.h"

#include <utils/d_ptr_implementation.h>
#include <utils/remove_if.h>

Resources::Private::Private(Resources * parent)
    : QThread(parent), focussedWindow(0), q(parent)
{
}

namespace {
    EventList events;
    QMutex events_mutex;
}

void Resources::Private::run()
{
    forever {
        // initial delay before processing the events
        sleep(5);

        EventList currentEvents;

        {
            QMutexLocker locker(& events_mutex);

            if (events.count() == 0) {
                // qDebug() << "No more events to process, exiting.";
                return;
            }

            currentEvents = events;
            events.clear();
        }

        emit q->ProcessedResourceEvents(currentEvents);
    }
}

void Resources::Private::insertEvent(const Event & newEvent)
{
    if (lastEvent == newEvent) return;
    lastEvent = newEvent;

    {
        QMutexLocker locker(& events_mutex);
        events << newEvent;
    }

    emit q->RegisteredResourceEvent(newEvent);
}

void Resources::Private::addEvent(const QString & application, WId wid, const QString & uri,
            int type, int reason)
{
    Event newEvent(application, wid, uri, type, reason);
    addEvent(newEvent);
}

void Resources::Private::addEvent(const Event & newEvent)
{
    // And now, for something completely delayed
    {
        QMutexLocker locker(& events_mutex);

        // Deleting previously registered Accessed events if
        // the current one has the same application and uri
        if (newEvent.type != Event::Accessed) {
            kamd::utils::remove_if(events, [&newEvent] (const Event & event) -> bool {
                return
                    event.reason      == Event::Accessed      &&
                    event.application == newEvent.application &&
                    event.uri         == newEvent.uri
                ;
            });
        }
    }

    // Process the windowing
    // Essentially, this is the brain of SLC. We need to track the
    // window focus changes to be able to generate the potential
    // missing events like FocussedOut before Closed and similar.
    // So, there is no point in having the same logic in SLC plugin
    // as well.

    if (newEvent.wid != 0) {
        WindowData & data = windows[newEvent.wid];
        const KUrl & kuri(newEvent.uri);

        qDebug() << kuri << data.focussedResource;

        data.application = newEvent.application;

        switch (newEvent.type) {
            case Event::Opened:
                insertEvent(newEvent);

                if (data.focussedResource.isEmpty()) {
                    // This window haven't had anything focused,
                    // assuming the new document is focused

                    data.focussedResource = newEvent.uri;
                    insertEvent(newEvent.deriveWithType(Event::FocussedIn));
                }

                break;

            case Event::FocussedIn:

                if (!data.resources.contains(kuri)) {
                    // This window did not contain this resource before,
                    // sending Opened event

                    insertEvent(newEvent.deriveWithType(Event::Opened));
                }

                data.focussedResource = newEvent.uri;
                insertEvent(newEvent);

                break;

            case Event::Closed:

                qDebug() << data.focussedResource << kuri;

                if (data.focussedResource == kuri) {
                    // If we are closing a document that is in focus,
                    // release focus first

                    insertEvent(newEvent.deriveWithType(Event::FocussedOut));
                    data.focussedResource.clear();
                }

                insertEvent(newEvent);

                break;

            case Event::FocussedOut:

                if (data.focussedResource == kuri) {
                    data.focussedResource.clear();
                }

                insertEvent(newEvent);

                break;

            default:
                insertEvent(newEvent);
                break;

        }
    }

    start();
}

void Resources::Private::windowClosed(WId windowId)
{
    // Testing whether the window is a registered one

    if (!windows.contains(windowId)) {
        return;
    }

    if (focussedWindow == windowId) {
        focussedWindow = 0;
    }

    // Closing all the resources that the window registered

    foreach (const KUrl & uri, windows[windowId].resources) {
        q->RegisterResourceEvent(windows[windowId].application,
                toInt(windowId), uri.url(), Event::Closed, 0);
    }

    windows.remove(windowId);
}

void Resources::Private::activeWindowChanged(WId windowId)
{
    // If the focused window has changed, we need to create a
    // FocussedOut event for the resource it contains,
    // and FocussedIn for the resource of the new active window.
    // The windows can do this manually, but if they are
    // SDI, we can do it on our own.

    if (windowId == focussedWindow) return;

    if (windows.contains(focussedWindow)) {
        const WindowData & data = windows[focussedWindow];

        if (!data.focussedResource.isEmpty()) {
            insertEvent(Event(data.application, focussedWindow, data.focussedResource.url(), Event::FocussedOut));
        }
    }

    focussedWindow = windowId;

    if (windows.contains(focussedWindow)) {
        const WindowData & data = windows[focussedWindow];

        if (!data.focussedResource.isEmpty()) {
            insertEvent(Event(data.application, windowId, data.focussedResource.url(), Event::FocussedIn));
        }
    }
}


Resources::Resources(QObject * parent)
    : Module("resources", parent), d(this)
{
    qRegisterMetaType < Event > ("Event");
    qRegisterMetaType < EventList > ("EventList");
    qRegisterMetaType < WId > ("WId");

    new ResourcesAdaptor(this);
    KDBusConnectionPool::threadConnection().registerObject(
            ACTIVITY_MANAGER_OBJECT_PATH(Resources), this);

    d->connect(KWindowSystem::self(), SIGNAL(windowRemoved(WId)),
            SLOT(windowClosed(WId)));
    d->connect(KWindowSystem::self(), SIGNAL(activeWindowChanged(WId)),
            SLOT(activeWindowChanged(WId)));

}

Resources::~Resources()
{
}

void Resources::RegisterResourceEvent(QString application, uint _windowId,
        const QString & uri, uint event, uint reason)
{
    Q_ASSERT_X(!uri.startsWith("nepomuk:"), "Resources::RegisterResourceEvent",
            "We do not accept nepomuk URIs for resource events");

    if (
           event > Event::LastEventType
        || reason > Event::LastEventReason
        || uri.isEmpty()
        || application.isEmpty()
        // Dirty way to skip special web browser URIs
        // This is up to the plugin - whether it wants it filtered out or not
        // || uri.startsWith(QLatin1String("about:"))
    ) return;

    KUrl kuri(uri);
    WId windowId = (WId) _windowId;

    d->addEvent(application, windowId,
            kuri.url(), (Event::Type) event, (Event::Reason) reason);
}


void Resources::RegisterResourceMimeType(const QString & uri, const QString & mimetype)
{
    if (!mimetype.isEmpty()) return;

    KUrl kuri(uri);

    emit RegisteredResourceMimeType(uri, mimetype);
}


void Resources::RegisterResourceTitle(const QString & uri, const QString & title)
{
    // A dirty saninty check for the title
    if (title.length() < 3) return;

    KUrl kuri(uri);

    emit RegisteredResourceTitle(uri, title);
}

bool Resources::isFeatureOperational(const QStringList & feature) const
{
    Q_UNUSED(feature)
    return false;
}

bool Resources::isFeatureEnabled(const QStringList & feature) const
{
    Q_UNUSED(feature)
    return false;

}

void Resources::setFeatureEnabled(const QStringList & feature, bool value)
{
    Q_UNUSED(feature)
    Q_UNUSED(value)
}

QStringList Resources::listFeatures(const QStringList & feature) const
{
    Q_UNUSED(feature)
    static QStringList features;

    return features;
}
