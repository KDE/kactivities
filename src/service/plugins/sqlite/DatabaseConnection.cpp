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

#include "DatabaseConnection.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>
#include <QDir>

#include <QStandardPaths>

#include <cmath>

#include <kactivities-features.h>

#include <utils/d_ptr_implementation.h>

class DatabaseConnection::Private {
public:
    QSqlDatabase database;
    bool initialized : 1;

    static const QString insertSchemaInfoQuery;
    static const QString updateSchemaInfoQuery;

    static const QString openDesktopEventQuery;
    static const QString closeDesktopEventQuery;
    static const QString getScoreAdditionQuery;

    static const QString createResourceScoreCacheQuery;
    static const QString getResourceScoreCacheQuery;
    static const QString updateResourceScoreCacheQuery;

    qreal timeFactor(int days) const
    {
        // Exp is falling rather quickly, we are slowing it 32 times
        return ::exp(-days / 32.0);
    }

    qreal timeFactor(QDateTime fromTime, QDateTime toTime = QDateTime::currentDateTime()) const
    {
        return timeFactor(fromTime.daysTo(toTime));
    }
};

const QString DatabaseConnection::Private::insertSchemaInfoQuery
    = QStringLiteral("INSERT INTO schemaInfo VALUES ('%1', '%2')");

const QString DatabaseConnection::Private::updateSchemaInfoQuery
    = QStringLiteral("UPDATE schemaInfo SET value = '%2' WHERE key = '%1'");

const QString DatabaseConnection::Private::openDesktopEventQuery
    = QStringLiteral("INSERT INTO nuao_DesktopEvent VALUES ("
                     "'%1', " // usedActivity
                     "'%2', " // initiatingAgent
                     "'%3', " // targettedResource
                     "%4, " // start
                     "%5" // end
                     ")");

const QString DatabaseConnection::Private::closeDesktopEventQuery
    = QStringLiteral("UPDATE nuao_DesktopEvent SET "
                     "end = %4 "
                     "WHERE "
                     "'%1' = usedActivity AND "
                     "'%2' = initiatingAgent AND "
                     "'%3' = targettedResource AND "
                     "end IS NULL");

const QString DatabaseConnection::Private::createResourceScoreCacheQuery
    = QStringLiteral("INSERT INTO kext_ResourceScoreCache VALUES("
                     "'%1', " // usedActivity
                     "'%2', " // initiatingAgent
                     "'%3', " // targettedResource
                     "0," // scoreType
                     "0.0," // cachedScore
                     "-1," // lastUpdate
                     "%4" // firstUpdate
                     ")");

const QString DatabaseConnection::Private::getResourceScoreCacheQuery
    = QStringLiteral("SELECT cachedScore, lastUpdate FROM kext_ResourceScoreCache "
                     "WHERE "
                     "'%1' = usedActivity AND "
                     "'%2' = initiatingAgent AND "
                     "'%3' = targettedResource ");

const QString DatabaseConnection::Private::updateResourceScoreCacheQuery
    = QStringLiteral("UPDATE kext_ResourceScoreCache SET "
                     "cachedScore = %4, "
                     "lastUpdate = %5 "
                     "WHERE "
                     "'%1' = usedActivity AND "
                     "'%2' = initiatingAgent AND "
                     "'%3' = targettedResource ");

const QString DatabaseConnection::Private::getScoreAdditionQuery
    = QStringLiteral("SELECT start, end FROM nuao_DesktopEvent "
                     "WHERE "
                     "'%1' = usedActivity AND "
                     "'%2' = initiatingAgent AND "
                     "'%3' = targettedResource AND "
                     "start > %4");

void DatabaseConnection::openDesktopEvent(const QString &usedActivity, const QString &initiatingAgent,
                                          const QString &targettedResource, const QDateTime &start, const QDateTime &end)
{
    d->database.exec(
        Private::openDesktopEventQuery
            .arg(usedActivity)
            .arg(initiatingAgent)
            .arg(targettedResource)
            .arg(start.toTime_t())
            .arg(end.isNull() ? QStringLiteral("NULL") : QString::number(end.toTime_t())));
}

void DatabaseConnection::closeDesktopEvent(const QString &usedActivity, const QString &initiatingAgent,
                                           const QString &targettedResource, const QDateTime &end)
{
    d->database.exec(
        Private::closeDesktopEventQuery
            .arg(usedActivity)
            .arg(initiatingAgent)
            .arg(targettedResource)
            .arg(end.toTime_t()));
}

