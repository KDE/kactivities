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

// Self
#include "ResourceScoreMaintainer.h"

// Qt
#include <QList>
#include <QMutex>
#include <QThread>

// System
#include <time.h>

// Utils
#include <utils/for_each_assoc.h>
#include <utils/d_ptr_implementation.h>

// Local
#include "StatsPlugin.h"
#include "ResourceScoreCache.h"


class ResourceScoreMaintainer::Private : public QThread {
public:
    typedef QString ApplicationName;
    typedef QString ActivityID;
    typedef QList<QString> ResourceList;

    typedef QMap<ApplicationName, ResourceList> Applications;
    typedef QMap<ActivityID, Applications> ResourceTree;

    ResourceTree openResources;
    QMutex openResources_mutex;

    void run();
    void processActivity(const ActivityID &activity,
                         const Applications &applications);
};

void ResourceScoreMaintainer::Private::run()
{
    forever
    {
        // initial delay before processing the resources
        sleep(5);

        ResourceTree resources;

        {
            QMutexLocker lock(&openResources_mutex);
            std::swap(resources, openResources);
        }

        const auto activity = StatsPlugin::self()->currentActivity();

        // Let us first process the events related to the current
        // activity so that the stats are available quicker

        if (resources.contains(activity)) {
            processActivity(activity, resources[activity]);
            resources.remove(activity);
        }

        kamd::utils::for_each_assoc(resources,
                                    [this](const ActivityID & activity,
                                           const Applications & applications) {
            processActivity(activity, applications);
        });
    }
}

void ResourceScoreMaintainer::Private::processActivity(const ActivityID
                                                       &activity,
                                                       const Applications
                                                       &applications)
{
    kamd::utils::for_each_assoc(applications,
                                [activity](const ApplicationName & application,
                                           const ResourceList & resources) {
        for (const auto &resource: resources)
        {
            ResourceScoreCache(activity, application, resource).updateScore();
        }
    });
}

ResourceScoreMaintainer *ResourceScoreMaintainer::self()
{
    static ResourceScoreMaintainer instance;
    return &instance;
}

ResourceScoreMaintainer::ResourceScoreMaintainer()
    : d()
{
}

ResourceScoreMaintainer::~ResourceScoreMaintainer()
{
}

void ResourceScoreMaintainer::processResource(const QString &resource, const QString &application)
{
    QMutexLocker lock(&d->openResources_mutex);

    // Checking whether the item is already scheduled for
    // processing

    const auto activity = StatsPlugin::self()->currentActivity();

    if (d->openResources.contains(activity) && d->openResources[activity].contains(application) && d->openResources[activity][application].contains(resource)) {

        // Nothing

    } else {
        d->openResources[activity][application] << resource;
    }

    d->start();
}
