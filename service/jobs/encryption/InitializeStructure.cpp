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

#include "InitializeStructure.h"
#include "Common.h"

#include <jobs/ui/AskPassword.h>

#include <QDir>

#include <KIO/DeleteJob>
#include <KIO/CopyJob>
#include <KJob>
#include <KUrl>
#include <KDebug>

namespace Jobs {
namespace Encryption {

InitializeStructure::JOB_FACTORY(const QString & activity, int action)
{
    JOB_FACTORY_PROPERTY(activity);
    JOB_FACTORY_PROPERTY(action);
}

QString InitializeStructure::activity() const
{
    return m_activity;
}

void InitializeStructure::setActivity(const QString & activity)
{
    m_activity = activity;
}

int InitializeStructure::action() const
{
    return m_action;
}

void InitializeStructure::setAction(int value)
{
    m_action = value;
}

void InitializeStructure::start()
{
    kDebug() << m_activity << m_action;

    switch (m_action) {
        case InitializeStructure::InitializeInEncrypted:
            move(
                Common::folderPath(m_activity, Common::NormalFolder),
                Common::folderPath(m_activity, Common::MountPointFolder)
            );

            break;

        case InitializeStructure::InitializeInNormal:
            move(
                Common::folderPath(m_activity, Common::MountPointFolder),
                Common::folderPath(m_activity, Common::NormalFolder)
            );

            break;

        case InitializeStructure::DeinitializeEncrypted:
            del(QStringList()
                    << Common::folderPath(m_activity, Common::MountPointFolder)
                    << Common::folderPath(m_activity, Common::EncryptedFolder)
                );
            break;

        case InitializeStructure::DeinitializeNormal:
            del(QStringList()
                    << Common::folderPath(m_activity, Common::NormalFolder)
                );
            break;

        case InitializeStructure::DeinitializeBoth:
            del(QStringList()
                    << Common::folderPath(m_activity, Common::MountPointFolder)
                    << Common::folderPath(m_activity, Common::EncryptedFolder)
                    << Common::folderPath(m_activity, Common::NormalFolder)
                );
            break;
    }
}

void InitializeStructure::del(const QStringList & items)
{
    KUrl::List toDelete;
    QDir dir;

    foreach (const QString & item, items) {
        if (dir.exists(item)) {
            kDebug() << item;
            toDelete << item;
        }
    }

    if (toDelete.size()) {
        startJob(KIO::del(toDelete, KIO::HideProgressInfo));
    } else {
        setError(KJob::NoError);
        emitResult();
    }
}

void InitializeStructure::move(const QString & source, const QString & destination)
{
    QDir destinationDir(destination);
    QDir sourceDir(source);

    destinationDir.mkdir(destinationDir.path());

    KUrl::List toMove;

    for (int i = Common::FirstFolderType; i <= Common::LastFolderType; i++) {
        const QString & folder = Common::folderName((Common::FolderType)i);

        destinationDir.mkdir(folder);

        if (sourceDir.exists(folder)) {
            toMove << sourceDir.filePath(folder);
        }
    }

    if (toMove.size()) {
        startJob(
            KIO::move(
                toMove,
                KUrl(destination),
                KIO::HideProgressInfo | KIO::Overwrite
            )
        );
    } else {
        emitResult();
    }
}

void InitializeStructure::startJob(KJob * job)
{
    connect(job, SIGNAL(finished(KJob*)),
            this, SLOT(jobFinished(KJob*)));
}

void InitializeStructure::jobFinished(KJob * job)
{
    setError(job->error());
    emitResult();
}

} // namespace Enc
} // namespace Jobs

