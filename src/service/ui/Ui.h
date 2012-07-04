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

#ifndef UI_UI_H
#define UI_UI_H

#include <QObject>
#include <utils/d_ptr.h>

/**
 * Ui
 */
class Ui: public QObject {
    Q_OBJECT

public:
    virtual ~Ui();

    /**
     * Asynchronous way to ask for a password
     * @arg title title for the window
     * @arg message password prompt message
     * @arg newPassword should the password be asked twice
     * @arg receiver object to receive the signal when the password is returned
     * @arg slot method to call
     */
    static void askPassword(const QString & title, const QString & message,
            bool newPassword, bool unlockMode,
            QObject * receiver, const char * slot);

    static void ask(const QString & title, const QString & message,
            const QStringList & choices,
            QObject * receiver, const char * slot);

    static void message(const QString & title, const QString & message);

    static void setBusy(bool value = true);

    static Ui * self();

public Q_SLOTS:
    void unsetBusy();


private Q_SLOTS:

    void _askPassword(const QString & title, const QString & message,
            bool newPassword, bool unlockMode,
            QObject * receiver, const char * slot);

    void _ask(const QString & title, const QString & message,
            const QStringList & choices,
            QObject * receiver, const char * slot);

    void _message(const QString & title, const QString & message);

    void _setBusy(bool value);

private:
    Ui(QObject * parent);

    D_PTR;
};

#endif // UI_UI_H