void DatabaseConnection::getResourceScoreCache(const QString &usedActivity, const QString &initiatingAgent,
                                               const QString &targettedResource, qreal &score, QDateTime &lastUpdate)
{
    // This can fail if we have the cache already made

    d->database.exec(
        Private::createResourceScoreCacheQuery
            .arg(usedActivity)
            .arg(initiatingAgent)
            .arg(targettedResource)
            .arg(QDateTime::currentDateTime().toTime_t()));

    // Getting the old score

    auto query = d->database.exec(
        Private::getResourceScoreCacheQuery
            .arg(usedActivity)
            .arg(initiatingAgent)
            .arg(targettedResource));

    if (query.next()) {
        const auto time = query.value(1).toLongLong();

        if (time < 0) {
            // If we haven't had the cache before, set the score to 0
            lastUpdate = QDateTime();
            score = 0;

        } else {
            // Adjusting the score depending on the time that passed since the
            // last update
            lastUpdate.setTime_t(time);

            score = query.value(0).toReal();
            score *= d->timeFactor(lastUpdate);
        }
    }

    // Calculating the updated score

    query = d->database.exec(
        Private::getScoreAdditionQuery
            .arg(usedActivity)
            .arg(initiatingAgent)
            .arg(targettedResource)
            .arg(lastUpdate.toTime_t()));

    long start = 0;

    while (query.next()) {
        start = query.value(0).toLongLong();

        const auto end = query.value(1).toLongLong();
        const auto intervalLength = end - start;

        if (intervalLength == 0) {
            // We have an Accessed event - otherwise, this wouldn't be 0
            score += d->timeFactor(QDateTime::fromTime_t(end)); // like it is open for 1 minute

        } else if (intervalLength >= 4) {
            // Ignoring stuff that was open for less than 4 seconds
            score += d->timeFactor(QDateTime::fromTime_t(end)) * intervalLength / 60.0;
        }
    }

    // Updating the score

    d->database.exec(
        Private::updateResourceScoreCacheQuery
            .arg(usedActivity)
            .arg(initiatingAgent)
            .arg(targettedResource)
            .arg(score)
            .arg(start));
}

DatabaseConnection *DatabaseConnection::s_instance = Q_NULLPTR;

DatabaseConnection *DatabaseConnection::self()
{
    if (!s_instance) {
        s_instance = new DatabaseConnection();
    }

    return s_instance;
}

DatabaseConnection::DatabaseConnection()
    : d()
{
    const QString databaseDir = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QStringLiteral("/activitymanager/resources/");

    QDir().mkpath(databaseDir);

    d->database = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), QStringLiteral("plugins_sqlite_db_resources"));
    d->database.setDatabaseName(databaseDir + QStringLiteral("database"));

    d->initialized = d->database.open();

    if (!d->initialized) {
        qDebug() << "Failed to open the database" << databaseDir
                 << d->database.lastError();
    }

    initDatabaseSchema();
}

DatabaseConnection::~DatabaseConnection()
{
}

QSqlDatabase &DatabaseConnection::database()
{
    return d->database;
}

void DatabaseConnection::initDatabaseSchema()
{
    QString dbSchemaVersion = QStringLiteral("0.0");

    auto query = d->database.exec(QStringLiteral("SELECT value FROM SchemaInfo WHERE key = 'version'"));

    if (query.next()) {
        dbSchemaVersion = query.value(0).toString();
    }

    if (dbSchemaVersion < QStringLiteral("1.0")) {
        qDebug() << "The database doesn't exist, it is being created";

        query.exec(QStringLiteral("CREATE TABLE IF NOT EXISTS SchemaInfo (key text PRIMARY KEY, value text)"));
        query.exec(Private::insertSchemaInfoQuery.arg(QStringLiteral("version"), QStringLiteral("1.0")));

        query.exec(QStringLiteral(
            "CREATE TABLE IF NOT EXISTS nuao_DesktopEvent ("
            "usedActivity TEXT, "
            "initiatingAgent TEXT, "
            "targettedResource TEXT, "
            "start INTEGER, "
            "end INTEGER "
            ")"));

        query.exec(QStringLiteral(
            "CREATE TABLE IF NOT EXISTS kext_ResourceScoreCache ("
            "usedActivity TEXT, "
            "initiatingAgent TEXT, "
            "targettedResource TEXT, "
            "scoreType INTEGER, "
            "cachedScore FLOAT, "
            "lastUpdate INTEGER, "
            "PRIMARY KEY(usedActivity, initiatingAgent, targettedResource)"
            ")"));
    }

    if (dbSchemaVersion < QStringLiteral("1.01")) {
        qDebug() << "Upgrading the database version to 1.01";

        // Adding the firstUpdate field so that we can have
        // a crude way of deleting the score caches when
        // the user requests a partial history deletion

        query.exec(Private::updateSchemaInfoQuery.arg(QStringLiteral("version"), QStringLiteral("1.01")));

        query.exec(QStringLiteral(
            "ALTER TABLE kext_ResourceScoreCache "
            "ADD COLUMN firstUpdate INTEGER"));

        query.exec(QStringLiteral(
                       "UPDATE kext_ResourceScoreCache "
                       "SET firstUpdate = ") + QString::number(QDateTime::currentDateTime().toTime_t()));
    }
}
