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

#include <KStandardDirs>

#include <cmath>

#include <config-features.h>

#include <utils/nullptr.h>
#include <utils/d_ptr_implementation.h>
#include <utils/val.h>

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
        return ::exp(- days / 32.0);
    }

    qreal timeFactor(QDateTime fromTime, QDateTime toTime = QDateTime::currentDateTime()) const
    {
        return timeFactor(fromTime.daysTo(toTime));
    }
};

const QString DatabaseConnection::Private::insertSchemaInfoQuery
        = "INSERT INTO schemaInfo VALUES ('%1', '%2')";

const QString DatabaseConnection::Private::updateSchemaInfoQuery
        = "UPDATE schemaInfo SET value = '%2' WHERE key = '%1'";

const QString DatabaseConnection::Private::openDesktopEventQuery
        = "INSERT INTO nuao_DesktopEvent VALUES ("
            "'%1', "     // usedActivity
            "'%2', "     // initiatingAgent
            "'%3', "     // targettedResource
            "%4, "       // start
            "%5"         // end
            ")";

const QString DatabaseConnection::Private::closeDesktopEventQuery
        = "UPDATE nuao_DesktopEvent SET "
            "end = %4 "
            "WHERE "
            "'%1' = usedActivity AND "
            "'%2' = initiatingAgent AND "
            "'%3' = targettedResource AND "
            "end IS NULL"
            ;

const QString DatabaseConnection::Private::createResourceScoreCacheQuery
        = "INSERT INTO kext_ResourceScoreCache VALUES("
            "'%1', "     // usedActivity
            "'%2', "     // initiatingAgent
            "'%3', "     // targettedResource
            "0,"         // scoreType
            "0.0,"       // cachedScore
            "-1,"        // lastUpdate
            "%4"         // firstUpdate
            ")"
            ;

const QString DatabaseConnection::Private::getResourceScoreCacheQuery
        = "SELECT cachedScore, lastUpdate FROM kext_ResourceScoreCache "
            "WHERE "
            "'%1' = usedActivity AND "
            "'%2' = initiatingAgent AND "
            "'%3' = targettedResource "
            ;

const QString DatabaseConnection::Private::updateResourceScoreCacheQuery
        = "UPDATE kext_ResourceScoreCache SET "
            "cachedScore = %4, "
            "lastUpdate = %5 "
            "WHERE "
            "'%1' = usedActivity AND "
            "'%2' = initiatingAgent AND "
            "'%3' = targettedResource "
            ;

const QString DatabaseConnection::Private::getScoreAdditionQuery
        = "SELECT start, end FROM nuao_DesktopEvent "
            "WHERE "
            "'%1' = usedActivity AND "
            "'%2' = initiatingAgent AND "
            "'%3' = targettedResource AND "
            "start > %4"
            ;

void DatabaseConnection::openDesktopEvent(const QString & usedActivity, const QString & initiatingAgent,
        const QString & targettedResource, const QDateTime & start, const QDateTime & end)
{
    d->database.exec(
        Private::openDesktopEventQuery
            .arg(usedActivity)
            .arg(initiatingAgent)
            .arg(targettedResource)
            .arg(start.toTime_t())
            .arg(end.isNull() ? "NULL" : QString::number(end.toTime_t()))
        );
}

void DatabaseConnection::closeDesktopEvent(const QString & usedActivity, const QString & initiatingAgent,
        const QString & targettedResource, const QDateTime & end)
{
    d->database.exec(
        Private::closeDesktopEventQuery
            .arg(usedActivity)
            .arg(initiatingAgent)
            .arg(targettedResource)
            .arg(end.toTime_t())
        );
}

void DatabaseConnection::getResourceScoreCache(const QString & usedActivity, const QString & initiatingAgent,
        const QUrl & targettedResource, qreal & score, QDateTime & lastUpdate)
{
    // This can fail if we have the cache already made

    d->database.exec(
        Private::createResourceScoreCacheQuery
            .arg(usedActivity)
            .arg(initiatingAgent)
            .arg(targettedResource.toString())
            .arg(QDateTime::currentDateTime().toTime_t())
        );

    // Getting the old score

    auto query = d->database.exec(
        Private::getResourceScoreCacheQuery
            .arg(usedActivity)
            .arg(initiatingAgent)
            .arg(targettedResource.toString())
        );

    if (query.next()) {
        val time = query.value(1).toLongLong();

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
            .arg(targettedResource.toString())
            .arg(lastUpdate.toTime_t())
        );

    long start = 0;

    while (query.next()) {
        start = query.value(0).toLongLong();

        val end = query.value(1).toLongLong();
        val intervalLength = end - start;

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
            .arg(targettedResource.toString())
            .arg(score)
            .arg(start)
        );
}

DatabaseConnection * DatabaseConnection::s_instance = nullptr;

DatabaseConnection * DatabaseConnection::self()
{
    if (!s_instance) {
        s_instance = new DatabaseConnection();
    }

    return s_instance;
}

DatabaseConnection::DatabaseConnection()
    : d()
{
    val path = KStandardDirs::locateLocal("data", "activitymanager/resources/database", true);

    d->database = QSqlDatabase::addDatabase("QSQLITE", "plugins_sqlite_db_resources");
    d->database.setDatabaseName(path);

    d->initialized = d->database.open();

    if (!d->initialized) {
        qDebug() << "Failed to open the database" << path
            << d->database.lastError();
    }

    initDatabaseSchema();
}

DatabaseConnection::~DatabaseConnection()
{
}

QSqlDatabase & DatabaseConnection::database()
{
    return d->database;
}

void DatabaseConnection::initDatabaseSchema()
{
    QString dbSchemaVersion = "0.0";

    auto query = d->database.exec("SELECT value FROM SchemaInfo WHERE key = 'version'");

    if (query.next()) {
        dbSchemaVersion = query.value(0).toString();
    }

    if (dbSchemaVersion < "1.0") {
        qDebug() << "The database doesn't exist, it is being created";

        query.exec("CREATE TABLE IF NOT EXISTS SchemaInfo (key text PRIMARY KEY, value text)");
        query.exec(Private::insertSchemaInfoQuery.arg("version", "1.0"));

        query.exec(
                "CREATE TABLE IF NOT EXISTS nuao_DesktopEvent ("
                "usedActivity TEXT, "
                "initiatingAgent TEXT, "
                "targettedResource TEXT, "
                "start INTEGER, "
                "end INTEGER "
                ")"
            );

        query.exec(
                "CREATE TABLE IF NOT EXISTS kext_ResourceScoreCache ("
                "usedActivity TEXT, "
                "initiatingAgent TEXT, "
                "targettedResource TEXT, "
                "scoreType INTEGER, "
                "cachedScore FLOAT, "
                "lastUpdate INTEGER, "
                "PRIMARY KEY(usedActivity, initiatingAgent, targettedResource)"
                ")"
            );
    }

    if (dbSchemaVersion < "1.01") {
        qDebug() << "Upgrading the database version to 1.01";

        // Adding the firstUpdate field so that we can have
        // a crude way of deleting the score caches when
        // the user requests a partial history deletion

        query.exec(Private::updateSchemaInfoQuery.arg("version", "1.01"));

        query.exec(
                "ALTER TABLE kext_ResourceScoreCache "
                "ADD COLUMN firstUpdate INTEGER"
            );

        query.exec(
                "UPDATE kext_ResourceScoreCache "
                "SET firstUpdate = " + QString::number(QDateTime::currentDateTime().toTime_t())
            );
    }
}

