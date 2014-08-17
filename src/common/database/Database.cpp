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

#include <QStandardPaths>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlField>
#include <QSqlError>
#include <QSqlDriver>
#include <QThread>

#include <memory>
#include <mutex>
#include <map>

class Database::Private {
public:
    QSqlDatabase database;
};

namespace {
    std::mutex databases_mutex;
    std::map<Qt::HANDLE, std::weak_ptr<Database>> databases;
};

std::shared_ptr<Database> Database::instance(Source source)
{
    Q_UNUSED(source) // for the time being

    Qt::HANDLE thread = QThread::currentThreadId();
    std::lock_guard<std::mutex> lock(databases_mutex);

    auto search = databases.find(thread);
    if (search != databases.end()) {
        auto ptr = search->second.lock();

        if (ptr) {
            return std::move(ptr);
        }
    }

    auto ptr = std::make_shared<Database>();

    ptr->d->database = QSqlDatabase::addDatabase(
        "QSQLITE",
        "kactivities_db_resources_" + QString::number((quintptr)thread));
    ptr->d->database.setConnectOptions("QSQLITE_OPEN_READONLY");

    ptr->d->database.setDatabaseName(
            QStandardPaths::writableLocation(
                QStandardPaths::GenericDataLocation)
            + QStringLiteral("/kactivitymanagerd/resources/database"));

    if (!ptr->d->database.open()) {
        ptr.reset();
    } else {
        databases[thread] = ptr;
    }

    return std::move(ptr);
}

Database::Database()
{

}


Database::~Database()
{
}

QSqlQuery Database::query() const
{
    return QSqlQuery(d->database);
}

QSqlQuery Database::query(const QString &query) const
{
    return QSqlQuery(query, d->database);
}


