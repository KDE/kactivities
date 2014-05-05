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
#include <kactivities-features.h>
#include "Database.h"

// Qt
#include <QDir>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QVariant>

// Utils
#include <utils/d_ptr_implementation.h>
#include <utils/qsqlquery.h>

// System
#include <cmath>
#include <memory>

// Local
#include "Debug.h"
#include "Utils.h"

class Database::Private {
public:
    QSqlDatabase database;

    template <typename T>
    inline QSqlQuery execQueries(const T &query)
    {
        auto result = QSqlQuery(query, database);

        if (result.lastError().isValid()) {
            qDebug() << result.lastError().text();
        }

        return result;
    }

    template <typename T1, typename T2, typename... Ts>
    inline QSqlQuery execQueries(const T1 &query1, const T2 &query2,
                          const Ts &... queries)
    {
        execQueries(query1);
        return execQueries(query2, queries...);
    }
};

Database *Database::self()
{
    static Database instance;
    return &instance;
}

Database::Database()
    : d()
{
    const QString databaseDir
        = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)
          + QStringLiteral("/kactivitymanagerd/resources/");

    QDir().mkpath(databaseDir);

    d->database = QSqlDatabase::addDatabase(
        QStringLiteral("QSQLITE"),
        QStringLiteral("plugins_sqlite_db_resources"));

    d->database.setDatabaseName(databaseDir + QStringLiteral("database"));

    d->database.open();

    initDatabaseSchema();
}

Database::~Database()
{
}

QSqlDatabase &Database::database()
{
    return d->database;
}

void Database::initDatabaseSchema()
{
    const QString currentSchemaVersion = QStringLiteral("2014.05.05");

    QString dbSchemaVersion;

    auto query = d->execQueries(
        QStringLiteral("SELECT value FROM SchemaInfo WHERE key = 'version'"));

    if (query.next()) {
        dbSchemaVersion = query.value(0).toString();
    }

    // Early bail-out if the schema is up-to-date
    if (dbSchemaVersion == currentSchemaVersion) {
        return;
    }

    // Transition to KF5:
    // We left the world of Nepomuk, and all the ontologies went
    // across the sea to the Undying Lands
    if (dbSchemaVersion < QStringLiteral("2014.04.14")) {
        d->execQueries(
            QStringLiteral("ALTER TABLE nuao_DesktopEvent RENAME TO ResourceEvent"),
            QStringLiteral("ALTER TABLE kext_ResourceScoreCache RENAME TO ResourceScoreCache")
        );
    }

    const QString insertSchemaInfoQuery
        = QStringLiteral("INSERT OR IGNORE INTO schemaInfo VALUES ('%1', '%2')");

    const QString updateSchemaInfoQuery
        = QStringLiteral("UPDATE schemaInfo SET value = '%2' WHERE key = '%1'");

    d->execQueries(
        QStringLiteral("CREATE TABLE IF NOT EXISTS SchemaInfo "
                       "(key text PRIMARY KEY, value text)"),

        insertSchemaInfoQuery.arg(QStringLiteral("version"),
                                           currentSchemaVersion),

        updateSchemaInfoQuery.arg(QStringLiteral("version"),
                                           currentSchemaVersion),

        QStringLiteral("CREATE TABLE IF NOT EXISTS ResourceEvent ("
                       "usedActivity TEXT, "
                       "initiatingAgent TEXT, "
                       "targettedResource TEXT, "
                       "start INTEGER, "
                       "end INTEGER "
                       ")"),

        QStringLiteral("CREATE TABLE IF NOT EXISTS ResourceScoreCache ("
                       "usedActivity TEXT, "
                       "initiatingAgent TEXT, "
                       "targettedResource TEXT, "
                       "scoreType INTEGER, "
                       "cachedScore FLOAT, "
                       "firstUpdate INTEGER, "
                       "lastUpdate INTEGER, "
                       "PRIMARY KEY(usedActivity, initiatingAgent, targettedResource)"
                       ")"),

        // Introduced in 2014.05.05
        QStringLiteral("CREATE TABLE IF NOT EXISTS ResourceLink ("
                       "usedActivity TEXT, "
                       "initiatingAgent TEXT, "
                       "targettedResource TEXT, "
                       "PRIMARY KEY(usedActivity, initiatingAgent, targettedResource)"
                       ")")
    );
}
