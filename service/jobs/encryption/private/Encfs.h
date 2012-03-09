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

#ifndef ENCRYPTION_ENCFS_H_
#define ENCRYPTION_ENCFS_H_

#include <QObject>
#include <QProcess>

namespace Jobs {
namespace Encryption {
namespace Private {

/**
 * Encfs
 */
class Encfs: public QObject {
    Q_OBJECT

public:
    Encfs(QObject * parent = 0);
    virtual ~Encfs();

    QProcess * mount(const QString & what, const QString & mountPoint, bool initialize, const QString & password);
    QProcess * unmount(const QString & mountPoint, bool deinitialize);

    void unmountAll();
    void unmountAllExcept(const QString & path = QString());

    bool isEncryptionInitialized(const QString & path) const;
    bool isMounted(const QString & path) const;

private:
    class Private;
    Private * const d;
};

} // namespace Private
} // namespace Encryption
} // namespace Jobs

#endif // ENCRYPTION_ENCFS_H_

