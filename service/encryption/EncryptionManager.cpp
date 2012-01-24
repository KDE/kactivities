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

#include "EncryptionManager_p.h"

#include <unistd.h>
#include <config-features.h>

#include <QHash>
#include <QProcess>
#include <QString>

#include <KDebug>
#include <KLocale>
#include <KConfig>
#include <KConfigGroup>

EncryptionManager * EncryptionManager::s_instance = 0;

EncryptionManager * EncryptionManager::self(const ActivityManager * manager)
{
    if (!s_instance) {
        s_instance = new EncryptionManager(manager);
    }

    return s_instance;
}

EncryptionManager::EncryptionManager(const ActivityManager * m)
    : d(new Private(this))
{
    d->manager = m;
    int permissions = ::access(FUSERMOUNT_PATH, X_OK) | ::access(ENCFS_PATH, X_OK);

    d->enabled = (permissions == 0);

    if (!d->enabled) {
        kDebug() << "Encryption is not enabled";
    }

    // Getting the activities folder
    // TODO: This needs to be tested by people actually using i18n :)

    QString activityFolderName = i18nc("Name for the activities folder in user's home", "Activities");

    KConfig config("activitymanagerrc");
    KConfigGroup configGroup(&config, "EncryptionManager");

    QString oldActivityFolderName = configGroup.readEntry("activityFolderName", activityFolderName);

    if (oldActivityFolderName != activityFolderName) {
        if (!QDir::home().rename(oldActivityFolderName, activityFolderName)) {
            activityFolderName = oldActivityFolderName;
        }
    }

    configGroup.writeEntry("activityFolderName", activityFolderName);


    d->activitiesFolder = QDir(QDir::home().filePath(activityFolderName + '/'));
    d->activitiesDataFolder = QDir(QDir::home().filePath(activityFolderName + "/.data/"));
    d->activitiesDataFolder.mkpath(d->activitiesDataFolder.path());

    kDebug() << "Main root folder" << d->activitiesDataFolder;

    // We are going to receive this from ActivityManager directly
    // connect(m, SIGNAL(CurrentActivityChanged(QString)),
    //         this, SLOT(setCurrentActivity(QString)));

    connect(m, SIGNAL(ActivityRemoved(QString)),
            this, SLOT(removeActivity(QString)));

    connect(m, SIGNAL(ActivityChanged(QString)),
            this, SLOT(updateActivity(QString)));

    connect(m, SIGNAL(aboutToQuit()),
            this, SLOT(unmountAll()));

    setCurrentActivity(m->CurrentActivity());
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
    if (!d->enabled) return;

    if (encrypted) {
        d->setupActivityEncryption(activity);

    } else {
        d->terminateActivityEncryption(activity);

    }
}

void EncryptionManager::mountActivityEncrypted(const QString & activity, bool encrypted)
{
    if (!d->enabled) return;

    if (encrypted) {
        d->mountEncryptedFolder(activity);

    } else {
        d->unmountEncryptedFolder(activity);

    }
}

