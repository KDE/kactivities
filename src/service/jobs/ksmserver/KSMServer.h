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

#ifndef KSMSERVER_H
#define KSMSERVER_H

#include <QObject>

#include <utils/d_ptr.h>
#include <utils/nullptr.h>

/**
 * KSMServer
 */
class KSMServer: public QObject {
    Q_OBJECT
public:

    enum ReturnStatus {
        Started      = 0,
        Stopped      = 1,
        FailedToStop = 2
    };

    KSMServer(QObject * parent = nullptr);
    virtual ~KSMServer();

public Q_SLOTS:
    void startActivitySession(const QString & activity);
    void stopActivitySession(const QString & activity);

Q_SIGNALS:
    void activitySessionStateChanged(const QString & activity, int status);

private:
    D_PTR;
};

#endif // KSMSERVER_H

