/*
 *   Copyright (C) 2011, 2012, 2013, 2014, 2015 Ivan Cukic <ivan.cukic(at)kde.org>
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
#include <kactivities-features.h>
#include "ResourceLinking.h"

// Qt
#include <QFileSystemWatcher>
#include <QSqlQuery>

// KDE
#include <kconfig.h>
#include <kdbusconnectionpool.h>
#include <kdirnotify.h>

// Boost
#include <boost/range/algorithm/binary_search.hpp>
#include <utils/range.h>

// Local
#include "Debug.h"
#include "Database.h"
#include "Utils.h"
#include "StatsPlugin.h"
#include "resourcelinkingadaptor.h"

ResourceLinking::ResourceLinking(QObject *parent)
    : QObject(parent)
{
    new ResourcesLinkingAdaptor(this);
    KDBusConnectionPool::threadConnection().registerObject(
        QStringLiteral("/ActivityManager/Resources/Linking"), this);
}

void ResourceLinking::init()
{
    auto activities = StatsPlugin::self()->activitiesInterface();

    connect(activities, SIGNAL(CurrentActivityChanged(QString)),
            this, SLOT(onCurrentActivityChanged(QString)));
    connect(activities, SIGNAL(ActivityAdded(QString)),
            this, SLOT(onActivityAdded(QString)));
    connect(activities, SIGNAL(ActivityRemoved(QString)),
            this, SLOT(onActivityRemoved(QString)));
}

void ResourceLinking::LinkResourceToActivity(QString initiatingAgent,
                                             QString targettedResource,
                                             QString usedActivity)
{
    // qDebug() << "Linking " << targettedResource << " to " << usedActivity << " from " << initiatingAgent;

    if (!validateArguments(initiatingAgent, targettedResource, usedActivity)) {
        qWarning() << "Invalid arguments" << initiatingAgent
                   << targettedResource << usedActivity;
        return;
    }

    Q_ASSERT_X(!initiatingAgent.isEmpty(),
               "ResourceLinking::LinkResourceToActivity",
               "Agent shoud not be empty");
    Q_ASSERT_X(!usedActivity.isEmpty(),
               "ResourceLinking::LinkResourceToActivity",
               "Activity shoud not be empty");
    Q_ASSERT_X(!targettedResource.isEmpty(),
               "ResourceLinking::LinkResourceToActivity",
               "Resource shoud not be empty");

    Utils::prepare(resourcesDatabase(), linkResourceToActivityQuery,
        QStringLiteral(
            "INSERT OR REPLACE INTO ResourceLink"
            "        (usedActivity,  initiatingAgent,  targettedResource) "
            "VALUES ( "
                "COALESCE(:usedActivity,''),"
                "COALESCE(:initiatingAgent,''),"
                "COALESCE(:targettedResource,'')"
            ")"
        ));

    DATABASE_TRANSACTION(resourcesDatabase());

    Utils::exec(Utils::FailOnError, *linkResourceToActivityQuery,
        ":usedActivity"      , usedActivity,
        ":initiatingAgent"   , initiatingAgent,
        ":targettedResource" , targettedResource
    );

    if (!usedActivity.isEmpty()) {
        // qDebug() << "Sending link event added: activities:/" << usedActivity;
        org::kde::KDirNotify::emitFilesAdded(QStringLiteral("activities:/")
                                             + usedActivity);

        if (usedActivity == StatsPlugin::self()->currentActivity()) {
            // qDebug() << "Sending link event added: activities:/current";
            org::kde::KDirNotify::emitFilesAdded(
                QStringLiteral("activities:/current"));
        }
    }

    emit ResourceLinkedToActivity(initiatingAgent, targettedResource,
                                  usedActivity);
}

void ResourceLinking::UnlinkResourceFromActivity(QString initiatingAgent,
                                                 QString targettedResource,
                                                 QString usedActivity)
{
    // qDebug() << "Unlinking " << targettedResource << " from " << usedActivity << " from " << initiatingAgent;

    if (!validateArguments(initiatingAgent, targettedResource, usedActivity)) {
        qWarning() << "Invalid arguments" << initiatingAgent
                   << targettedResource << usedActivity;
        return;
    }

    Q_ASSERT_X(!initiatingAgent.isEmpty(),
               "ResourceLinking::UnlinkResourceFromActivity",
               "Agent shoud not be empty");
    Q_ASSERT_X(!usedActivity.isEmpty(),
               "ResourceLinking::UnlinkResourceFromActivity",
               "Activity shoud not be empty");
    Q_ASSERT_X(!targettedResource.isEmpty(),
               "ResourceLinking::UnlinkResourceFromActivity",
               "Resource shoud not be empty");

    Utils::prepare(resourcesDatabase(), unlinkResourceFromActivityQuery,
        QStringLiteral(
            "DELETE FROM ResourceLink "
            "WHERE "
            "usedActivity      = COALESCE(:usedActivity     , '') AND "
            "initiatingAgent   = COALESCE(:initiatingAgent  , '') AND "
            "targettedResource = COALESCE(:targettedResource, '') "
        ));

    DATABASE_TRANSACTION(resourcesDatabase());

    Utils::exec(Utils::FailOnError, *unlinkResourceFromActivityQuery,
        ":usedActivity"      , usedActivity,
        ":initiatingAgent"   , initiatingAgent,
        ":targettedResource" , targettedResource
    );

    if (!usedActivity.isEmpty()) {
        // auto mangled = QString::fromUtf8(QUrl::toPercentEncoding(targettedResource));
        auto mangled = QString::fromLatin1(targettedResource.toUtf8().toBase64(
            QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals));

        // qDebug() << "Sending link event removed: activities:/" << usedActivity << '/' << mangled;
        org::kde::KDirNotify::emitFilesRemoved(
            { QStringLiteral("activities:/") + usedActivity + '/' + mangled });

        if (usedActivity == StatsPlugin::self()->currentActivity()) {
            // qDebug() << "Sending link event removed: activities:/current/" << mangled;
            org::kde::KDirNotify::emitFilesRemoved({
                QStringLiteral("activities:/current/") + mangled});
        }
    }

    emit ResourceUnlinkedFromActivity(initiatingAgent, targettedResource,
                                      usedActivity);
}

bool ResourceLinking::IsResourceLinkedToActivity(QString initiatingAgent,
                                                 QString targettedResource,
                                                 QString usedActivity)
{
    if (!validateArguments(initiatingAgent, targettedResource, usedActivity)) {
        return false;
    }

    Q_ASSERT_X(!initiatingAgent.isEmpty(),
               "ResourceLinking::IsResourceLinkedToActivity",
               "Agent shoud not be empty");
    Q_ASSERT_X(!usedActivity.isEmpty(),
               "ResourceLinking::IsResourceLinkedToActivity",
               "Activity shoud not be empty");
    Q_ASSERT_X(!targettedResource.isEmpty(),
               "ResourceLinking::IsResourceLinkedToActivity",
               "Resource shoud not be empty");

    Utils::prepare(resourcesDatabase(), isResourceLinkedToActivityQuery,
        QStringLiteral(
            "SELECT * FROM ResourceLink "
            "WHERE "
            "usedActivity      = COALESCE(:usedActivity     , '') AND "
            "initiatingAgent   = COALESCE(:initiatingAgent  , '') AND "
            "targettedResource = COALESCE(:targettedResource, '') "
        ));

    Utils::exec(Utils::FailOnError, *isResourceLinkedToActivityQuery,
        ":usedActivity"      , usedActivity,
        ":initiatingAgent"   , initiatingAgent,
        ":targettedResource" , targettedResource
    );

    return isResourceLinkedToActivityQuery->next();
}

bool ResourceLinking::validateArguments(QString &initiatingAgent,
                                        QString &targettedResource,
                                        QString &usedActivity)
{
    // Validating targetted resource
    if (targettedResource.startsWith(QStringLiteral("file://"))) {
        targettedResource = QUrl(targettedResource).toLocalFile();
    }

    if (targettedResource.startsWith(QStringLiteral("/"))) {
        QFileInfo file(targettedResource);

        if (!file.exists()) {
            return false;
        }

        targettedResource = file.canonicalFilePath();
    }

    // Handling special values for the agent
    if (initiatingAgent.isEmpty()) {
        initiatingAgent = ":global";
    }

    // Handling special values for activities
    if (usedActivity == ":current") {
        usedActivity = StatsPlugin::self()->currentActivity();

    } else if (usedActivity.isEmpty()) {
        usedActivity = ":global";
    }

    // If the activity is not empty and the passed activity
    // does not exist, cancel the request
    if (!usedActivity.isEmpty()
        && !StatsPlugin::self()->listActivities().contains(usedActivity)) {
        return false;
    }

    // qDebug() << "agent" << initiatingAgent
    //          << "resource" << targettedResource
    //          << "activity" << usedActivity;

    return true;
}

void ResourceLinking::onActivityAdded(const QString &activity)
{
    Q_UNUSED(activity);

    // Notify KIO
    // qDebug() << "Added: activities:/  (" << activity << ")";
    org::kde::KDirNotify::emitFilesAdded(QStringLiteral("activities:/"));
}

void ResourceLinking::onActivityRemoved(const QString &activity)
{
    // Notify KIO
    // qDebug() << "Removed: activities:/" << activity;
    org::kde::KDirNotify::emitFilesRemoved(
        { QStringLiteral("activities:/") + activity });

    // Remove statistics for the activity
}

void ResourceLinking::onCurrentActivityChanged(const QString &activity)
{
    Q_UNUSED(activity);

    // Notify KIO
    // qDebug() << "Changed: activities:/current -> " << activity;
    org::kde::KDirNotify::emitFilesAdded(
        { QStringLiteral("activities:/current") });
}