EncryptionManager::Private::Private(EncryptionManager * parent)
    : QObject(parent), q(parent)
{
    connect(&encfs, SIGNAL(mounted(QString)), SLOT(onEncryptedFolderMounted(QString)));
    connect(&encfs, SIGNAL(unmounted(QString)), SLOT(onEncryptedFolderUnmounted(QString)));
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

QString EncryptionManager::Private::activityName(const QString & folderName) const
{
    QStringList sl = QDir(folderName).dirName().split('-');
    if (sl.size() > 1) {
        sl.removeFirst();
        return sl.join("-");
    }
    return QString();
}

void EncryptionManager::Private::setupActivityEncryption(const QString & activity)
{
    // Checking whether the encryption folder is already present

    const QString & encryptedFolderName = folderName(activity, EncryptedFolder);
    const QString & mountFolderName = folderName(activity, MountPointFolder);

    if (encfs.isEncryptionInitialized(activitiesDataFolder.filePath(encryptedFolderName))) {
        return;
    }

    activitiesToSetAsEncrypted.append(activity);

    // Setting the encryption

    encfs.mount(
        activitiesDataFolder.filePath(encryptedFolderName),
        activitiesDataFolder.filePath(mountFolderName)
    );

    // TODO:
    // The above will leave the system mounted. We should probably check
    // whether the activity is current, and if not, unmount it.
    // Probably not, since we are going to mount only current activity
}

void EncryptionManager::Private::terminateActivityEncryption(const QString & activity)
{
    activitiesToTerminateEncryption.append(activity);
    mountEncryptedFolder(activity);
}

void EncryptionManager::Private::mountEncryptedFolder(const QString & activity)
{
    // Mount the file system
    encfs.mount(
            activitiesDataFolder.filePath(folderName(activity, EncryptedFolder)),
            activitiesDataFolder.filePath(folderName(activity, MountPointFolder))
        );
}

void EncryptionManager::Private::setCurrentActivityDone(const QString & activity)
{
    encfs.unmountAllExcept(activitiesDataFolder.filePath(folderName(activity, MountPointFolder)));
    emit q->currentActivityChanged(activity);
}

void EncryptionManager::Private::onEncryptedFolderMounted(const QString & mountPoint)
{
    // Set a symbolic link for current activity

    const QString & activity = activityName(mountPoint);

    if (currentActivity == activity) {
        setCurrentActivityDone(activity);
    }

    if (activitiesToTerminateEncryption.contains(activity)) {

        // move files
        moveFiles(
                folderName(activity, MountPointFolder),
                folderName(activity, NormalFolder)
            );

        // unmount
        unmountEncryptedFolder(activity);

    } else if (activitiesToSetAsEncrypted.contains(activity)) {

        activitiesToSetAsEncrypted.removeAll(activity);
        emit q->activityEncryptionChanged(activity, true);

    }
}

void EncryptionManager::Private::unmountEncryptedFolder(const QString & activity)
{
    encfs.unmount(
            activitiesDataFolder.filePath(folderName(activity, MountPointFolder))
        );
}

void EncryptionManager::Private::onEncryptedFolderUnmounted(const QString & mountPoint)
{
    const QString & activity = activityName(mountPoint);

    if (activitiesToTerminateEncryption.contains(activityName(mountPoint))) {

        // remove dirs
        QDir dir(activitiesDataFolder.path() + '/' + folderName(activity, EncryptedFolder));

        const QStringList & files = dir.entryList(QDir::Hidden | QDir::Files);

        // removing .encfs*.xml set the activity as non-encrypted.
        foreach (const QString & file, files) {
            if (file.startsWith(".encfs")) {
                if (dir.remove(file)) {
                    emit q->activityEncryptionChanged(activity, false);
                } else {
                    kWarning() << "error removing" << activitiesDataFolder.currentPath() + '/' + file;
                }
                // ::unlink(file.toAscii());
            }
        }

        activitiesDataFolder.rmdir(folderName(activity, EncryptedFolder));
        activitiesToTerminateEncryption.removeAll(activity);
    }

    activitiesDataFolder.rmdir(folderName(activity, MountPointFolder));
}

void EncryptionManager::unmountAll()
{
    if (!d->enabled) return;

    d->encfs.unmountAll();
}

void EncryptionManager::Private::moveFiles(const QString & from, const QString & to)
{

}

bool EncryptionManager::isActivityEncrypted(const QString & activity)
{
    if (!d->enabled) return false;

    return d->encfs.isEncryptionInitialized(
            d->activitiesDataFolder.filePath(d->folderName(activity, Private::EncryptedFolder))
        );
}

void EncryptionManager::updateActivity(const QString & activity)
{
}

void EncryptionManager::removeActivity(const QString & activity)
{
}

void EncryptionManager::setCurrentActivity(const QString & activity)
{
    const QString & currentFolderName = i18nc("Directory name for the current activity", "Current");
    kDebug() << "This is now the current activity" << activity;

    d->currentActivity = activity;

    if (isActivityEncrypted(activity)) {
        kDebug() << "Mounting" << activity << d->manager->ActivityName(activity);

        kDebug() << "It is initialized, we are creating the link" <<
                d->activitiesDataFolder.filePath(d->folderName(activity, Private::MountPointFolder)) <<
                d->activitiesFolder.filePath(currentFolderName);

        QFile::link(
                d->activitiesDataFolder.filePath(d->folderName(activity, Private::MountPointFolder)),
                d->activitiesFolder.filePath(currentFolderName)
            );

        d->mountEncryptedFolder(activity);

    } else {
        kDebug() << "It is not initialized, removing the link" <<
                d->activitiesFolder.filePath(currentFolderName);

        QFile::remove(d->activitiesFolder.filePath(currentFolderName));

        d->setCurrentActivityDone(activity);
    }
}

