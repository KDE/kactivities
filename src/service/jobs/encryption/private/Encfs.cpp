/*
 *   Copyright (C) 2012 Ivan Cukic <ivan.cukic(at)kde.org>
 *   Copyright (C) 2012 Lamarque V. Souza <lamarque@kde.org>
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

#include "Encfs_p.h"

#include <QDir>
#include <QStringList>

#include <KLocale>
#include <KDebug>
#include <kmountpoint.h>

#include <utils/d_ptr_implementation.h>

namespace Jobs {
namespace Encryption {
namespace Private {

Encfs::Private::Private(Encfs * parent)
    : QObject(parent), q(parent)
{
}

Encfs::Encfs(QObject * parent)
    : QObject(parent), d(this)
{
}

Encfs::~Encfs()
{
    unmountAll();
}

bool Encfs::isEncryptionInitialized(const QString & path) const
{
    QDir dir(path);

    const QStringList & files = dir.entryList(QDir::Hidden | QDir::Files);

    foreach (const QString & file, files) {
        if (file.startsWith(QLatin1String(".encfs"))) {
            return true;
        }
    }

    return false;
}

bool Encfs::isMounted(const QString & path) const
{
    // warning: KMountPoint depends on /etc/mtab according to the documentation.
    KMountPoint::Ptr ptr = KMountPoint::currentMountPoints().findByPath(path);

    if (ptr)
        kDebug() << ptr.data()->mountPoint() << path;

    return (ptr && ptr.data()->mountPoint() == path);
}

void Encfs::unmountAllExcept(const QString & path)
{
    kDebug() << "Unmounting everything except " << path;

    foreach (const QString & mount, d->mounts) {
        if (path != mount) {
            unmount(mount);
        }
    }
}

void Encfs::unmountAll()
{
    kDebug() << "Unmounting everything";

    // TODO: Maybe we should umount all directories, not only
    // what is in d->mounts
    foreach (const QString & mount, d->mounts) {
        QProcess::execute(FUSERMOUNT_PATH, QStringList()
                << "-u" // unmount
                << mount
                );
    }

    d->mounts.clear();
}

QProcess * Encfs::mount(const QString & what, const QString & mountPoint, const QString & password)
{
    kDebug() << "mounting" << what << mountPoint;

    if (isMounted(mountPoint)) {
        kDebug() << mountPoint << "already mounted";
        d->mounts << mountPoint;

        return nullptr;
    }

    bool init = !isEncryptionInitialized(what);

    kDebug() << "Executing" << ENCFS_PATH << " -S"
            << what
            << mountPoint;

    QDir dir;
    dir.mkpath(what);
    dir.mkpath(mountPoint);

    QProcess * encfs = new QProcess(this);
    encfs->setProcessChannelMode(QProcess::ForwardedChannels);
    encfs->setProperty("mountPoint", mountPoint);

    encfs->start(ENCFS_PATH, QStringList()
            // << "-f" // foreground mode // we don't use this anymore
            << "-S" // password read from stdin
            << what
            << mountPoint
        );

    if (init) {
        encfs->write("p\n"); // paranoia mode
    }

    encfs->write(password.toAscii()); // writing the password
    encfs->write("\n");

    d->mounts << mountPoint;

    return encfs;
}

QProcess * Encfs::unmount(const QString & mountPoint)
{
    kDebug() << mountPoint;

    if (!isMounted(mountPoint)) return nullptr;

    kDebug() << "it is mounted";

    d->mounts.remove(mountPoint);

    QProcess * process = new QProcess(this);
    process->setProcessChannelMode(QProcess::ForwardedChannels);
    process->setProperty("mountPoint", mountPoint);

    process->start(FUSERMOUNT_PATH, QStringList()
            << "-u" // unmount
            << mountPoint
        );

    return process;
}

} // namespace Private
} // namespace Encryption
} // namespace Jobs
