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

#include "KioActivities.h"

#include <sys/types.h>
#include <unistd.h>

#include <QCoreApplication>

#include <QDebug>
#include <QFile>
#include <QDir>

#include <KFileItem>
#include <KStandardDirs>

#include <Nepomuk2/ResourceManager>
#include <Soprano/QueryResultIterator>
#include <Soprano/Model>
#include <Soprano/Node>

#include <KUrl>
#include <KUser>
#include <KComponentData>

#include "kao.h"

#include <Nepomuk2/Resource>
#include <Nepomuk2/Query/Query>
#include <Nepomuk2/Query/FileQuery>
#include <Nepomuk2/Query/ResourceTerm>
#include <Nepomuk2/Query/ComparisonTerm>

#include "lib/core/info.h"

#include <Soprano/Vocabulary/NAO>

namespace Nepomuk = Nepomuk2;
using namespace Soprano::Vocabulary;
using namespace KDE::Vocabulary;
using namespace KIO;

class ActivitiesProtocol::Private {
public:
    Private(ActivitiesProtocol * parent)
        : kio(parent)
    {
    }

    enum Path {
        RootItem,
        ActivityRootItem,
        ActivityPathItem,
        PrivateActivityPathItem
    };

    KActivities::Consumer activities;
    QString activity;
    QString filename;

    inline QString realActivityId(const QString & activity) const
    {
        if (activity == "current") {
            return activities.currentActivity();
        }

        return activity;
    }

