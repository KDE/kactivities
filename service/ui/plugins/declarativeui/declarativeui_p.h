/*
 *   Copyright (C) 2012 Ivan Cukic ivan.cukic(at)kde.org
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2 of
 *   the License, or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef UI_HANDLER_DECLARATIVE_P_H_
#define UI_HANDLER_DECLARATIVE_P_H_

#include "declarativeui.h"
#include "kdeclarativemainwindow.h"

#include <QObject>

class DeclarativeUiHandler::Private: public QObject {
    Q_OBJECT

public:
    Private();
    KDeclarativeMainWindow * window;
    QObject * receiver;
    const char * slot;

public Q_SLOTS:
    void onCurrentActivityChanged(const QString & activity);
    void returnPassword(const QString & password);
    void cancel();

Q_SIGNALS:
    void message(const QString & message);
    void askPassword(const QString & title, const QString & message, bool newPassword);
    void hideAll();

    friend class DeclarativeUiHandler;
};

#endif // UI_HANDLER_DECLARATIVE_P_H_

