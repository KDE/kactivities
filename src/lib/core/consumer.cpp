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

#include "consumer.h"
#include "consumer_p.h"
#include "manager_p.h"

#include "debug_p.h"

namespace KActivities {

ConsumerPrivate::ConsumerPrivate()
    : cache(ActivitiesCache::self())
{
}

void ConsumerPrivate::setServiceStatus(Consumer::ServiceStatus status)
{
    emit serviceStatusChanged(status);
}

Consumer::Consumer(QObject *parent)
    : QObject(parent)
    , d(new ConsumerPrivate())
{
    qCDebug(KAMD_CORELIB) << "Creating a consumer";

    connect(d->cache.get(), SIGNAL(currentActivityChanged(QString)),
            this, SIGNAL(currentActivityChanged(QString)));
    connect(d->cache.get(), SIGNAL(activityAdded(QString)),
            this, SIGNAL(activityAdded(QString)));
    connect(d->cache.get(), SIGNAL(activityRemoved(QString)),
            this, SIGNAL(activityRemoved(QString)));
    connect(d->cache.get(), SIGNAL(serviceStatusChanged(Consumer::ServiceStatus)),
            this, SIGNAL(serviceStatusChanged(Consumer::ServiceStatus)));

    connect(d->cache.get(), &ActivitiesCache::activityListChanged,
            this, [=]() { emit activitiesChanged(activities()); });

    connect(d->cache.get(), SIGNAL(serviceStatusChanged(Consumer::ServiceStatus)),
            this, SIGNAL(serviceStatusChanged(Consumer::ServiceStatus)));

    // connect(d->cache.get(), SIGNAL(activityStateChanged(QString,int)),
    //         this, SIGNAL(activityStateChanged(QString,int)));
}

Consumer::~Consumer()
{
    qCDebug(KAMD_CORELIB) << "Killing the consumer";
}

QString Consumer::currentActivity() const
{
    return d->cache->m_currentActivity;
}

QStringList Consumer::activities(Info::State state) const
{
    QStringList result;

    result.reserve(d->cache->m_activities.size());

    foreach (const auto & info, d->cache->m_activities) {
        if (info.state == state)
            result << info.id;
    }

    return result;
}

QStringList Consumer::activities() const
{
    QStringList result;

    result.reserve(d->cache->m_activities.size());

    foreach (const auto & info, d->cache->m_activities) {
        result << info.id;
    }

    return result;
}

Consumer::ServiceStatus Consumer::serviceStatus()
{
    return d->cache->m_status;
}

} // namespace KActivities
