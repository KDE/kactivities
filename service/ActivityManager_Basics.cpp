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

#include "ActivityManager.h"
#include "ActivityManager_p.h"
#include "NepomukActivityManager.h"

#include "jobs/encryption/Common.h"

KConfigGroup ActivityManagerPrivate::activityIconsConfig()
{
    return KConfigGroup(&config, "activities-icons");
}

KConfigGroup ActivityManagerPrivate::activitiesConfig()
{
    return KConfigGroup(&config, "activities");
}

KConfigGroup ActivityManagerPrivate::mainConfig()
{
    return KConfigGroup(&config, "main");
}

KConfigGroup ActivityManagerPrivate::activitiesDesktopsConfig()
{
    return KConfigGroup(&config, "activitiesDesktops");
}

QString ActivityManagerPrivate::activityName(const QString & id)
{
    return activitiesConfig().readEntry(id, QString());
}

QString ActivityManagerPrivate::activityIcon(const QString & id)
{
    return activityIconsConfig().readEntry(id, QString());
}

void ActivityManagerPrivate::scheduleConfigSync(const bool shortInterval)
{
#define SHORT_INTERVAL (5 * 1000)
#define LONG_INTERVAL (2 * 60 * 1000)

    if (shortInterval) {
        if (configSyncTimer.interval() != SHORT_INTERVAL) {
            // always change to SHORT_INTERVAL if the current one is LONG_INTERVAL.
            configSyncTimer.stop();
            configSyncTimer.setInterval(SHORT_INTERVAL);
        }
    } else if (configSyncTimer.interval() != LONG_INTERVAL && !configSyncTimer.isActive()) {
        configSyncTimer.setInterval(LONG_INTERVAL);
    }

    if (!configSyncTimer.isActive()) {
        configSyncTimer.start();
    }
#undef SHORT_INTERVAL
#undef LONG_INTERVAL
}

void ActivityManagerPrivate::configSync()
{
    configSyncTimer.stop();
    config.sync();
}

void ActivityManager::Start()
{
    // doing absolutely nothing
}

void ActivityManager::Stop()
{
    d->configSync();
    QCoreApplication::quit();
}

bool ActivityManager::IsFeatureOperational(const QString & feature) const
{
    if (feature == "activity/resource-linking") {
        return NEPOMUK_PRESENT;
    }

    if (feature == "activity/encryption") {
        return Jobs::Encryption::Common::isEnabled();
    }

    return false;
}

QStringList ActivityManager::ListActivities() const
{
    return d->activities.keys();
}

QStringList ActivityManager::ListActivities(int state) const
{
    return d->activities.keys((State)state);
}

QString ActivityManager::ActivityName(const QString & id) const
{
    return d->activityName(id);
}

void ActivityManager::SetActivityName(const QString & id, const QString & name)
{
    if (!d->activities.contains(id)) return;

    d->activitiesConfig().writeEntry(id, name);

    EXEC_NEPOMUK( setActivityName(id, name) );

    d->scheduleConfigSync();
    emit ActivityChanged(id);
}

QString ActivityManager::ActivityDescription(const QString & id) const
{
    // This is not used anyway
    Q_UNUSED(id)
    return QString();
}

void ActivityManager::SetActivityDescription(const QString & id, const QString & description)
{
    // This is not used anyway
    Q_UNUSED(id)
    Q_UNUSED(description)
}

QString ActivityManager::ActivityIcon(const QString & id) const
{
    return d->activityIcon(id);
}

void ActivityManager::SetActivityIcon(const QString & id, const QString & icon)
{
    if (!d->activities.contains(id)) return;

    d->activityIconsConfig().writeEntry(id, icon);

    EXEC_NEPOMUK( setActivityIcon(id, icon) );

    d->scheduleConfigSync();
    emit ActivityChanged(id);
}

// static
ActivityManager * ActivityManager::self()
{
    return static_cast<ActivityManager*>(kapp);
}

