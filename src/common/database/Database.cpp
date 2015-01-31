/*
 * Copyright 2014 Ivan Cukic <ivan.cukic@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Database.h"

#include <utils/d_ptr_implementation.h>
#include <common/database/schema/ResourcesDatabaseSchema.h>

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlField>
#include <QSqlError>
#include <QSqlDriver>
#include <QThread>
#include <QDebug>

#include <memory>
#include <mutex>
#include <map>

namespace Common {

class Database::Private {
public:
    QSqlDatabase database;
};

namespace {
#ifdef QT_DEBUG
    QString lastExecutedQuery;
#endif

    std::mutex databases_mutex;

    struct DatabaseInfo {
        Qt::HANDLE thread;
        Database::OpenMode openMode;
    };

    bool operator<(const DatabaseInfo &left, const DatabaseInfo &right)
    {
        return
            left.thread < right.thread     ? true  :
            left.thread > right.thread     ? false :
            left.openMode < right.openMode;
    }

    std::map<DatabaseInfo, std::weak_ptr<Database>> databases;
};

Database::Ptr Database::instance(Source source, OpenMode openMode)
{
    Q_UNUSED(source) // for the time being

    std::lock_guard<std::mutex> lock(databases_mutex);

    // We are saving instances per thread and per read/write mode
    DatabaseInfo info;
    info.thread   = QThread::currentThreadId();
    info.openMode = openMode;

    // Do we have an instance matching the request?
    auto search = databases.find(info);
    if (search != databases.end()) {
        auto ptr = search->second.lock();

        if (ptr) {
            qDebug() << "Matched an existing database";
            return std::move(ptr);
        }
    }

    // Creating a new database instance
    // qDebug() << "We do not have an instance for this thread / mode";
    auto ptr = std::make_shared<Database>();

    auto databaseConnectionName =
            "kactivities_db_resources_"
                // Adding the thread number to the database name
                + QString::number((quintptr)info.thread)
                // And whether it is read-only or read-write
                + (info.openMode == ReadOnly ? "_readonly" : "_readwrite");

    ptr->d->database
        = QSqlDatabase::contains(databaseConnectionName)
              ? QSqlDatabase::database(databaseConnectionName)
              : QSqlDatabase::addDatabase("QSQLITE", databaseConnectionName);

    if (info.openMode == ReadOnly) {
        ptr->d->database.setConnectOptions("QSQLITE_OPEN_READONLY");
    }

    // We are allowing the database file to be overridden mostly for testing purposes
    ptr->d->database.setDatabaseName(ResourcesDatabaseSchema::path());

    if (!ptr->d->database.open()) {
        qDebug() << "Database is not open: "
                 << ptr->d->database.connectionName()
                 << ptr->d->database.databaseName()
                 << ptr->d->database.lastError();

        if (info.openMode == ReadWrite) {
            qFatal("Opening the database in RW mode should always succeed");
        }
        ptr.reset();
    } else {
        databases[info] = ptr;
    }

    return std::move(ptr);
}

Database::Database()
{

}


Database::~Database()
{
}

QSqlQuery Database::createQuery() const
{
    return QSqlQuery(d->database);
}

QString Database::lastQuery() const
{
    return lastExecutedQuery;
}

QSqlQuery Database::execQuery(const QString &query, bool ignoreErrors) const
{
#ifdef QT_NO_DEBUG
    return QSqlQuery(query, d->database);
#else
    auto result = QSqlQuery(query, d->database);

    lastExecutedQuery = query;

    if (!ignoreErrors && result.lastError().isValid()) {
        qWarning() << "SQL: "
                   << "\n    error: " << result.lastError()
                   << "\n    query: " << query;
    }

    return result;
#endif
}

QSqlQuery Database::execQueries(const QStringList &queries) const
{
    QSqlQuery result;

    for (const auto query: queries) {
        result = execQuery(query);
    }

    return result;
}

} // namespace Common

