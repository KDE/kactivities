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

#include "EncryptionManager.h"
#include "EncfsInterface.h"

#include <unistd.h>
#include <config-features.h>

#include <QHash>
#include <QProcess>
#include <QDir>
#include <QString>

#include <KDebug>
#include <KLocale>

class EncryptionManager::Private {
public:
    Private(EncryptionManager * parent)
        : q(parent) {};

    enum Folder {
        NormalFolder = 0,
        EncryptedFolder = 1,
        MountPointFolder = 2
    };

    QString folderName(const QString & activity, Folder folder) const;
    void moveFiles(const QString & from, const QString & to);

    void setupActivityEncryption(const QString & activity);
    void terminateActivityEncryption(const QString & activity);
    void mountEncryptedFolder(const QString & activity);
    void umountEncryptedFolder(const QString & activity);

    EncryptionManager * const q;
    QDir activitiesFolder;
    bool enabled: 1;

    EncfsInterface encfs;
};

EncryptionManager * EncryptionManager::s_instance = 0;

EncryptionManager * EncryptionManager::self()
{
    if (!s_instance) {
        s_instance = new EncryptionManager();
    }

    return s_instance;
}

EncryptionManager::EncryptionManager()
    : d(new Private(this))
{
    int permissions = ::access(FUSERMOUNT_PATH, X_OK);

    d->enabled = (permissions == 0);

    if (!d->enabled) {
        kDebug() << "Encryption is not enabled";
    }

    // TODO: localize
    d->activitiesFolder = QDir(QDir::home().filePath("Activities/.data/"));
    d->activitiesFolder.mkpath(d->activitiesFolder.path());
    kDebug() << "Main root folder" << d->activitiesFolder;
}

EncryptionManager::~EncryptionManager()
{
    delete d;
}

bool EncryptionManager::isEnabled() const
{
    return d->enabled;
}

// Folder setup

void EncryptionManager::setActivityEncrypted(const QString & activity, bool encrypted)
{
    if (encrypted) {
        d->setupActivityEncryption(activity);

    } else {
        d->terminateActivityEncryption(activity);

    }
}

void EncryptionManager::mountActivityEncrypted(const QString & activity, bool encrypted)
{
    if (encrypted) {
        d->mountEncryptedFolder(activity);

    } else {
        d->umountEncryptedFolder(activity);

    }
}

QString EncryptionManager::Private::folderName(const QString & activity, Folder folder) const
{
    switch (folder) {
        case NormalFolder:
            return activity;

        case EncryptedFolder:
            return ".crypt-" + activity;

        case MountPointFolder:
            return "crypt-" + activity;

    }

    return QString();
}

void EncryptionManager::Private::setupActivityEncryption(const QString & activity)
{
    // Checking whether the encryption folder is already present

    const QString & encryptedFolderName = folderName(activity, EncryptedFolder);
    const QString & mountFolderName = folderName(activity, MountPointFolder);

    if (encfs.isEncryptionInitialized(activitiesFolder.filePath(encryptedFolderName))) {
        return;
    }

    // Setting the encryption

    encfs.mount(
        activitiesFolder.filePath(encryptedFolderName),
        activitiesFolder.filePath(mountFolderName)
    );

    // TODO:
    // The above will leave the system mounted. We should probably check
    // whether the activity is current, and if not, unmount it.
    // Probably not, since we are going to mount only current activity
}

void EncryptionManager::Private::terminateActivityEncryption(const QString & activity)
{
    // mount
    mountEncryptedFolder(activity);

    // move files
    moveFiles(
            folderName(activity, MountPointFolder),
            folderName(activity, NormalFolder)
        );

    // unmount
    umountEncryptedFolder(activity);

    // remove dirs
    QDir dir = activitiesFolder;
    dir.cd(folderName(activity, EncryptedFolder));

    const QStringList & files = dir.entryList(QDir::Hidden);

    foreach (const QString & file, files) {
        if (file.startsWith(".encfs")) {
            dir.remove(file);
            // ::unlink(file.toAscii());
        }
    }

    activitiesFolder.rmdir(folderName(activity, EncryptedFolder));
    activitiesFolder.rmdir(folderName(activity, MountPointFolder));
}

void EncryptionManager::Private::mountEncryptedFolder(const QString & activity)
{
    // Mount the file system
    encfs.mount(
            activitiesFolder.filePath(folderName(activity, EncryptedFolder)),
            activitiesFolder.filePath(folderName(activity, MountPointFolder))
        );

    // Set a symbolic link for current activity
}

void EncryptionManager::Private::umountEncryptedFolder(const QString & activity)
{
    encfs.umount(
            activitiesFolder.filePath(folderName(activity, MountPointFolder))
        );
}

void EncryptionManager::Private::moveFiles(const QString & from, const QString & to)
{

}

bool EncryptionManager::isEncryptionInitialized(const QString & activity)
{
    return d->encfs.isEncryptionInitialized(
            d->activitiesFolder.filePath(d->folderName(activity, Private::EncryptedFolder))
        );
}

