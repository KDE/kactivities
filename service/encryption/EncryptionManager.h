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

#ifndef ENCRYPTIONMANAGER_H_
#define ENCRYPTIONMANAGER_H_

#include <QObject>
#include <QString>
#include <QProcess>

#include "ActivityManager.h"

class EncryptionManager: public QObject {
    Q_OBJECT

public:
    virtual ~EncryptionManager();

    static EncryptionManager * self(const ActivityManager * manager = 0);

    bool isEnabled() const;
    bool isEncryptionInitialized(const QString & activity);

Q_SIGNALS:
    void activityEncryptionChanged(const QString & activity, const bool encrypted);

public Q_SLOTS:
    void setActivityEncrypted(const QString & activity, bool encrypted);
    void mountActivityEncrypted(const QString & activity, bool encrypted);

    void currentActivityChanged(const QString & activity);
    void activityRemoved(const QString & activity);
    void activityChanged(const QString & activity);
    void unmountAll();

private:
    EncryptionManager(const ActivityManager * m);

    static EncryptionManager * s_instance;

    class Private;
    Private * const d;
};

#endif // ENCRYPTIONMANAGER_H_

