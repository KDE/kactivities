/*
 *   Copyright (C) 2015 Ivan Cukic <ivan.cukic(at)kde.org>
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

#ifndef EXTRA_ACTIVITES_INTERFACE_H
#define EXTRA_ACTIVITES_INTERFACE_H

#include <QAbstractListModel>

#include <utils/d_ptr.h>

#include <QJSValue>
#include <QKeySequence>

class ExtraActivitiesInterface : public QObject {
    Q_OBJECT

public:
    ExtraActivitiesInterface(QObject *parent = Q_NULLPTR);
    ~ExtraActivitiesInterface();

public Q_SLOTS:
    void setIsPrivate(const QString &activity, bool isPrivate,
                      QJSValue callback);
    void getIsPrivate(const QString &activity, QJSValue callback);

    void setShortcut(const QString &activity, const QKeySequence &keySequence);
    QKeySequence shortcut(const QString &activity);

private:
    D_PTR;
};

#endif // EXTRA_ACTIVITES_INTERFACE_H
