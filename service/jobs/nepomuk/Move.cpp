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
#include <Nepomuk/Vocabulary/NIE>
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

namespace Private {

    void replaceUrl(File & file, const QString & destination)
    {
        kDebug();
        // Remove some properties

        QString path = destination + '/' + file.url().fileName();
        kDebug() << "Future location: " << path;

        Resource(file).setProperty(NIE::url(), KUrl(path));
    }

    void unlinkOtherActivities(Resource & resource, const QString & activity)
    {
        kDebug();
        // Find all activities that this resource are linked to
        // and remove the links for all except for the current

        const QString query = QString::fromLatin1(
                "select ?activity where { ?activity a kao:Activity . ?activity nao:isRelated %1");

        // First we are moving the directories linked to the activity
        Soprano::QueryResultIterator it
            = ResourceManager::instance()->mainModel()->executeQuery(
                query.arg(
                    Soprano::Node::resourceToN3(resource.resourceUri())
                ),
                Soprano::Query::QueryLanguageSparql);

        while (it.next()) {
            Resource activityRes(it[0].uri());

            if (activityRes.identifiers().contains(activity)) continue;

            activityRes.removeProperty(NAO::isRelated(), resource);
        }
    }

    void removeSensitiveData(Resource & resource)
    {
        kDebug();
        // Remove some properties

        Resource r(resource);

        r.removeProperty(NAO::hasSubResource());
        r.removeProperty(NFO::hasHash());

        r.removeProperty(NFO::wordCount());
        r.removeProperty(NFO::characterCount());
        r.removeProperty(NFO::lineCount());
        r.removeProperty(NIE::plainTextContent());
        r.removeProperty(NIE::contentSize());
    }

} // namespace Private

CollectFilesToMove::CollectFilesToMove(const QString & activity, const QString & destination)
    : m_activity(activity), m_destination(destination)
{
    kDebug();

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

        Private::unlinkOtherActivities(result, m_activity);
        Private::removeSensitiveData(result);

        scheduleMoveDir(result);

        Private::replaceUrl(result, m_destination);
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

        Private::unlinkOtherActivities(result, m_activity);
        Private::removeSensitiveData(result);

        scheduleMoveFile(result);

        Private::replaceUrl(result, m_destination);
    }

    it.close();

    emit result(m_scheduledForMoving);
}


Move::JOB_FACTORY(const QString & activity, bool toEncrypted, const QStringList & files)
{
    JOB_FACTORY_PROPERTY(activity);
    JOB_FACTORY_PROPERTY(toEncrypted);
    JOB_FACTORY_PROPERTY(files);
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

QStringList Move::files() const
{
    return m_files;
}

void Move::setFiles(const QStringList & files)
{
    m_files = files;
}

void Move::start()
{
    kDebug() << ">>> Move" << m_activity;

    if (m_files.isEmpty()) {
        CollectFilesToMove * job = new CollectFilesToMove(m_activity, destination());

        connect(job, SIGNAL(result(KUrl::List)),
                this, SLOT(moveFiles(KUrl::List)),
                Qt::QueuedConnection);

        job->start();

    } else {
        KUrl::List files;
        const QString & m_destination = destination();

        foreach (const QString & file, m_files) {
            KUrl url(file);

            files << file;

            ::Nepomuk::File result(url);

            Jobs::Nepomuk::Private::unlinkOtherActivities(result, m_activity);
            Jobs::Nepomuk::Private::removeSensitiveData(result);
            Jobs::Nepomuk::Private::replaceUrl(result, m_destination);

        }

        moveFiles(files);
    }
}

void Move::moveFiles(const KUrl::List & list)
{
    KIO::CopyJob * job = KIO::move(list, KUrl(destination()), KIO::HideProgressInfo);

    connect(job, SIGNAL(result(KJob *)),
            this, SLOT(emitResult()));
}

QString Move::destination() const
{
    QString result;

    using namespace Jobs::Encryption::Common;

    if (m_toEncrypted) {
        result = path(m_activity, MountPointFolder);
    } else {
        result = path(m_activity, NormalFolder);
    }

    result.append("/user");

    return result;
}

void Move::emitResult()
{
    Job::emitResult();
}

} // namespace Nepomuk
} // namespace Jobs

#endif

