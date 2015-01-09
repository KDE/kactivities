/*
 *   Copyright (C) 2015 Ivan Cukic <ivan.cukic(at)kde.org>
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

#include "ResourcesDatabaseSchema.h"

#include <QStandardPaths>
#include <QVariant>
#include <QCoreApplication>

namespace Common {
namespace ResourcesDatabaseSchema {

const QString name = QStringLiteral("Resources");

QString version()
{
    return "2014.05.05";
}

QStringList schema()
{
    // If only we could use initializer lists here ...

    return QStringList()

        << // Schema informations table, used for versioning
           "CREATE TABLE IF NOT EXISTS SchemaInfo ("
               "key text PRIMARY KEY, value text"
           ")"

        << QStringLiteral("INSERT OR IGNORE INTO schemaInfo VALUES ('version', '%1')").arg(version())
        << QStringLiteral("UPDATE schemaInfo SET value = '%1' WHERE key = 'version'").arg(version())


        << // The ResourceEvent table saves the Opened/Closed event pairs for
           // a resource. The Accessed event is mapped to those.
           // Focussing events are not stored in order not to get a
           // huge database file and to lessen writes to the disk.
           "CREATE TABLE IF NOT EXISTS ResourceEvent ("
               "usedActivity TEXT, "
               "initiatingAgent TEXT, "
               "targettedResource TEXT, "
               "start INTEGER, "
               "end INTEGER "
           ")"

        << // The ResourceScoreCache table stores the calcualted scores
           // for resources based on the recorded events.
           "CREATE TABLE IF NOT EXISTS ResourceScoreCache ("
               "usedActivity TEXT, "
               "initiatingAgent TEXT, "
               "targettedResource TEXT, "
               "scoreType INTEGER, "
               "cachedScore FLOAT, "
               "firstUpdate INTEGER, "
               "lastUpdate INTEGER, "
               "PRIMARY KEY(usedActivity, initiatingAgent, targettedResource)"
           ")"


        << // @since 2014.05.05
           // The ResourceLink table stores the information, formerly kept by
           // Nepomuk, of which resources are linked to which activities.
           // The additional features compared to the old days are
           // the ability to limit the link to specific applications, and
           // to create global links.
           "CREATE TABLE IF NOT EXISTS ResourceLink ("
               "usedActivity TEXT, "
               "initiatingAgent TEXT, "
               "targettedResource TEXT, "
               "PRIMARY KEY(usedActivity, initiatingAgent, targettedResource)"
           ")"

       ;
}

// TODO: This will require some refactoring after we introduce more databases
QString defaultPath()
{
    return QStandardPaths::writableLocation(
               QStandardPaths::GenericDataLocation)
           + QStringLiteral("/kactivitymanagerd/resources/database");
}

const char *overrideFlagProperty = "org.kde.KActivities.ResourcesDatabase.overrideDatabase";
const char *overrideFileProperty = "org.kde.KActivities.ResourcesDatabase.overrideDatabaseFile";

QString path()
{
    auto app = QCoreApplication::instance();

    return
        (app->property(overrideFlagProperty).toBool()) ?
            app->property(overrideFileProperty).toString() :
            defaultPath();
}

void overridePath(const QString &path)
{
    auto app = QCoreApplication::instance();

    app->setProperty(overrideFlagProperty, true);
    app->setProperty(overrideFileProperty, path);
}

void initSchema(Database &database)
{
    QString dbSchemaVersion;

    auto query = database.execQuery(
        QStringLiteral("SELECT value FROM SchemaInfo WHERE key = 'version'"));

    if (query.next()) {
        dbSchemaVersion = query.value(0).toString();
    }

    // Early bail-out if the schema is up-to-date
    if (dbSchemaVersion == version()) {
        return;
    }

    // Transition to KF5:
    // We left the world of Nepomuk, and all the ontologies went
    // across the sea to the Undying Lands
    if (dbSchemaVersion < QStringLiteral("2014.04.14")) {
        database.execQueries({
            QStringLiteral("ALTER TABLE nuao_DesktopEvent RENAME TO ResourceEvent"),
            QStringLiteral("ALTER TABLE kext_ResourceScoreCache RENAME TO ResourceScoreCache")
        });
    }

    const QString insertSchemaInfoQuery
        = QStringLiteral("INSERT OR IGNORE INTO schemaInfo VALUES ('%1', '%2')");

    const QString updateSchemaInfoQuery
        = QStringLiteral("UPDATE schemaInfo SET value = '%2' WHERE key = '%1'");

    database.execQueries(ResourcesDatabaseSchema::schema());
}

} // namespace Common
} // namespace ResourcesDatabaseSchema

