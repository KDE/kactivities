/*
 *   Copyright (C) 2012 Ivan Cukic <ivan.cukic(at)kde.org>
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

#include "Move.h"

#ifdef HAVE_NEPOMUK
#include "../../ui/Ui.h"

#include <KDebug>
#include <QThread>
#include <QFile>

#include <Soprano/QueryResultIterator>
#include <Soprano/Node>
#include <Soprano/Model>

#include <Nepomuk/Variant>
#include <Nepomuk/ResourceManager>
#include <Nepomuk/Resource>
#include <Nepomuk/File>

#include <Nepomuk/Query/Query>
#include <Nepomuk/Query/ResourceTypeTerm>
#include <Nepomuk/Query/ResourceTerm>
#include <Nepomuk/Query/ComparisonTerm>
#include <Nepomuk/Types/Property>

#include <kao.h>
#include <Nepomuk/Vocabulary/NFO>
#include <Soprano/Vocabulary/NAO>

#include <KIO/CopyJob>
#include <jobs/encryption/Common.h>
#include <KUrl>

using namespace Soprano::Vocabulary;
using namespace Nepomuk::Vocabulary;
using namespace Nepomuk::Query;
using namespace Nepomuk;

namespace Jobs {
namespace Nepomuk {

CollectFilesToMove::CollectFilesToMove(const QString & activity)
    : m_activity(activity)
{
    kDebug();

}

void CollectFilesToMove::unlinkOtherActivities(Resource & resource)
{
    kDebug();
    // Find all activities that this resource are linked to
    // and remove the links for all except for the current

}

void CollectFilesToMove::removeSensitiveData(Resource & resource)
{
    kDebug();
    // Remove some properties

}

void CollectFilesToMove::scheduleMoveDir(File & dir)
{
    kDebug();
    QString path = dir.url().toLocalFile();
    if (!path.endsWith("/")) {
        path += "/";
    }
    m_movedDirs << path;

    scheduleMove(dir);
}

void CollectFilesToMove::scheduleMoveFile(File & file)
{
    kDebug();
    QString path = file.url().toLocalFile();
    foreach (const QString & dir, m_movedDirs) {
        if (path.startsWith(dir)) return;
    }

    scheduleMove(file);
}

void CollectFilesToMove::scheduleMove(File & item)
{
    kDebug() << item.url().toLocalFile();

    m_scheduledForMoving << item.url().toLocalFile();

}

// void move()
// {
//     kDebug() << m_scheduledForMoving << m_destination;

//     foreach (const KUrl & url, m_scheduledForMoving) {
//         const QString & path = url.toLocalFile();

//         kDebug() << QFile::exists(path) << path;

//     }

//     kDebug() << QFile::exists(m_destination) << m_destination;

//     KIO::CopyJob * job = KIO::move(m_scheduledForMoving, KUrl(m_destination), KIO::HideProgressInfo);

//     connect(job, SIGNAL(result(KJob *)),
//             m_target, SLOT(errorReturned(KJob *)));
//     connect(job, SIGNAL(result(KJob *)),
//             m_target, m_method,
//             Qt::QueuedConnection);
// }

void CollectFilesToMove::run()
{
    kDebug();
    // We need to get only the stuff that are related to our activity
    Resource activity(m_activity, KAO::Activity());
    ComparisonTerm related = ComparisonTerm(NAO::isRelated(), ResourceTerm(activity));
    related.setInverted(true);

    // First we are moving the directories linked to the activity
    Soprano::QueryResultIterator it
        = ResourceManager::instance()->mainModel()->executeQuery(
            ::Nepomuk::Query::Query(ResourceTypeTerm(NFO::Folder()) && related).toSparqlQuery(),
            Soprano::Query::QueryLanguageSparql);

    while (it.next()) {
        File result(Resource(it[0].uri()));

        unlinkOtherActivities(result);
        removeSensitiveData(result);
        scheduleMoveDir(result);
    }

    it.close();

    // And now the files that haven't been inside a linked directory
    it = ResourceManager::instance()->mainModel()->executeQuery(
            ::Nepomuk::Query::Query(
                ResourceTypeTerm(NFO::FileDataObject())
                && !ResourceTypeTerm(NFO::Folder())
                && related
                ).toSparqlQuery(),
            Soprano::Query::QueryLanguageSparql);

    while (it.next()) {
        File result(Resource(it[0].uri()));

        unlinkOtherActivities(result);
        removeSensitiveData(result);
        scheduleMoveFile(result);
    }

    it.close();

    emit result(m_scheduledForMoving);
}


Move::JOB_FACTORY(const QString & activity, bool toEncrypted)
{
    JOB_FACTORY_PROPERTY(activity);
    JOB_FACTORY_PROPERTY(toEncrypted);
}

QString Move::activity() const
{
    return m_activity;
}

void Move::setActivity(const QString & activity)
{
    m_activity = activity;
}

bool Move::toEncrypted() const
{
    return m_toEncrypted;
}

void Move::setToEncrypted(bool value)
{
    m_toEncrypted = value;
}

void Move::start()
{
    kDebug() << ">>> Move" << m_activity;
    KIO::move(KUrl("/tmp/ASD"), KUrl("/tmp/asd"), KIO::HideProgressInfo);

    CollectFilesToMove * job = new CollectFilesToMove(m_activity);

    connect(job, SIGNAL(result(KUrl::List)),
            this, SLOT(moveFiles(KUrl::List)),
            Qt::QueuedConnection);

    job->start();
}

void Move::moveFiles(const KUrl::List & list)
{
    using namespace Jobs::Encryption::Common;
    QString destination;
    if (m_toEncrypted) {
        destination = path(m_activity, MountPointFolder);
    } else {
        destination = path(m_activity, NormalFolder);
    }
    destination.append("/user");

    KIO::CopyJob * job = KIO::move(list, KUrl(destination), KIO::HideProgressInfo);

    connect(job, SIGNAL(result(KJob *)),
            this, SLOT(emitResult()));
}

void Move::emitResult()
{
    Job::emitResult();
}

} // namespace Nepomuk
} // namespace Jobs

#endif

