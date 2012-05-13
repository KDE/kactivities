/*
 * Copyright 2012 Ivan Cukic <ivan.cukic@kde.org>
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

#include "kio_activities.h"

#include <sys/types.h>
#include <unistd.h>

#include <QCoreApplication>

#include <QDebug>
#include <QFile>

// #define KIO_ACTIVITIES_DEBUG

#ifdef KIO_ACTIVITIES_DEBUG
class KioDebug {
public:
    QDebug & kioDebug() {
        static QFile log_file("/tmp/kio_activities");
        log_file.flush();
        log_file.close();
        log_file.open(QIODevice::WriteOnly | QIODevice::Append);

        static QDebug log_stream(&log_file);
        return log_stream;
    }
} _kiodebug;

QDebug & kioDebug()
{
    return _kiodebug.kioDebug();
}
#else
QDebug kioDebug()
{
    return qDebug();
}
#endif

#include <KUrl>
#include <KUser>
#include <KComponentData>

#include "kao.h"

#include <Nepomuk/Resource>
#include <Nepomuk/Query/Query>
#include <Nepomuk/Query/FileQuery>
#include <Nepomuk/Query/ResourceTerm>
#include <Nepomuk/Query/ComparisonTerm>

#include "lib/info.h"

#include <Soprano/Vocabulary/NAO>

using namespace Soprano::Vocabulary;
using namespace Nepomuk::Vocabulary;
using namespace KIO;

namespace {
    KIO::UDSEntry createFolderUDSEntry(const QString & name, const QString & displayName, const QDate & date)
    {
        KIO::UDSEntry uds;
        QDateTime dt(date, QTime(0, 0, 0));
        // kDebug() << dt;
        kioDebug() << "ActivitiesProtocol createFolderUDSEntry" << name << displayName << date << '\n';
        uds.insert(KIO::UDSEntry::UDS_NAME, name);
        uds.insert(KIO::UDSEntry::UDS_DISPLAY_NAME, displayName);
        uds.insert(KIO::UDSEntry::UDS_FILE_TYPE, S_IFDIR);
        uds.insert(KIO::UDSEntry::UDS_MIME_TYPE, QString::fromLatin1("inode/directory"));
        uds.insert(KIO::UDSEntry::UDS_MODIFICATION_TIME, dt.toTime_t());
        uds.insert(KIO::UDSEntry::UDS_CREATION_TIME, dt.toTime_t());
        uds.insert(KIO::UDSEntry::UDS_ACCESS, 0700);
        uds.insert(KIO::UDSEntry::UDS_USER, KUser().loginName());
        return uds;
    }

    Nepomuk::Query::Query buildQuery(const QString & activityId)
    {
        kioDebug() << "ActivitiesProtocol building query for" << activityId << '\n';
        Nepomuk::Resource activityResource(activityId, KAO::Activity());

        Nepomuk::Query::FileQuery query;

        Nepomuk::Query::ComparisonTerm usedActivity(NAO::isRelated(), Nepomuk::Query::ResourceTerm(activityResource));
        usedActivity.setInverted(true);

        query.setTerm(usedActivity);

        kioDebug() << "ActivitiesProtocol query: " << query.toSearchUrl();
        kioDebug() << "ActivitiesProtocol query: " << query.toSparqlQuery();

        return query;
    }


}


ActivitiesProtocol::ActivitiesProtocol(const QByteArray & poolSocket, const QByteArray & appSocket)
    : KIO::ForwardingSlaveBase("activities", poolSocket, appSocket)
{
    // kDebug();
    kioDebug() << "ActivitiesProtocol constructor" << '\n';
}


ActivitiesProtocol::~ActivitiesProtocol()
{
    // kDebug();
    kioDebug() << "ActivitiesProtocol destr" << '\n';
}


void ActivitiesProtocol::listDir(const KUrl & url)
{
    // TODO: Test whether necessary services are running
    if (false) {
        error(KIO::ERR_SLAVE_DEFINED, i18n("Activity manager is not running properly."));
        return;
    }

    switch(parseUrl(url)) {
        case RootItem:
        {
            foreach (const QString & activityId, activities.listActivities()) {
                listEntry(createFolderUDSEntry(
                        activityId,
                        KActivities::Info::name(activityId),
                        QDate::currentDate()), false
                    );
            }

            listEntry(KIO::UDSEntry(), true);
            finished();
            break;
        }

        case ActivityRootItem:
        case ActivityPathItem:
            ForwardingSlaveBase::listDir(url);
            break;

        default:
            error(KIO::ERR_DOES_NOT_EXIST, url.prettyUrl());
            break;
    }
}


void ActivitiesProtocol::mkdir(const KUrl & url, int permissions)
{
    Q_UNUSED(permissions);
    error(ERR_UNSUPPORTED_ACTION, url.prettyUrl());
}


void ActivitiesProtocol::get(const KUrl & url)
{
    // kDebug() << url;
    kioDebug() << "ActivitiesProtocol get" << url << '\n';

    if (parseUrl(url)) {
        ForwardingSlaveBase::get(url);

    } else {
        error(KIO::ERR_DOES_NOT_EXIST, url.prettyUrl());

    }
}


void ActivitiesProtocol::put(const KUrl & url, int permissions, KIO::JobFlags flags)
{
    // kDebug() << url;
    kioDebug() << "ActivitiesProtocol put" << url << '\n';

    if (parseUrl(url)) {
        ForwardingSlaveBase::put(url, permissions, flags);
    }
    else {
        error(KIO::ERR_DOES_NOT_EXIST, url.prettyUrl());
    }
}


void ActivitiesProtocol::copy(const KUrl & src, const KUrl & dest, int permissions, KIO::JobFlags flags)
{
    Q_UNUSED(src);
    Q_UNUSED(dest);
    Q_UNUSED(permissions);
    Q_UNUSED(flags);

    error(ERR_UNSUPPORTED_ACTION, src.prettyUrl());
}


void ActivitiesProtocol::rename(const KUrl & src, const KUrl & dest, KIO::JobFlags flags)
{
    Q_UNUSED(src);
    Q_UNUSED(dest);
    Q_UNUSED(flags);

    error(ERR_UNSUPPORTED_ACTION, src.prettyUrl());
}


void ActivitiesProtocol::del(const KUrl & url, bool isfile)
{
    // kDebug() << url;
    ForwardingSlaveBase::del(url, isfile);
}


void ActivitiesProtocol::mimetype(const KUrl & url)
{
    // kDebug() << url;
    ForwardingSlaveBase::mimetype(url);
}


void ActivitiesProtocol::stat(const KUrl & url)
{
    switch(parseUrl(url)) {
        case RootItem:
        {
            KIO::UDSEntry uds;
            uds.insert(KIO::UDSEntry::UDS_NAME, QString::fromLatin1("/"));
            uds.insert(KIO::UDSEntry::UDS_ICON_NAME, QString::fromLatin1("preferences-activities"));
            uds.insert(KIO::UDSEntry::UDS_FILE_TYPE, S_IFDIR);
            uds.insert(KIO::UDSEntry::UDS_MIME_TYPE, QString::fromLatin1("inode/directory"));
            statEntry(uds);
            finished();
            break;
        }

    default:
        error(KIO::ERR_DOES_NOT_EXIST, url.prettyUrl());
        break;
    }
}


// only used for the queries
bool ActivitiesProtocol::rewriteUrl(const KUrl & url, KUrl & newURL)
{
    kioDebug() << "REWRITE URL ----------------------------------\n";
    kioDebug() << "original url" << url.url() << '\n';
    Path pathType = parseUrl(url);

    if (pathType == ActivityRootItem) {
        kioDebug() << "Activity root for" << m_activityId;

        newURL = buildQuery(m_activityId).toSearchUrl();
        return true;

    } else if (pathType == ActivityPathItem) {
        kioDebug() << "Activity path item for" << m_activityId << " " << m_filename << '\n';
        newURL = buildQuery(m_activityId).toSearchUrl();
        newURL.addPath(m_filename);
        return true;

    } else {
        return false;
    }
}


void ActivitiesProtocol::prepareUDSEntry(KIO::UDSEntry & entry,
                                                 bool listing) const
{
    // kDebug() << entry.stringValue(KIO::UDSEntry::UDS_NEPOMUK_URI) << entry.stringValue(KIO::UDSEntry::UDS_MIME_TYPE) << listing;
    ForwardingSlaveBase::prepareUDSEntry(entry, listing);
}


ActivitiesProtocol::Path ActivitiesProtocol::parseUrl(const KUrl & url)
{
    kioDebug() << "parsing ... " << url << '\n';

    if (url.path().length() <= 1) {
        return RootItem;
    }

    QString path = url.path();
    kioDebug() << "path is" << path << '\n';

    if (path.startsWith('/'))
        path.remove(0, 1);
    kioDebug() << "path is now" << path << '\n';

    int slashPos = path.indexOf('/');

    if (slashPos == -1) {
        m_activityId = path;
        m_filename.clear();

        kioDebug() << "Activity is" << m_activityId << "filename is" << m_filename << '\n';
        return ActivityRootItem;

    } else if (slashPos == path.length() - 1) {
        m_activityId = path.left(slashPos - 1);
        m_filename.clear();

        kioDebug() << "Activity is" << m_activityId << "filename is" << m_filename << '\n';
        return ActivityRootItem;

    } else {
        m_activityId = path.left(slashPos - 1);
        m_filename   = path.mid(slashPos + 1);

        kioDebug() << "Activity is" << m_activityId << "filename is" << m_filename << '\n';
        return ActivityPathItem;
    }
}

extern "C"
{
    KDE_EXPORT int kdemain(int argc, char **argv)
    {
        // necessary to use other kio slaves
        KComponentData("kio_activities");
        QCoreApplication app(argc, argv);

        // kDebug() << "Starting activities slave " << getpid();

        if (argc != 4) {
            // kError() << "Usage: kio_activities protocol domain-socket1 domain-socket2";
            exit(-1);
        }

        ActivitiesProtocol slave(argv[2], argv[3]);
        slave.dispatchLoop();

        // kDebug() << "activities slave Done";

        return 0;
    }
}

#include "kio_activities.moc"
