/*
 *   Copyright (C) 2011 Ivan Cukic <ivan.cukic(at)kde.org>
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

#include "ResourceScoreMaintainer.h"

#include <QList>
#include <QMutex>
#include <QThread>

#include <time.h>

#include "StatsPlugin.h"
#include "ResourceScoreCache.h"

class ResourceScoreMaintainerPrivate: public QThread {
public:
    typedef QString ApplicationName;
    typedef QString ActivityID;
    typedef QList < QUrl > ResourceList;

    typedef QMap < ApplicationName, ResourceList > Applications;
    typedef QMap < ActivityID, Applications > ResourceTree;

    ResourceTree openResources;
    QMutex openResources_mutex;

    void run();
    void processActivity(const ActivityID & activity, const Applications & applications);

    static ResourceScoreMaintainer * s_instance;

};

ResourceScoreMaintainer * ResourceScoreMaintainerPrivate::s_instance = NULL;

void ResourceScoreMaintainerPrivate::run()
{
    forever {
        // initial delay before processing the resources
        sleep(5);

        ResourceScoreMaintainerPrivate::openResources_mutex.lock();
            ResourceTree resources = openResources;
            openResources.clear();
        ResourceScoreMaintainerPrivate::openResources_mutex.unlock();

        const QString & activity = StatsPlugin::self()->sharedInfo()->currentActivity();

        // Let us first process the events related to the current
        // activity so that the stats are available quicker

        if (resources.contains(activity)) {
            processActivity(activity, resources[activity]);
            resources.remove(activity);
        }

        foreach (const ActivityID & activity, resources.keys()) {
            processActivity(activity, resources[activity]);
        }
    }
}

void ResourceScoreMaintainerPrivate::processActivity(const ActivityID & activity, const Applications & applications)
{
    foreach (const ApplicationName & application, applications.keys()) {
        // Processing resources for the pair (activity, application)

        foreach (const QUrl & resource, applications[application]) {
            ResourceScoreCache(activity, application, resource).updateScore();

        }
    }
}

ResourceScoreMaintainer * ResourceScoreMaintainer::self()
{
    if (!ResourceScoreMaintainerPrivate::s_instance) {
        ResourceScoreMaintainerPrivate::s_instance = new ResourceScoreMaintainer();
    }

    return ResourceScoreMaintainerPrivate::s_instance;
}

ResourceScoreMaintainer::ResourceScoreMaintainer()
    : d(new ResourceScoreMaintainerPrivate())
{
}

ResourceScoreMaintainer::~ResourceScoreMaintainer()
{
    delete d;
}

void ResourceScoreMaintainer::processResource(const KUrl & resource, const QString & application)
{
    d->openResources_mutex.lock();

    // Checking whether the item is already scheduled for
    // processing

    const QString & activity = StatsPlugin::self()->sharedInfo()->currentActivity();

    if (d->openResources.contains(activity) &&
            d->openResources[activity].contains(application) &&
            d->openResources[activity][application].contains(resource)) {

        // Nothing

    } else {
        d->openResources[activity][application] << resource;

    }

    d->openResources_mutex.unlock();

    d->start();
}


