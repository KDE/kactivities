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

#ifndef JOBS_ENCRYPTION_ENCFS_P_H
#define JOBS_ENCRYPTION_ENCFS_P_H

#include "Encfs.h"

#include <QSet>
#include <QProcess>

namespace Jobs {
namespace Encryption {
namespace Private {

class Encfs::Private: public QObject
{
Q_OBJECT
public:
    Private(Encfs * parent);

    QSet < QString > mounts;

    QProcess * startEncfs(const QString & what, const QString & mountPoint, const QString & password);

    Encfs * const q;
};

} // namespace Private
} // namespace Encryption
} // namespace Jobs

#endif // JOBS_ENCRYPTION_ENCFS_P_H
