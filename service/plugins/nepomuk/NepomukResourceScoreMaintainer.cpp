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

#include "NepomukResourceScoreMaintainer.h"

#include <QList>
#include <QMutex>
#include <QThread>

#include <KDebug>

#include <Nepomuk/Resource>

#include <time.h>
#include "kext.h"

#include "NepomukResourceScoreCache.h"
#include "NepomukCommon.h"

class NepomukResourceScoreMaintainerPrivate: public QThread {
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

    static NepomukResourceScoreMaintainer * s_instance;

};

NepomukResourceScoreMaintainer * NepomukResourceScoreMaintainerPrivate::s_instance = NULL;

void NepomukResourceScoreMaintainerPrivate::run()
{
    forever {
        // initial delay before processing the resources
        sleep(5);

        NepomukResourceScoreMaintainerPrivate::openResources_mutex.lock();
            ResourceTree resources = openResources;
            openResources.clear();
        NepomukResourceScoreMaintainerPrivate::openResources_mutex.unlock();

        const QString & activity = currentActivityId;

        // Let us first process the events related to the current
        // activity so that the stats are available quicker

        if (resources.contains(activity)) {
            kDebug() << "Processing current activity events";

            processActivity(activity, resources[activity]);
            resources.remove(activity);
        }

        foreach (const ActivityID & activity, resources.keys()) {
            kDebug() << "Processing activity" << activity;

            processActivity(activity, resources[activity]);
        }
    }
}

void NepomukResourceScoreMaintainerPrivate::processActivity(const ActivityID & activity, const Applications & applications)
{
    foreach (const ApplicationName & application, applications.keys()) {
        // Processing resources for the pair (activity, application)
        kDebug() << "  Processing application" << application;

        foreach (const QUrl & resource, applications[application]) {
            kDebug() << "    Updating score for" << activity << application << resource;
            NepomukResourceScoreCache(activity, application, resource).updateScore();

        }
    }
}

NepomukResourceScoreMaintainer * NepomukResourceScoreMaintainer::self()
{
    if (!NepomukResourceScoreMaintainerPrivate::s_instance) {
        NepomukResourceScoreMaintainerPrivate::s_instance = new NepomukResourceScoreMaintainer();
    }

    return NepomukResourceScoreMaintainerPrivate::s_instance;
}

NepomukResourceScoreMaintainer::NepomukResourceScoreMaintainer()
    : d(new NepomukResourceScoreMaintainerPrivate())
{
}

NepomukResourceScoreMaintainer::~NepomukResourceScoreMaintainer()
{
    delete d;
}

void NepomukResourceScoreMaintainer::processResource(const KUrl & resource, const QString & application)
{
    d->openResources_mutex.lock();

    // Checking whether the item is already scheduled for
    // processing

    kDebug() << "Adding" << resource << application << "to the queue";

    const QString & activity = currentActivityId;

    if (d->openResources.contains(activity) &&
            d->openResources[activity].contains(application) &&
            d->openResources[activity][application].contains(resource)) {
        return;
    }

    d->openResources[activity][application] << resource;

    d->openResources_mutex.unlock();

    d->start();
}


