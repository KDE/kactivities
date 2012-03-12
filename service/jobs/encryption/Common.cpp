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

#include "Common.h"

#include <unistd.h>
#include <config-features.h>

#include <QDir>

#include <KLocale>
#include <KConfig>
#include <KConfigGroup>
#include <KDebug>

#include "private/Encfs.h"

namespace Jobs {
namespace Encryption {
namespace Common {

class Private {
public:
    Private();
    Private * operator -> () { return this; }
    QString folderName(const QString & activity, FolderType type);

    bool enabled;
    QDir activitiesFolder;
    QDir activitiesDataFolder;
    Jobs::Encryption::Private::Encfs encfs;

    static Private * s_instance;
} d;

Private * Private::s_instance = 0;

Private::Private()
{
    int permissions = ::access(FUSERMOUNT_PATH, X_OK) | ::access(ENCFS_PATH, X_OK);

    d->enabled = (permissions == 0);

    if (!d->enabled) {
        kDebug() << "Encryption is not enabled";
        return;
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
}


bool isEnabled()
{
    return d->enabled;
}

bool isActivityEncrypted(const QString & activity)
{
    if (!isEnabled()) return false;

    return d->encfs.isEncryptionInitialized(
            d->activitiesDataFolder.filePath(d->folderName(activity, EncryptedFolder))
        );

}

QString Private::folderName(const QString & activity, FolderType type)
{
    switch (type) {
        case NormalFolder:
            return activity;

        case EncryptedFolder:
            return ".crypt-" + activity;

        case MountPointFolder:
            return "crypt-" + activity;

        default:
            return QString();
    }

    // return QString();
}

QString path(const QString & activity, FolderType type)
{
    if (type < ActivityFolder) {
        return d->activitiesDataFolder.filePath(
                d->folderName(activity, type)
            );
    }

    bool activityEncrypted = isActivityEncrypted(activity);
    QString activityPath = d->activitiesDataFolder.filePath(
            d->folderName(activity,
                activityEncrypted ? MountPointFolder : NormalFolder));
    if (!activityPath.endsWith('/'))
        activityPath.append('/');


    switch (type) {
        case UserDataFolder:
            return activityPath.append("user/");

        case ConfigFolder:
            return activityPath.append("config/");

        case DataFolder:
            return activityPath.append("apps/");

        default:
            return activityPath;

    }
}

QProcess * execMount(const QString & activity, bool initialize, const QString & password)
{
    if (password.isEmpty()) {
        return NULL;
    }

    const QString & encryptedFolderName = d->folderName(activity, EncryptedFolder);
    const QString & mountFolderName = d->folderName(activity, MountPointFolder);

    return d->encfs.mount(
        d->activitiesDataFolder.filePath(encryptedFolderName),
        d->activitiesDataFolder.filePath(mountFolderName),
        initialize,
        password
    );
}

QProcess * execUnmount(const QString & activity, bool deinitialize)
{
    return d->encfs.unmount(
        d->activitiesDataFolder.filePath(d->folderName(activity, MountPointFolder)),
        deinitialize
    );
}

void unmountExcept(const QString & activity)
{
    d->encfs.unmountAllExcept(
        d->activitiesDataFolder.filePath(d->folderName(activity, MountPointFolder))
    );
}

void unmountAll()
{
    d->encfs.unmountAllExcept();
}

void initializeStructure(const QString & activity)
{
    const QString & mountFolderName = d->folderName(activity, MountPointFolder);
    QDir dir(d->activitiesDataFolder.filePath(mountFolderName));

    dir.mkdir("user");
    dir.mkdir("config");
    dir.mkdir("apps");
}

} // namespace Common
} // namespace Encryption
} // namespace Jobs
