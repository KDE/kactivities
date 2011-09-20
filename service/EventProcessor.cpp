/*
 *   Copyright (C) 2010 Ivan Cukic <ivan.cukic(at)kde.org>
 *   Copyright (c) 2010 Sebastian Trueg <trueg@kde.org>
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

#include "EventProcessor.h"
#include "Plugin.h"

#include "config-features.h"

#include "Plugin.h"

#include <KDebug>

#include <QDateTime>
#include <QList>
#include <QMutex>

#include <KDebug>
#include <KServiceTypeTrader>

#include <time.h>

class EventProcessorPrivate: public QThread {
public:
    QList < Plugin * > lazyBackends;
    QList < Plugin * > syncBackends;

    QList < Event > events;
    QMutex events_mutex;

    void run();

    static EventProcessor * s_instance;

};

EventProcessor * EventProcessorPrivate::s_instance = NULL;

void EventProcessorPrivate::run()
{
    kDebug() << "starting event processing...";

    forever {
        // initial delay before processing the events
        sleep(5); // do we need it?

        EventProcessorPrivate::events_mutex.lock();

        if (events.count() == 0) {
            kDebug() << "No more events to process, exiting.";

            EventProcessorPrivate::events_mutex.unlock();
            return;
        }

        QList < Event > currentEvents = events;
        events.clear();

        EventProcessorPrivate::events_mutex.unlock();

        kDebug() << "Passing the event to" << lazyBackends.size() << "lazy plugins";
        foreach (Plugin * backend, lazyBackends) {
            backend->addEvents(currentEvents);
        }
    }
}

EventProcessor * EventProcessor::self()
{
    if (!EventProcessorPrivate::s_instance) {
        EventProcessorPrivate::s_instance = new EventProcessor();
    }

    return EventProcessorPrivate::s_instance;
}

EventProcessor::EventProcessor()
    : d(new EventProcessorPrivate())
{
    // Initializing SharedInfo
    SharedInfo * shared = SharedInfo::self();

    connect(shared, SIGNAL(scoreUpdateRequested(const QString &, const QString &)),
            this, SLOT(updateScore(const QString &, const QString &)));

    // Plugin loading

    kDebug() << "Loading plugins...";

    KService::List offers = KServiceTypeTrader::self()->query("ActivityManager/Plugin");

    QStringList disabledPlugins = shared->pluginConfig("Global").readEntry("disabledPlugins", QStringList());
    kDebug() << disabledPlugins << "disabled due to the configuration in activitymanager-pluginsrc";

    foreach(const KService::Ptr & service, offers) {
        if (!disabledPlugins.contains(service->library())) {
            disabledPlugins.append(
                    service->property("X-ActivityManager-PluginOverrides", QVariant::StringList).toStringList()
                );
            kDebug() << service->name() << "disables" <<
                    service->property("X-ActivityManager-PluginOverrides", QVariant::StringList);

        }
    }

    foreach(const KService::Ptr & service, offers) {
        if (disabledPlugins.contains(service->library())) {
            continue;
        }

        kDebug() << "Loading plugin:"
            << service->name() << service->storageId() << service->library()
            << service->property("X-ActivityManager-PluginType", QVariant::String);

        KPluginFactory * factory = KPluginLoader(service->library()).factory();

        if (!factory) {
            kDebug() << "Failed to load plugin:" << service->name();
            continue;
        }

        Plugin * plugin = factory->create < Plugin > (this);

        if (plugin) {
            plugin->setSharedInfo(shared);
            plugin->init();

            const QString & type = service->property("X-ActivityManager-PluginType", QVariant::String).toString();

            if (type == "lazyeventhandler") {
                d->lazyBackends << plugin;
                kDebug() << "Added to lazy plugins";

            } else if (type == "synceventhandler"){
                d->syncBackends << plugin;
                kDebug() << "Added to sync plugins";

            }

        } else {
            kDebug() << "Failed to load plugin:" << service->name();
        }

    }
}

EventProcessor::~EventProcessor()
{
    qDeleteAll(d->lazyBackends);
    qDeleteAll(d->syncBackends);
    delete d;
}

void EventProcessor::addEvent(const QString & application, WId wid, const QString & uri,
            int type, int reason)
{
    Event newEvent(application, wid, uri, type, reason);

    foreach (Plugin * backend, d->syncBackends) {
        backend->addEvents(QList < Event > () << newEvent);
    }

    d->events_mutex.lock();

    if (newEvent.type != Event::Accessed) {
        foreach (const Event & event, d->events) {
            if (event.type == Event::Accessed && event.uri == uri
                    && event.application == application) {
                // Accessed events are of a lower priority
                // then the other ones
                if (type == Event::Accessed) {
                    d->events.removeAll(newEvent);
                }
            }
        }
    }

    d->events.append(newEvent);

    d->events_mutex.unlock();

    d->start();
}

void EventProcessor::updateScore(const QString & application, const QString & uri)
{
    // This is not a real event, it just notifies the system to recalculate
    // the score for the specified uri
    EventProcessor::self()->addEvent(application, 0, uri,
            Event::UpdateScore, Event::UserEventReason);

    kDebug() << "Score updating requested for" << application << uri;
}


