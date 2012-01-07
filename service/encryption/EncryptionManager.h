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

class EncryptionManager: public QObject {
public:
    virtual ~EncryptionManager();

    static EncryptionManager * self();

    bool isEnabled() const;

    void setActivityEncrypted(const QString & activity, bool encrypted);

private:
    EncryptionManager();

    static EncryptionManager * s_instance;

    class Private;
    Private * const d;
};


#endif // ENCRYPTIONMANAGER_H_

