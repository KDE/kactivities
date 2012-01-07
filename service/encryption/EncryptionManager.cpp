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

#include <unistd.h>
#include <config-features.h>

#include <QHash>
#include <QProcess>
#include <QDir>
#include <QString>

#include <KDebug>

class EncryptionManager::Private {
public:
    Private(EncryptionManager * parent)
        : q(parent) {};

    enum Folder {
        NormalFolder = 0,
        EncryptedFolder = 1,
        MountPointFolder = 2
    };

    QString askForPassword() const;
    QString folderName(const QString & activity, Folder folder) const;
    void moveFiles(const QString & from, const QString & to);

    void setupActivityEncryption(const QString & activity);
    void terminateActivityEncryption(const QString & activity);
    void mountEncryptedFolder(const QString & activity);
    void umountEncryptedFolder(const QString & activity);

    EncryptionManager * const q;
    QHash < QString, QProcess * > encfsProcesses;
    QString folderPrefix;
    bool enabled: 1;
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

    d->folderPrefix = QDir::homePath();

    if (!d->folderPrefix.endsWith("/")) {
        d->folderPrefix += "/";
    }

    // TODO: localize
    d->folderPrefix += "Activities/";
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

QString EncryptionManager::Private::folderName(const QString & activity, Folder folder) const
{
    switch (folder) {
        case NormalFolder:
            return folderPrefix + activity;

        case EncryptedFolder:
            return folderPrefix + ".crypt-" + activity;

        case MountPointFolder:
            return folderPrefix + "crypt-" + activity;

    }

    return QString();
}

QString EncryptionManager::Private::askForPassword() const
{
    // TODO: Ask kdialog for the password
    return "somepassword";
}

void EncryptionManager::Private::setupActivityEncryption(const QString & activity)
{
    // Checking whether the encryption folder is already present
    const QString & encryptedFolderPath = folderName(activity, EncryptedFolder);
    const QString & mountFolderPath = folderName(activity, MountPointFolder);

    QDir encryptedFolder(encryptedFolderPath);

    if (encryptedFolder.exists()) {
        const QStringList & files = encryptedFolder.entryList(QDir::Hidden);

        foreach (const QString & file, files) {
            if (file.startsWith(".encfs")) {
                // We already have the encfs directory set up, exiting
                return;
            }
        }

    } else {
        encryptedFolder.mkpath(encryptedFolderPath);

    }

    encryptedFolder.mkpath(mountFolderPath);

    // Setting the encryption

    QProcess * encfs = new QProcess(q);
    encfs->setProcessChannelMode(QProcess::ForwardedChannels);

    encfs->start(ENCFS_PATH, QStringList()
            << "-f" // foreground mode
            << "-S" // password read from stdin
            << encryptedFolderPath
            << mountFolderPath
        );

    encfs->write("p\n"); // paranoia mode
    encfs->write(askForPassword().toAscii()); // writing the password
    encfs->write("\n");

    encfsProcesses[activity] = encfs;

    // TODO:
    // The above will leave the system mounted. Weshould probably check
    // whether the activity is current, and if not, unmount it.
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
    QDir dir(folderName(activity, EncryptedFolder));

    const QStringList & files = dir.entryList(QDir::Hidden);

    foreach (const QString & file, files) {
        if (file.startsWith(".encfs")) {
            dir.remove(file);
            // ::unlink(file.toAscii());
        }
    }

    dir.rmpath(folderName(activity, NormalFolder));
    dir.rmpath(folderName(activity, EncryptedFolder));
}

void EncryptionManager::Private::mountEncryptedFolder(const QString & activity)
{
    // Create mount point
    // Mount the file system
    // Set a symbolic link for current activity
}

void EncryptionManager::Private::umountEncryptedFolder(const QString & activity)
{
    if (!encfsProcesses.contains(activity)) return;

    // terminate encfs process
    delete encfsProcesses[activity];
    encfsProcesses.remove(activity);

    QDir().rmpath(folderName(activity, MountPointFolder));

    // remove the symbolic link if this is the current activity
}

void EncryptionManager::Private::moveFiles(const QString & from, const QString & to)
{

}


