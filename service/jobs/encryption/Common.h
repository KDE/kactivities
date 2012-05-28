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

#ifndef JOBS_ENCRYPTION_COMMON_H
#define JOBS_ENCRYPTION_COMMON_H

#include <QString>

class QProcess;

namespace Jobs {
namespace Encryption {

namespace Private {
    class Encfs;
} // namespace Private

namespace Common {

    bool isEnabled();
    bool isActivityEncrypted(const QString & activity);

    QProcess * execMount(const QString & activity, const QString & password);
    QProcess * execUnmount(const QString & activity);

    void unmountAllExcept(const QString & activity);
    void unmountAll();

    enum FolderType {
        NormalFolder = 0,
        EncryptedFolder = 1,
        MountPointFolder = 2,

        FirstFolderType = 8,
        ActivityFolder = 8,
        UserDataFolder = 9,
        ConfigFolder = 10,
        DataFolder = 11,
        AutostartFolder = 12,
        AutostopFolder = 13,
        LastFolderType = 13
    };

    QString folderPath(const QString & activity, FolderType type);
    QString folderName(FolderType type);

} // namespace Common
} // namespace Encryption
} // namespace Jobs

#endif // JOBS_ENCRYPTION_COMMON_H

