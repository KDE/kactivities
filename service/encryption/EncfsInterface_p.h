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

#ifndef ENCFSINTERFACE_P_H
#define ENCFSINTERFACE_P_H

#include "EncfsInterface.h"

#include <QSet>
#include <QProcess>

class EncfsInterface::Private: public QObject
{
Q_OBJECT
public:
    Private(EncfsInterface * parent);

    QSet < QString > mounts;

    QProcess * startEncfs(const QString & what, const QString & mountPoint, const QString & password, bool init = false);

    EncfsInterface * const q;
    bool shouldInitialize;
    QString what;
    QString mountPoint;

Q_SIGNALS:
    void gotPassword(const QString & password);

public Q_SLOTS:
    void onGotPassword(const QString & password);
    void askForPassword(bool twice);
};

#endif // ENCFSINTERFACE_P_H
