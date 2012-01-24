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
    umountAll();

    delete d;
}

bool EncfsInterface::isEncryptionInitialized(const QString & path)
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

void EncfsInterface::umountAll()
{
    kDebug() << "Unmounting everything";

    foreach (const QString & mount, d->mounts.keys()) {
        umount(mount);
    }

    d->mounts.clear();
}

void EncfsInterface::mount(const QString & what, const QString & mountPoint)
{
    // warning: KMountPoint depends on /etc/mtab according to the documentation.
    KMountPoint::Ptr ptr = KMountPoint::currentMountPoints().findByPath(mountPoint);
    if (ptr && ptr.data()->mountPoint() == mountPoint) {
        kDebug() << mountPoint << "already mounted";
        d->mounts.insert(mountPoint, 0);
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

void EncfsInterface::umount(const QString & mountPoint)
{
    if (!d->mounts.contains(mountPoint)) return;

    QProcess * encfs = new QProcess(this);
    encfs->setProcessChannelMode(QProcess::ForwardedChannels);
    encfs->setProperty("mountPoint", mountPoint);

    connect(encfs, SIGNAL(finished(int, QProcess::ExitStatus)),
                   SLOT(umountProcessFinished(int,QProcess::ExitStatus)));

    encfs->start(FUSERMOUNT_PATH, QStringList()
            << "-u" // unmount
            << mountPoint
        );
}

void EncfsInterface::umountProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitCode)
    Q_UNUSED(exitStatus)

    QProcess * process = qobject_cast < QProcess * > (sender());
    emit unmounted(process->property("mountPoint").toString());
    process->deleteLater();
}

QProcess * EncfsInterface::Private::startEncfs(const QString & what,
        const QString & mountPoint, const QString & password, bool init)
{
    // TODO: sometimes this command stalls QProcess and the signal finished is never emitted.
    kDebug() << "Executing" << ENCFS_PATH << "-f -S"
            << what
            << mountPoint;

    QProcess * encfs = new QProcess(q);
    encfs->setProcessChannelMode(QProcess::ForwardedChannels);
    encfs->setProperty("mountPoint", mountPoint);

    connect(encfs, SIGNAL(finished(int,QProcess::ExitStatus)),
            q, SLOT(mountProcessFinished(int,QProcess::ExitStatus)));

    encfs->start(ENCFS_PATH, QStringList()
            << "-f" // foreground mode
            << "-S" // password read from stdin
            << what
            << mountPoint
        );

    if (init) {
        encfs->write("p\n"); // paranoia mode
    }

    encfs->write(password.toAscii()); // writing the password
    encfs->write("\n");

    mounts[mountPoint] = encfs;

    return encfs;
}

void EncfsInterface::mountProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitCode)
    Q_UNUSED(exitStatus)

    QProcess * process = qobject_cast < QProcess * > (sender());
    QString mountPoint = process->property("mountPoint").toString();

    if (process->exitCode() == 0 && process->exitStatus() == QProcess::NormalExit) {
        emit mounted(mountPoint);
    } else {
        KMountPoint::Ptr ptr = KMountPoint::currentMountPoints().findByPath(mountPoint);
        // probably we tried to mount an already mounted encfs.
        // This should happen, anyway we do not need to bother the user with a popup.
        if (ptr && ptr.data()->mountPoint() == mountPoint) {
            kDebug() << mountPoint << "already mounted";
            d->mounts.insert(mountPoint, 0);
            emit mounted(mountPoint);
            goto out;
        }

        // There is an error calling encfs
        kDebug() << "ERROR: Mounting failed! Probably a wrong password";
        QDir().rmpath(mountPoint);
        d->mounts.remove(mountPoint);

        Ui::message(
                i18n("Error"),
                i18n("Error setting up the encrypted folder for the activity.")
            );
    }

out:
    process->deleteLater();
}
