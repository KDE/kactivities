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

#ifndef ENCRYPTIONMANAGER_P_H
#define ENCRYPTIONMANAGER_P_H

#include "EncryptionManager.h"
#include "EncfsInterface.h"

#include <QDir>

class EncryptionManager::Private: public QObject
{
Q_OBJECT
public:
    Private(EncryptionManager * parent);

    enum Folder {
        NormalFolder = 0,
        EncryptedFolder = 1,
        MountPointFolder = 2
    };

    QString folderName(const QString & activity, Folder folder) const;
    QString activityName(const QString & folderName) const;
    void moveFiles(const QString & from, const QString & to);

    void setupActivityEncryption(const QString & activity);
    void terminateActivityEncryption(const QString & activity);
    void mountEncryptedFolder(const QString & activity);
    void unmountEncryptedFolder(const QString & activity);
    void setCurrentActivityDone(const QString & activity);

    EncryptionManager * const q;
    const ActivityManager * manager;

    QStringList activitiesToTerminateEncryption;
    QStringList activitiesToSetAsEncrypted;

    QDir activitiesFolder;
    QDir activitiesDataFolder;

    QString currentActivity;
    bool enabled: true;

    EncfsInterface encfs;

public Q_SLOTS:
    void onEncryptedFolderMounted(const QString & mountPoint);
    void onEncryptedFolderUnmounted(const QString & mountPoint);
};

#endif // ENCRYPTIONMANAGER_P_H
