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

void EncfsInterface::Private::askForPassword()
{
    if (kdialog) {
        kWarning() << "kdialog already running";
        return;
    }

    kdialog = new QProcess(this);
    connect(kdialog, SIGNAL(finished(int,QProcess::ExitStatus)), SLOT(onKdialogFinished(int,QProcess::ExitStatus)));

    kdialog->start("kdialog",
               QStringList()
               << "--title"
               << i18n("Activity password")
               << "--password"
               << i18n("Enter password to use for encryption")
           );
}

void EncfsInterface::Private::onKdialogFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitCode || exitStatus != QProcess::NormalExit) {
        kDebug() << "kdialog returned error";
        emit gotPassword(QString());
        kdialog->deleteLater();
        kdialog = 0;
        return;
    }

    QString result = kdialog->readAllStandardOutput().trimmed();
    kdialog->deleteLater();
    kdialog = 0;

    if (firstPasswordCandidate.isEmpty()) {
        if (twice && !result.isEmpty()) {
            firstPasswordCandidate = result;
            askForPassword();
        } else {
            emit gotPassword(result);
        }
    } else {
        if (firstPasswordCandidate == result) {
            emit gotPassword(result);
        } else {
            emit gotPassword(QString());
        }
    }
}

void EncfsInterface::Private::askForPassword(bool twice)
{
    twice = twice;
    firstPasswordCandidate.clear();
    askForPassword();
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

    d->shouldInitialize = !isEncryptionInitialized(what);
    d->what = what;
    d->mountPoint = mountPoint;

    // Asynchronously getting the password

    connect(d, SIGNAL(gotPassword(QString)), SLOT(gotPassword(QString)));
    QMetaObject::invokeMethod(d, "askForPassword", Qt::QueuedConnection,
                              Q_ARG(bool, /* twice = */ d->shouldInitialize));
}

void EncfsInterface::gotPassword(const QString & password)
{
    if (password.isEmpty()) {
        // TODO: Report error - empty password
        return;
    }

    // This should be necessary only for init mount, but
    // we are doing it always just in case

    QDir dir;
    dir.mkpath(d->what);
    dir.mkpath(d->mountPoint);

    // Setting the encryption
    d->startEncfs(d->what, d->mountPoint, password, /* initialize = */ d->shouldInitialize);
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
    QProcess * process = qobject_cast < QProcess * > (sender());

    QString mountPoint = d->mounts.key(process);

    if (process->exitCode() == 0 && process->exitStatus() == QProcess::NormalExit) {
        emit mounted(mountPoint);
    } else {
        // There is an error calling encfs
        kDebug() << "ERROR: Mounting failed! Probably a wrong password";
        QDir().rmpath(mountPoint);
        d->mounts.remove(mountPoint);

        QProcess::execute("kdialog", QStringList()
                << "--title"
                << i18n("Error")
                << "--msgbox"
                << i18n("Error setting up the encrypted folder for the activity.")
            );
    }
    process->deleteLater();
}
