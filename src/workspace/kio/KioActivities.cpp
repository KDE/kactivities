/*
 * Copyright 2012, 2013, 2014 Ivan Cukic <ivan.cukic@kde.org>
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

#include <QCoreApplication>

#include <QDebug>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlField>
#include <QSqlError>
#include <QSqlDriver>

#include <KLocalizedString>
#include <KUser>
#include <kio/job.h>

#include <utils/d_ptr_implementation.h>
#include <utils/qsqlquery.h>
#include <common/database/Database.h>

#include "lib/core/info.h"
#include "lib/core/consumer.h"

#include <QProcess>

class ActivitiesProtocol::Private {
public:
    Private(ActivitiesProtocol *parent)
        : kio(parent)
    {
    }

    enum PathType {
        RootItem,
        ActivityRootItem,
        ActivityPathItem
    };

    PathType pathType(const QUrl &url, QString *activity = Q_NULLPTR,
                      QString *filePath = Q_NULLPTR) const
    {
        const auto fullPath = url.adjusted(QUrl::StripTrailingSlash).path();
        const auto path = fullPath.midRef(fullPath.startsWith('/') ? 1 : 0);

        if (activity) {
            *activity = path.mid(0, path.indexOf("/") - 1).toString();
        }

        if (filePath) {
            *filePath = path.mid(path.indexOf("/")).toString();
        }

        return path.length() == 0 ? RootItem
             : path.contains("/") ? ActivityPathItem
             : ActivityRootItem;
    }

    void syncActivities(KActivities::Consumer &activities)
    {
        // We need to use the consumer in a synchronized way
        while (activities.serviceStatus() == KActivities::Consumer::Unknown) {
            QCoreApplication::processEvents();
        }
    }

    KIO::UDSEntry activityEntry(const QString &activity)
    {
        KIO::UDSEntry uds;
        KActivities::Info activityInfo(activity);
        uds.insert(KIO::UDSEntry::UDS_NAME, activity);
        uds.insert(KIO::UDSEntry::UDS_DISPLAY_NAME, activityInfo.name());
        uds.insert(KIO::UDSEntry::UDS_DISPLAY_TYPE, i18n("Activity"));
        uds.insert(KIO::UDSEntry::UDS_ICON_NAME, activityInfo.icon());
        uds.insert(KIO::UDSEntry::UDS_FILE_TYPE, S_IFDIR);
        uds.insert(KIO::UDSEntry::UDS_MIME_TYPE, QStringLiteral("inode/directory"));
        uds.insert(KIO::UDSEntry::UDS_ACCESS, 0500);
        uds.insert(KIO::UDSEntry::UDS_USER, KUser().loginName());
        return uds;
    }

    KIO::UDSEntry filesystemEntry(const QString &path)
    {
        KIO::UDSEntry uds;
        auto url = QUrl::fromLocalFile(path);

        if (KIO::StatJob* job = KIO::stat(url, KIO::HideProgressInfo)) {
            QScopedPointer<KIO::StatJob> sp(job);
            job->setAutoDelete(false);
            if (job->exec()) {
                uds = job->statResult();
            }
        }

        QString mangled = path;
        mangled.replace("/", "_");
        uds.insert(KIO::UDSEntry::UDS_NAME, mangled);
        uds.insert(KIO::UDSEntry::UDS_DISPLAY_NAME, url.fileName());
        uds.insert(KIO::UDSEntry::UDS_TARGET_URL, url.url());
        uds.insert(KIO::UDSEntry::UDS_LOCAL_PATH, path);
        uds.insert(KIO::UDSEntry::UDS_LINK_DEST, path);

        return uds;
    }

    // KActivities::Consumer activities;

private:
    ActivitiesProtocol *const kio;
};



extern "C" int Q_DECL_EXPORT kdemain(int argc, char **argv)
{
    // necessary to use other kio slaves
    QCoreApplication app(argc, argv);
    if (argc != 4) {
        fprintf(stderr, "Usage: kio_activities protocol domain-socket1 domain-socket2\n");
        exit(-1);
    }
    // start the slave
    ActivitiesProtocol slave(argv[2], argv[3]);
    slave.dispatchLoop();
    return 0;
}


ActivitiesProtocol::ActivitiesProtocol(const QByteArray &poolSocket,
                                       const QByteArray &appSocket)
    : KIO::ForwardingSlaveBase("activities", poolSocket, appSocket)
    , d(this)
{
}

ActivitiesProtocol::~ActivitiesProtocol()
{
}

bool ActivitiesProtocol::rewriteUrl(const QUrl &url, QUrl &newUrl)
{
    QString activity, path;
    switch (d->pathType(url, &activity, &path)) {
        case Private::RootItem:
        case Private::ActivityRootItem:
            return false;

        case Private::ActivityPathItem:
            newUrl = QUrl::fromLocalFile(path);

        default:
            return true;
    }
}

void ActivitiesProtocol::listDir(const QUrl &url)
{
    KActivities::Consumer activities;
    d->syncActivities(activities);

    QString activity, path;
    switch (d->pathType(url, &activity, &path)) {
        case Private::RootItem:
        {
            KIO::UDSEntryList udslist;
            for (const auto activity: activities.activities()) {
                udslist << d->activityEntry(activity);
            }
            listEntries(udslist);
            finished();
            break;
        }

        case Private::ActivityRootItem:
        {
            KIO::UDSEntryList udslist;

            auto database = Database::instance(Database::ResourcesDatabase);

            if (!database) {
                finished();
                break;
            }

            static const auto queryString = QStringLiteral(
                "SELECT targettedResource "
                "FROM ResourceLink "
                "WHERE usedActivity = '%1' "
                    "AND initiatingAgent = \"\" "
                );

            auto query = database->query(queryString.arg(activity));

            for (const auto& result: query) {
                auto path = result[0].toString();

                if (!QFile(path).exists()) continue;

                KIO::UDSEntry uds;

                udslist << d->filesystemEntry(path);
            }

            listEntries(udslist);
            finished();
            break;
        }

        case Private::ActivityPathItem:
            ForwardingSlaveBase::listDir(QUrl::fromLocalFile(path));
            break;
    }
}

void ActivitiesProtocol::prepareUDSEntry(KIO::UDSEntry &entry, bool listing) const
{
    ForwardingSlaveBase::prepareUDSEntry(entry, listing);
}

void ActivitiesProtocol::stat(const QUrl& url)
{
    QString activity;

    switch (d->pathType(url, &activity)) {
        case Private::RootItem:
        {
            QString dirName = i18n("Activities");
            KIO::UDSEntry uds;
            uds.insert(KIO::UDSEntry::UDS_NAME, dirName);
            uds.insert(KIO::UDSEntry::UDS_DISPLAY_NAME, dirName);
            uds.insert(KIO::UDSEntry::UDS_DISPLAY_TYPE, dirName);
            uds.insert(KIO::UDSEntry::UDS_ICON_NAME, QStringLiteral("preferences-activities"));
            uds.insert(KIO::UDSEntry::UDS_FILE_TYPE, S_IFDIR);
            uds.insert(KIO::UDSEntry::UDS_MIME_TYPE, QStringLiteral("inode/directory"));

            statEntry(uds);
            finished();
            break;
        }

        case Private::ActivityRootItem:
        {
            KActivities::Consumer activities;
            d->syncActivities(activities);
            statEntry(d->activityEntry(activity));
            finished();
            break;
        }

        case Private::ActivityPathItem:
            ForwardingSlaveBase::stat(url);
            break;
    }
}

void ActivitiesProtocol::mimetype(const QUrl& url)
{
    switch (d->pathType(url)) {
        case Private::RootItem:
        case Private::ActivityRootItem:
            mimeType(QStringLiteral("inode/directory"));
            finished();
            break;

        case Private::ActivityPathItem:
            ForwardingSlaveBase::mimetype(url);
            break;
    }

}

void ActivitiesProtocol::del(const QUrl& url, bool isfile)
{
}


#include "KioActivities.moc"
