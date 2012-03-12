/*
 *   Copyright (C) 2011 Ivan Cukic ivan.cukic(at)kde.org
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2 of
 *   the License, or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "activityranking.h"
#include "activityrankingadaptor.h"

#include <QFile>

#include <QDBusConnection>

#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QSqlTableModel>

#include <KStandardDirs>
#include <KDebug>

#include "Location.h"

#define AR_FAKE_EVENTS_FEED

#define PRINT_LAST_ERROR if (query.lastError().isValid()) kDebug() << "DATABASE ERROR" << query.lastError();

class ActivityRankingPlugin::Private {
public:
    QSqlDatabase database;
    // QSqlTableModel * tableActivityEvents;
    // QSqlTableModel * tableMonthScores;
    // QSqlTableModel * tableWeekScores;

    QSqlRecord currentActivityRecord;
    QString activity;
    QString lastLocation;
    qint64 activityStart;

    void processActivityInterval(const QString & activity, const QString & location, qint64 start, qint64 end);
    void processWeekData(const QString & activity, const QString & location, qint64 start, qint64 end);
    void processMonthData(const QString & activity, const QString & location, qint64 start, qint64 end);

    void closeDanglingActivityRecords();
    void ensureMonthScoreExists(const QString & activity, int year, int month, const QString & location);
    void ensureWeekScoreExists(const QString & activity, int year, int week, const QString & location);

    QMap <QString, qreal> topActivitiesFor(const QDateTime & time, const QString & location);

#ifdef AR_FAKE_EVENTS_FEED
    void FakeEventsFeed();
#endif

    static QString insertSchemaInfo;
    static QString closeActivityInterval;
    static QString insertActivityInterval;

    static QString insertWeekScore;
    static QString selectWeekScore;

    static QString insertMonthScore;
    static QString selectMonthScore;

    static QString selectScore;
};

QString ActivityRankingPlugin::Private::insertSchemaInfo         = "INSERT INTO schemaInfo VALUES ('%1', '%2')";
QString ActivityRankingPlugin::Private::closeActivityInterval    = "UPDATE ActivityEvents SET end = %1 WHERE activity = '%2' AND end IS NULL";
QString ActivityRankingPlugin::Private::insertActivityInterval   = "INSERT INTO ActivityEvents VALUES('%1', '%2', %3, NULL)";

// For the average and dispersion
// select activity, start - (select max(start) from ActivityEvents as ae where ae.start < a.start) as diff from ActivityEvents as a
// QString ActivityRankingPlugin::Private::selectDanglingActivities = "SELECT * FROM ActivityEvents WHERE end IS NULL ORDER BY start";

QString ActivityRankingPlugin::Private::insertWeekScore  =
    "INSERT INTO WeekScores  (activity, year, week, location)  VALUES('%1', %2, %3, '%4')";

QString ActivityRankingPlugin::Private::selectWeekScore  =
    "SELECT * FROM WeekScores WHERE activity = '%1' AND year = %2 AND week = %3 AND location = '%4'";


QString ActivityRankingPlugin::Private::insertMonthScore =
    "INSERT INTO MonthScores (activity, year, month, location) VALUES('%1', %2, %3, '%4')";

QString ActivityRankingPlugin::Private::selectMonthScore =
    "SELECT * FROM MonthScores WHERE activity = '%1' AND year = %2 AND month = %3 AND location = '%4'";


QString ActivityRankingPlugin::Private::selectScore =
    "SELECT week.activity, week.score + month.score as sumscore "
    "FROM "
    "(SELECT activity, location, SUM(s%1%2) as score FROM WeekScores GROUP BY activity) AS week, "
    "(SELECT activity, location, SUM(s%3) as score FROM MonthScores GROUP BY activity) AS month "
    "WHERE week.activity = month.activity AND week.location = '%4' AND month.location = '%4' ORDER BY sumscore DESC";


#ifdef AR_FAKE_EVENTS_FEED

#include <QTextStream>
#include <QDateTime>

void ActivityRankingPlugin::Private::FakeEventsFeed()
{
    kDebug() << "Fake data loading";
    QFile infile("/tmp/fakedata");

    if (!infile.open(QFile::ReadOnly)) return;

    QTextStream in(&infile);

    while(!in.atEnd()) {
        QStringList args = in.readLine().split(' ');

        if (args.size() == 5) {
            // qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
            qint64 start     = QDateTime::fromString(args[0] + ' ' + args[1], "yyyy.MM.dd hh:mm").toMSecsSinceEpoch();
            qint64 end       = QDateTime::fromString(args[2] + ' ' + args[3], "yyyy.MM.dd hh:mm").toMSecsSinceEpoch();
            QString activity = args[4];

            kDebug() << start << end << activity;

            processActivityInterval(activity, QString(), start, end);
        }
    }
}

#endif // AR_FAKE_EVENTS_FEED


void ActivityRankingPlugin::Private::ensureWeekScoreExists(const QString & activity, int year, int week, const QString & location)
{
    // If it fails, it means we already have it, ignore the error
    QSqlQuery query(
        insertWeekScore
        .arg(activity)
        .arg(year)
        .arg(week)
        .arg(location)
    ,
        database
    );
    PRINT_LAST_ERROR;
}

void ActivityRankingPlugin::Private::ensureMonthScoreExists(const QString & activity, int year, int month, const QString & location)
{
    // If it fails, it means we already have it, ignore the error
    QSqlQuery query(
        insertMonthScore
        .arg(activity)
        .arg(year)
        .arg(month)
        .arg(location)
    ,
        database
    );
    PRINT_LAST_ERROR;
}

void ActivityRankingPlugin::Private::processActivityInterval(const QString & activity, const QString & location, qint64 start, qint64 end)
{
    kDebug() << activity << location << start << end;

    // Processing the per-week data
    processWeekData(activity, location, start, end);

    // Processing the month data
    processMonthData(activity, location, start, end);

}

void ActivityRankingPlugin::Private::processWeekData(const QString & activity, const QString & location, qint64 start, qint64 end)
{
    QDateTime startDateTime = QDateTime::fromMSecsSinceEpoch(start);
    QDateTime endDateTime   = QDateTime::fromMSecsSinceEpoch(end);

    #define fordate(What, Start, End) \
        for (int What = Start.date().What(); What <= End.date().What(); What++)

    // This can be a bit more efficient
    fordate(year, startDateTime, endDateTime) {
        fordate(weekNumber, startDateTime, endDateTime) {
            kDebug() << activity << year << weekNumber;

            ensureWeekScoreExists(activity, year, weekNumber, location);

            QDateTime weekStartDateTime(QDate(year, 1, 1).addDays((weekNumber - 1) * 7));
            qint64 weekStart = weekStartDateTime.toMSecsSinceEpoch();
            qint64 weekEnd   = weekStartDateTime.addDays(7).toMSecsSinceEpoch();

            QDateTime currentStart = QDateTime::fromMSecsSinceEpoch(qMax(weekStart, start));
            QDateTime currentEnd   = QDateTime::fromMSecsSinceEpoch(qMin(weekEnd, end));

            QSqlQuery query(
                selectWeekScore
                    .arg(activity)
                    .arg(year)
                    .arg(weekNumber)
                    .arg(location)
                ,
                    database
                );
            PRINT_LAST_ERROR;


            if (query.next()) {
                QSqlRecord record = query.record();
                int iFirstColumn = record.indexOf("s00");

                #define SEGMENTS 8
                for (int day = currentStart.date().dayOfWeek(); day <= currentEnd.date().dayOfWeek(); day++) {
                    const int startSegment = floor(currentStart.time().hour() / 3.0);
                    const int endSegment   = ceil(currentEnd.time().hour() / 3.0);

                    for (int segment = 0; segment < SEGMENTS; segment++) {
                        const int index = iFirstColumn + SEGMENTS * (day - 1) + segment;

                        // Setting the 1.0 value for the active segments
                        if (startSegment <= segment && segment <= endSegment) {
                            record.setValue(index, 1.0);
                        }

                        // Setting the .33 for the whole day
                        else if (record.value(index).toDouble() < .33) {
                            record.setValue(index, .33);
                        }
                    }

                    // Setting the .67 for the edge stuff

                    if (startSegment > 1) {
                        const int index = iFirstColumn + SEGMENTS * (day - 1) + startSegment - 1;
                        if (record.value(index).toDouble() < .67) {
                            record.setValue(index, .67);
                        }
                    }

                    if (endSegment < 7) {
                        const int index = iFirstColumn + SEGMENTS * (day - 1) + endSegment + 1;
                        if (record.value(index).toDouble() < .67) {
                            record.setValue(index, .67);
                        }
                    }


                }
                #undef SEGMENTS


                const QString & where = " WHERE activity = '%1' AND year = %2 AND week = %3 AND location='%4'";
                QSqlQuery query(database.driver()->
                        sqlStatement(QSqlDriver::UpdateStatement, "WeekScores", record, false)
                        + where.arg(activity).arg(year).arg(weekNumber).arg(location)
                    ,
                        database
                    );
                PRINT_LAST_ERROR;
            }
        }
    }

    #undef fordate

}

void ActivityRankingPlugin::Private::processMonthData(const QString & activity, const QString & location, qint64 start, qint64 end)
{
    QDateTime startDateTime = QDateTime::fromMSecsSinceEpoch(start);
    QDateTime endDateTime   = QDateTime::fromMSecsSinceEpoch(end);

    #define fordate(What, Start, End) \
        for (int What = Start.date().What(); What <= End.date().What(); What++)

    // This can be a bit more efficient
    fordate(year, startDateTime, endDateTime) {
        fordate(month, startDateTime, endDateTime) {
            kDebug() << activity << year << month;

            ensureMonthScoreExists(activity, year, month, location);

            QDateTime monthStartDateTime(QDate(year, month, 1));
            qint64 monthStart = monthStartDateTime.toMSecsSinceEpoch();
            qint64 monthEnd   = monthStartDateTime.addMonths(1).toMSecsSinceEpoch();

            qint64 currentStart = qMax(monthStart, start);
            qint64 currentEnd   = qMin(monthEnd, end);

            qreal coefStart = (currentStart - monthStart) / (qreal) (monthEnd - monthStart) * 64;
            qreal coefEnd   = (currentEnd   - monthStart) / (qreal) (monthEnd - monthStart) * 64;

            int iStart   = ceil(coefStart);
            int iEnd     = floor(coefEnd);

            QSqlQuery query(
                selectMonthScore
                    .arg(activity)
                    .arg(year)
                    .arg(month)
                    .arg(location)
                ,
                    database
                );
            PRINT_LAST_ERROR;

            #define increaseValue(Index, Addition) \
                record.setValue(Index, record.value(Index).toDouble() + Addition)

            if (query.next()) {
                QSqlRecord record = query.record();
                int iFirstColumn = record.indexOf("s00");

                if (iStart > 0) {
                    increaseValue(iFirstColumn + iStart - 1, iStart - coefStart);
                }

                for (int i = iStart; i < iEnd; i++) {
                    increaseValue(iFirstColumn + i, 1.0);
                }

                if (iEnd + 1 < 64) {
                    increaseValue(iFirstColumn + iEnd, coefEnd - iEnd);
                }

                const QString & where = " WHERE activity = '%1' AND year = %2 AND month = %3 AND location = '%4'";
                QSqlQuery query(database.driver()->
                        sqlStatement(QSqlDriver::UpdateStatement, "MonthScores", record, false)
                        + where.arg(activity).arg(year).arg(month).arg(location)
                    ,
                        database
                    );
                PRINT_LAST_ERROR;
            }

            #undef increaseValue
        }
    }

    #undef fordate

}

void ActivityRankingPlugin::Private::closeDanglingActivityRecords()
{
    kDebug() << "closing...";

    // TODO: A possible problem is that theoretically the dangling ones can be
    // before a non dangling one which will produce overlapping

    QSqlTableModel tableActivityEvents(NULL, database);
    tableActivityEvents.setTable("ActivityEvents");
    tableActivityEvents.setFilter("end IS NULL");
    tableActivityEvents.select();

    // Setting the current time as the end of the last dangling event
    int i = tableActivityEvents.rowCount() - 1;
    kDebug() << "dangling count:" << i+1;

    if (i < 0) return;

    QSqlRecord record = tableActivityEvents.record(i);

    record.setValue("end", QDateTime::currentMSecsSinceEpoch());
    tableActivityEvents.setRecord(i, record);

    --i;

    // Setting the start of one event to be the end of the previous
    // one

    qint64 end = record.value("start").toLongLong();

    for (int i = tableActivityEvents.rowCount() - 2; i >= 0; i--) {
        record = tableActivityEvents.record(i);

        record.setValue("end", end);
        end = record.value("start").toLongLong();

        processActivityInterval(
                record.value("activity").toString(),
                record.value("location").toString(),
                end, // record.value("start").toLongLong(),
                record.value("end").toLongLong()
            );

        tableActivityEvents.setRecord(i, record);
    }

    tableActivityEvents.submitAll();

}

ActivityRankingPlugin::ActivityRankingPlugin(QObject * parent, const QVariantList & args)
    : Plugin(parent), d(new Private())
{
    Q_UNUSED(args)
    // kDebug() << "We are in the ActivityRankingPlugin";
}

bool ActivityRankingPlugin::init()
{
    QDBusConnection dbus = QDBusConnection::sessionBus();
    new ActivityRankingAdaptor(this);
    dbus.registerObject("/ActivityRanking", this);

    QString path = KStandardDirs::locateLocal("data", "activitymanager/activityranking/database", true);

    d->database = QSqlDatabase::addDatabase("QSQLITE", "plugins_activityranking_db");
    d->database.setDatabaseName(path);

    if (!d->database.open()) {
        qWarning() << "Can't open sqlite database" << d->database.lastError() << path;
        return false;
    }

    initDatabaseSchema();

    d->closeDanglingActivityRecords();

    activityChanged(sharedInfo()->currentActivity());
    connect(sharedInfo(), SIGNAL(currentActivityChanged(QString)),
            this, SLOT(activityChanged(QString)));
    connect(Location::self(this), SIGNAL(currentChanged(QString)), this, SLOT(locationChanged(QString)));

    #ifdef AR_FAKE_EVENTS_FEED
    d->FakeEventsFeed();
    #endif // AR_FAKE_EVENTS_FEED

    return true;
}

#define ACTIVITYRANKING_SCHEMA_VERSION "1.0"
void ActivityRankingPlugin::initDatabaseSchema()
{
    bool schemaUpToDate = false;

    QSqlQuery query("SELECT value FROM SchemaInfo WHERE key = 'version'", d->database);
    if (query.next()) {
        schemaUpToDate = (ACTIVITYRANKING_SCHEMA_VERSION == query.value(0).toString());
    }

    if (!schemaUpToDate) {
        // We just need to initialize - will handle updates if needed in post 1.0

        query.exec("CREATE TABLE IF NOT EXISTS SchemaInfo (key text PRIMARY KEY, value text)");

        query.exec(Private::insertSchemaInfo.arg("version", ACTIVITYRANKING_SCHEMA_VERSION));

        // flags
        // 1 - active segment
        // 2 - in the same week as an active segment
        // 4 - next to an active segment

        query.exec("CREATE TABLE IF NOT EXISTS WeekScores ("
                   "activity text, year int, week int, "
                   "s00 double default 0, s01 double default 0, s02 double default 0, s03 double default 0, s04 double default 0, s05 double default 0, s06 double default 0, s07 double default 0, "
                   "s10 double default 0, s11 double default 0, s12 double default 0, s13 double default 0, s14 double default 0, s15 double default 0, s16 double default 0, s17 double default 0, "
                   "s20 double default 0, s21 double default 0, s22 double default 0, s23 double default 0, s24 double default 0, s25 double default 0, s26 double default 0, s27 double default 0, "
                   "s30 double default 0, s31 double default 0, s32 double default 0, s33 double default 0, s34 double default 0, s35 double default 0, s36 double default 0, s37 double default 0, "
                   "s40 double default 0, s41 double default 0, s42 double default 0, s43 double default 0, s44 double default 0, s45 double default 0, s46 double default 0, s47 double default 0, "
                   "s50 double default 0, s51 double default 0, s52 double default 0, s53 double default 0, s54 double default 0, s55 double default 0, s56 double default 0, s57 double default 0, "
                   "s60 double default 0, s61 double default 0, s62 double default 0, s63 double default 0, s64 double default 0, s65 double default 0, s66 double default 0, s67 double default 0, "
                   "f0 int default 0, "
                   "f1 int default 0, "
                   "f2 int default 0, "
                   "f3 int default 0, "
                   "f4 int default 0, "
                   "f5 int default 0, "
                   "f6 int default 0, "
                   "location text default NULL, "
                   "PRIMARY KEY(activity, year, week, location)"
                   ")");


        // flags - nothing yet :)
        query.exec("CREATE TABLE IF NOT EXISTS MonthScores ("
                   "activity text, year int, month int, "
                   "s00 double default 0, s01 double default 0, s02 double default 0, s03 double default 0, s04 double default 0, s05 double default 0, s06 double default 0, s07 double default 0, "
                   "s10 double default 0, s11 double default 0, s12 double default 0, s13 double default 0, s14 double default 0, s15 double default 0, s16 double default 0, s17 double default 0, "
                   "s20 double default 0, s21 double default 0, s22 double default 0, s23 double default 0, s24 double default 0, s25 double default 0, s26 double default 0, s27 double default 0, "
                   "s30 double default 0, s31 double default 0, s32 double default 0, s33 double default 0, s34 double default 0, s35 double default 0, s36 double default 0, s37 double default 0, "
                   "s40 double default 0, s41 double default 0, s42 double default 0, s43 double default 0, s44 double default 0, s45 double default 0, s46 double default 0, s47 double default 0, "
                   "s50 double default 0, s51 double default 0, s52 double default 0, s53 double default 0, s54 double default 0, s55 double default 0, s56 double default 0, s57 double default 0, "
                   "s60 double default 0, s61 double default 0, s62 double default 0, s63 double default 0, s64 double default 0, s65 double default 0, s66 double default 0, s67 double default 0, "
                   "s70 double default 0, s71 double default 0, s72 double default 0, s73 double default 0, s74 double default 0, s75 double default 0, s76 double default 0, s77 double default 0, "
                   "f00 int default 0, "
                   "location text default NULL, "
                   "PRIMARY KEY(activity, year, month, location)"
                   ")");

        query.exec("CREATE TABLE IF NOT EXISTS ActivityEvents (activity text, location text, start bigint, end bigint DEFAULT NULL)");
    }
}

ActivityRankingPlugin::~ActivityRankingPlugin()
{
    d->database.close();
    delete d;
}

void ActivityRankingPlugin::activityChanged(const QString & activity)
{
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();

    kDebug() << ">>>> we have the new activity" << activity;

    if (!d->activity.isEmpty()) {
        QSqlQuery query(
            Private::closeActivityInterval
                .arg(currentTime)
                .arg(d->activity)
            ,
                d->database
            );
        PRINT_LAST_ERROR;

        d->processActivityInterval(d->activity, d->lastLocation, d->activityStart, currentTime);
    }

    d->activity = activity;
    d->lastLocation = Location::self(this)->current();
    d->activityStart = currentTime;

    QSqlQuery query(Private::insertActivityInterval
            .arg(activity)
            .arg(d->lastLocation)
            .arg(currentTime)
            ,
            d->database
        );
    PRINT_LAST_ERROR;
    emit rankingChanged(topActivities(), activities());
}

void ActivityRankingPlugin::locationChanged(const QString &location)
{
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();

    kDebug() << ">>>> we have the new location" << location;

    if (!d->activity.isEmpty()) {
        QSqlQuery query(
            Private::closeActivityInterval
                .arg(currentTime)
                .arg(d->activity)
            ,
                d->database
            );
        PRINT_LAST_ERROR;

        d->processActivityInterval(d->activity, d->lastLocation, d->activityStart, currentTime);
    }

    d->lastLocation = location;

    QSqlQuery query(Private::insertActivityInterval
            .arg(d->activity)
            .arg(d->lastLocation)
            .arg(currentTime)
            ,
            d->database
        );
    PRINT_LAST_ERROR;
    emit rankingChanged(topActivities(), activities());
}


QStringList ActivityRankingPlugin::topActivities()
{
    return d->topActivitiesFor(QDateTime::currentDateTime(), d->lastLocation).keys();
}

QMap <QString, qreal> ActivityRankingPlugin::Private::topActivitiesFor(const QDateTime & time, const QString & location)
{
    QMap <QString, qreal> result;

    if (location.isEmpty()) {
        kWarning() << "location should not be empty";
    }

    // We want to get the scores for the current week segment
    const QDateTime monthStartDateTime(QDate(time.date().year(), time.date().month(), 1));
    const qint64 monthStart = monthStartDateTime.toMSecsSinceEpoch();
    const qint64 monthEnd   = monthStartDateTime.addMonths(1).toMSecsSinceEpoch();

    const int coefStart = (time.toMSecsSinceEpoch() - monthStart) / (qreal) (monthEnd - monthStart) * 64;

    QString monthSegment = QString::number(coefStart, 8);
    if (monthSegment.size() == 1)
        monthSegment = '0' + monthSegment;

    QSqlQuery query(
        selectScore
            .arg(time.date().dayOfWeek() - 1)
            .arg(time.time().hour() / 3)
            .arg(monthSegment)
            .arg(location)
        ,
            database
        );
    PRINT_LAST_ERROR;

    while (query.next()) {
        QSqlRecord record = query.record();
        result[record.value(0).toString()] = record.value(1).toDouble();
    }

    return result;
}

QList < ActivityData > ActivityRankingPlugin::activities()
{
    QList < ActivityData > result;

    QMap <QString, qreal> topActivities = d->topActivitiesFor(QDateTime::currentDateTime(), d->lastLocation);
    QMap <QString, qreal>::const_iterator it;

    for (it = topActivities.constBegin(); it != topActivities.constEnd(); ++it) {
        ActivityData data;
        data.id = it.key();
        data.score = it.value();
        result.append(data);
    }

    return result;
}

void ActivityRankingPlugin::test()
{
    int year = 2011;

    for (int month = 10; month <= 12; month++) {
        for (int day = 0; day < 22; day++) {
            for (int hour = 0; hour < 24; hour += 4) {
                QDateTime test = QDateTime(QDate(year, month, day), QTime(hour, 10));

                // qDebug() << d->topActivitiesFor(test).values();

            }
        }
    }
}

KAMD_EXPORT_PLUGIN(ActivityRankingPlugin, "activitymanger_plugin_activityranking")

#include "activityranking.moc"