    KIO::UDSEntry createFolderUDSEntry(const QString & name, const QString & displayName, const QDate & date) const
    {
        KIO::UDSEntry uds;
        QDateTime dt(date, QTime(0, 0, 0));
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

    KIO::UDSEntry createUDSEntryForUrl(const KUrl & url) const
    {
        KIO::UDSEntry uds;

        KFileItem fileItem(KFileItem::Unknown, KFileItem::Unknown, url, false);

        QByteArray encodedPath = QUrl::toPercentEncoding(url.url());

        uds.insert(KIO::UDSEntry::UDS_NAME, QString::fromUtf8(encodedPath));
        uds.insert(KIO::UDSEntry::UDS_DISPLAY_NAME, fileItem.name());
        uds.insert(KIO::UDSEntry::UDS_MIME_TYPE, fileItem.mimetype());
        uds.insert(KIO::UDSEntry::UDS_SIZE, fileItem.size());
        uds.insert(KIO::UDSEntry::UDS_MODIFICATION_TIME, fileItem.time(KFileItem::ModificationTime).toTime_t());
        uds.insert(KIO::UDSEntry::UDS_CREATION_TIME,     fileItem.time(KFileItem::CreationTime).toTime_t());
        uds.insert(KIO::UDSEntry::UDS_ACCESS, fileItem.permissions());
        uds.insert(KIO::UDSEntry::UDS_USER, KUser().loginName());
        uds.insert(KIO::UDSEntry::UDS_LOCAL_PATH, url.toLocalFile());
        uds.insert(KIO::UDSEntry::UDS_TARGET_URL, url.prettyUrl());

        if (fileItem.isDir())
            uds.insert(KIO::UDSEntry::UDS_FILE_TYPE, S_IFDIR);

        return uds;
    }

    Nepomuk::Query::Query buildQuery(const QString & activity) const
    {
        Nepomuk::Resource activityResource(activity, KAO::Activity());

        Nepomuk::Query::FileQuery query;

        Nepomuk::Query::ComparisonTerm usedActivity(NAO::isRelated(), Nepomuk::Query::ResourceTerm(activityResource));
        usedActivity.setInverted(true);

        query.setTerm(usedActivity);

        return query;
    }

    void listActivities() const
    {
        kio->listEntry(createFolderUDSEntry(
                QString::fromLatin1("current"),
                i18n("Current activity"),
                QDate::currentDate()), false
            );

        foreach (const QString & activity, activities.listActivities()) {
            kio->listEntry(createFolderUDSEntry(
                    activity,
                    KActivities::Info::name(realActivityId(activity)),
                    QDate::currentDate()), false
                );
        }

        kio->listEntry(KIO::UDSEntry(), true);
        kio->finished();
    }

    void listActivity() const
    {
        // We need this to be changeable in a const method
        QString activity = realActivityId(this->activity);

        if (!activity.isEmpty()) {
            Nepomuk::Resource activityResource(activity, KAO::Activity());

            const QString query = QString::fromLatin1(
                    "select distinct ?r, ?url where { "
                    "     ?r a nfo:FileDataObject . "
                    "     ?r nie:url ?url . "
                    "     %1 nao:isRelated ?r . "
                    "} "
                );

            Soprano::QueryResultIterator it
                = Nepomuk::ResourceManager::instance()->mainModel()->executeQuery(
                    query.arg(Soprano::Node::resourceToN3(activityResource.uri())),
                    Soprano::Query::QueryLanguageSparql);

            while (it.next()) {
                QUrl resource = it[0].uri();
                QUrl file = it[1].uri();


                kio->listEntry(createUDSEntryForUrl(it[1].uri()), false);

            }

            it.close();

        }


        // kio->listEntry(createFolderUDSEntry(
        //             "_test", "_Test", QDate::currentDate()), false);
        kio->listEntry(KIO::UDSEntry(), true);
        kio->finished();
    }

    Path parseUrl(const KUrl & url)
    {
        activity.clear();
        filename.clear();

        if (url.path().length() <= 1) {
            return RootItem;
        }

        QStringList path = url.path().split('/', QString::SkipEmptyParts);

        if (path.isEmpty()) {
            return RootItem;
        }

        activity = path.takeFirst();

        if (path.isEmpty()) {
            return (KActivities::Info(realActivityId(activity)).isEncrypted())
                ? PrivateActivityPathItem : ActivityRootItem;
        }

        filename = path.join("/");

        return (KActivities::Info(realActivityId(activity)).isEncrypted())
            ? PrivateActivityPathItem : ActivityRootItem;
    }

private:
    ActivitiesProtocol * const kio;
};


ActivitiesProtocol::ActivitiesProtocol(const QByteArray & poolSocket, const QByteArray & appSocket)
    : KIO::ForwardingSlaveBase("activities", poolSocket, appSocket), d(new Private(this))
{
}


ActivitiesProtocol::~ActivitiesProtocol()
{
    delete d;
}


void ActivitiesProtocol::listDir(const KUrl & url)
{
    // TODO: Test whether necessary services are running
    if (false) {
        error(KIO::ERR_SLAVE_DEFINED, i18n("Activity manager is not running properly."));
        return;
    }

    switch (d->parseUrl(url)) {
        case Private::RootItem:
            d->listActivities();
            break;

        case Private::ActivityRootItem:
            d->listActivity();
            break;

        case Private::ActivityPathItem:
        case Private::PrivateActivityPathItem:
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
    if (d->parseUrl(url)) {
        ForwardingSlaveBase::get(url);

    } else {
        error(KIO::ERR_DOES_NOT_EXIST, url.prettyUrl());

    }
}


void ActivitiesProtocol::put(const KUrl & url, int permissions, KIO::JobFlags flags)
{
    if (d->parseUrl(url)) {
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
    Q_UNUSED(url);
    Q_UNUSED(isfile);
    // qDebug() << url;
    // ForwardingSlaveBase::del(url, isfile);

    error(ERR_UNSUPPORTED_ACTION, url.prettyUrl());
}


void ActivitiesProtocol::mimetype(const KUrl & url)
{
    // qDebug() << url;
    ForwardingSlaveBase::mimetype(url);
}


void ActivitiesProtocol::stat(const KUrl & url)
{
    switch (d->parseUrl(url)) {
        case Private::RootItem:
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

        case Private::ActivityRootItem:
        {
            KIO::UDSEntry uds;
            uds.insert(KIO::UDSEntry::UDS_NAME, d->activity);
            uds.insert(KIO::UDSEntry::UDS_FILE_TYPE, S_IFDIR);
            uds.insert(KIO::UDSEntry::UDS_MIME_TYPE, QString::fromLatin1("inode/directory"));
            statEntry(uds);
            finished();
            break;
        }

        case Private::ActivityPathItem:
        case Private::PrivateActivityPathItem:
        {
            // QString path = QUrl::fromPercentEncoding(d->filename.toUtf8());
            // statEntry(d->createUDSEntryForUrl(KUrl(path)));
            // finished();

            ForwardingSlaveBase::stat(url);

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
    Private::Path pathType = d->parseUrl(url);

    if (pathType == Private::ActivityPathItem) {
        QString path = QUrl::fromPercentEncoding(d->filename.toUtf8());
        newURL = KUrl(path);

        return true;
    }

    if (pathType == Private::PrivateActivityPathItem) {
        static QDir activitiesDataFolder = QDir(KStandardDirs::locateLocal("data", "activitymanager/activities"));

        newURL = KUrl("file://" + activitiesDataFolder.filePath("crypt-" + d->activity + "/user/" + d->filename));

        return true;
    }

    return false;
}


void ActivitiesProtocol::prepareUDSEntry(KIO::UDSEntry & entry,
                                                 bool listing) const
{
    ForwardingSlaveBase::prepareUDSEntry(entry, listing);
}

extern "C"
{
    KDE_EXPORT int kdemain(int argc, char **argv)
    {
        // necessary to use other kio slaves
        KComponentData("kio_activities");
        QCoreApplication app(argc, argv);

        // qDebug() << "Starting activities slave " << getpid();

        if (argc != 4) {
            // kError() << "Usage: kio_activities protocol domain-socket1 domain-socket2";
            exit(-1);
        }

        ActivitiesProtocol slave(argv[2], argv[3]);
        slave.dispatchLoop();

        // qDebug() << "activities slave Done";

        return 0;
    }
}

#include "KioActivities.moc"
