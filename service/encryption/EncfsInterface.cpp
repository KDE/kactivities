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

#include "EncfsInterface.h"
#include <config-features.h>

#include <QHash>
#include <QProcess>
#include <QDir>
#include <QStringList>

#include <KLocale>
#include <KDebug>

class EncfsInterface::Private {
public:
    Private(EncfsInterface * parent)
        : q(parent)
    {
    }

    QString askForPassword() const;
    QString askForPassword(bool twice) const;

    QHash < QString, QProcess* > mounts;

    QProcess * startEncfs(const QString & what, const QString & mountPoint, const QString & password, bool init = false);

    EncfsInterface * const q;
};

QString EncfsInterface::Private::askForPassword() const
{
    QProcess kdialog;

    kdialog.start("kdialog",
            QStringList()
            << "--title"
            << i18n("Activity password")
            << "--password"
            << i18n("Enter password to use for encryption")
        );

    if (kdialog.waitForFinished(-1)) {
        return kdialog.readAllStandardOutput().trimmed();
    }

    return QString();
}

QString EncfsInterface::Private::askForPassword(bool twice) const
{
    QString result = askForPassword();

    if (twice && !result.isEmpty()) {
        if (result != askForPassword()) {
            return QString();
        }
    }

    return result;
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
    if (d->mounts.contains(mountPoint)) return;

    bool shouldInitialize = !isEncryptionInitialized(what);

    // Getting the password

    QString password = d->askForPassword(/* twice = */ shouldInitialize);

    if (password.isEmpty()) {
        // TODO: Report error - empty password
        return;
    }

    // This should be necessary only for init mount, but
    // we are doing it always just in case

    QDir dir;
    dir.mkpath(what);
    dir.mkpath(mountPoint);

    // Setting the encryption
    d->startEncfs(what, mountPoint, password, /* initialize = */ shouldInitialize);
}

void EncfsInterface::umount(const QString & mountPoint)
{
    if (!d->mounts.contains(mountPoint)) return;

    QProcess * encfs = new QProcess(this);
    encfs->setProcessChannelMode(QProcess::ForwardedChannels);

    connect(encfs, SIGNAL(finished(int, QProcess::ExitStatus)),
            encfs, SLOT(deleteLater()));

    encfs->start(FUSERMOUNT_PATH, QStringList()
            << "-u" // unmount
            << mountPoint
        );
}

QProcess * EncfsInterface::Private::startEncfs(const QString & what,
        const QString & mountPoint, const QString & password, bool init)
{
    kDebug() << "Executing" << ENCFS_PATH << "-f -S"
            << what
            << mountPoint;

    QProcess * encfs = new QProcess(q);
    encfs->setProcessChannelMode(QProcess::ForwardedChannels);

    connect(encfs, SIGNAL(finished(int, QProcess::ExitStatus)),
            q, SLOT(processFinished(int, QProcess::ExitStatus)));

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

void EncfsInterface::processFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    QProcess * process = static_cast < QProcess * > (sender());

    QString mountPoint = d->mounts.key(process);

    if (process->exitCode() != 0) {
        // There is an error calling encfs
        kDebug() << "ERROR: Mounting failed! Probably a wrong password";
    }

    d->mounts.remove(mountPoint);
    QDir().rmpath(mountPoint);
}

