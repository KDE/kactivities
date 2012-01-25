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

#include "EncfsInterface_p.h"
#include <config-features.h>

#include <QDir>
#include <QStringList>

#include <KLocale>
#include <KDebug>
#include <kmountpoint.h>

#include "ui/Ui.h"

EncfsInterface::Private::Private(EncfsInterface * parent)
    : QObject(parent), q(parent), shouldInitialize(false)
{
}

void EncfsInterface::Private::askForPassword(bool newPassword)
{
    Ui::askPassword(
            i18n("Activity password"),
            i18n("Enter password to use for encryption"),
            newPassword,
            this, "onGotPassword"
       );
}

void EncfsInterface::Private::onGotPassword(const QString & password)
{
    if (password.isEmpty()) {
        // TODO: Report error - empty password
        return;
    }

    kDebug() << "Got the password ... mounting the encfs"
        << what << mountPoint << shouldInitialize;

    // This should be necessary only for init mount, but
    // we are doing it always just in case

    QDir dir;
    dir.mkpath(what);
    dir.mkpath(mountPoint);

    // Setting the encryption
    startEncfs(what, mountPoint, password, /* initialize = */ shouldInitialize);
}

EncfsInterface::EncfsInterface(QObject * parent)
    : QObject(parent), d(new Private(this))
{
}

EncfsInterface::~EncfsInterface()
{
    unmountAll();

    delete d;
}

bool EncfsInterface::isEncryptionInitialized(const QString & path) const
{
    QDir dir(path);

    const QStringList & files = dir.entryList(QDir::Hidden | QDir::Files);

    foreach (const QString & file, files) {
        if (file.startsWith(".encfs")) {
            return true;
        }
    }

    return false;
}

bool EncfsInterface::isMounted(const QString & path) const
{
    // warning: KMountPoint depends on /etc/mtab according to the documentation.
    KMountPoint::Ptr ptr = KMountPoint::currentMountPoints().findByPath(path);

    return (ptr && ptr.data()->mountPoint() == path);
}

void EncfsInterface::unmountAllExcept(const QString & path)
{
    kDebug() << "Unmounting everything except " << path;

    foreach (const QString & mount, d->mounts) {
        if (path != mount) {
            unmount(mount);
        }
    }
}

void EncfsInterface::unmountAll()
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

void EncfsInterface::mount(const QString & what, const QString & mountPoint)
{
    if (isMounted(mountPoint)) {
        kDebug() << mountPoint << "already mounted";
        d->mounts << mountPoint;
        emit mounted(mountPoint);
        return;
    }

    d->shouldInitialize = !isEncryptionInitialized(what);
    d->what = what;
    d->mountPoint = mountPoint;

    // Asynchronously getting the password
    QMetaObject::invokeMethod(d, "askForPassword", Qt::QueuedConnection,
                              Q_ARG(bool, /* twice = */ d->shouldInitialize));
}

void EncfsInterface::unmount(const QString & mountPoint)
{
    if (!isMounted(mountPoint)) return;

    d->mounts.remove(mountPoint);

    QProcess * fusermount = new QProcess(this);
    fusermount->setProcessChannelMode(QProcess::ForwardedChannels);
    fusermount->setProperty("mountPoint", mountPoint);

    connect(fusermount, SIGNAL(finished(int, QProcess::ExitStatus)),
            SLOT(unmountProcessFinished(int,QProcess::ExitStatus)));

    fusermount->start(FUSERMOUNT_PATH, QStringList()
            << "-u" // unmount
            << mountPoint
        );
}

void EncfsInterface::unmountProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitCode)
    Q_UNUSED(exitStatus)

    QProcess * process = qobject_cast < QProcess * > (sender());

    if (!process) return;

    const QString & mountPoint = process->property("mountPoint").toString();

    kDebug() << "Unmounted" << mountPoint;
    emit unmounted(mountPoint);
    process->deleteLater();
}

void EncfsInterface::mountProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitCode)
    Q_UNUSED(exitStatus)

    QProcess * process = qobject_cast < QProcess * > (sender());
    if (!process) return;

    const QString & mountPoint = process->property("mountPoint").toString();

    if (process->exitCode() == 0 && process->exitStatus() == QProcess::NormalExit) {
        kDebug() << "Mounted" << mountPoint;
        emit mounted(mountPoint);

    } else {
        // probably we tried to mount an already mounted encfs.
        // This should happen, anyway we do not need to bother the user with a popup.
        if (isMounted(mountPoint)) {
            kDebug() << mountPoint << "already mounted";
            d->mounts << mountPoint;
            emit mounted(mountPoint);

        } else {
            // There is an error calling encfs
            kDebug() << "ERROR: Mounting failed! Probably a wrong password";
            QDir().rmpath(mountPoint);
            d->mounts.remove(mountPoint);

            Ui::message(
                    i18n("Error"),
                    i18n("Error unlocking the activity.\nYou've probably entered a wrong password.")
                );

        }
    }

    process->deleteLater();
}

QProcess * EncfsInterface::Private::startEncfs(const QString & what,
        const QString & mountPoint, const QString & password, bool init)
{
    // TODO: sometimes this command stalls QProcess and the signal finished is never emitted.
    kDebug() << "Executing" << ENCFS_PATH << " -S"
            << what
            << mountPoint;

    QProcess * encfs = new QProcess(q);
    encfs->setProcessChannelMode(QProcess::ForwardedChannels);
    encfs->setProperty("mountPoint", mountPoint);

    connect(encfs, SIGNAL(finished(int,QProcess::ExitStatus)),
            q, SLOT(mountProcessFinished(int,QProcess::ExitStatus)));

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

    mounts << mountPoint;

    return encfs;
}
