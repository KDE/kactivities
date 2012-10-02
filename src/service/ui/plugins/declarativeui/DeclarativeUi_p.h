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

#ifndef UI_PLUGINS_DECLARATIVEUI_DECLARATIVE_UI_P_H
#define UI_PLUGINS_DECLARATIVEUI_DECLARATIVE_UI_P_H

#include "DeclarativeUi.h"
#include "KDeclarativeMainWindow.h"

#include <QObject>

class DeclarativeUiHandler::Private: public QObject {
    Q_OBJECT
    Q_PROPERTY(bool windowVisible READ isWindowVisible NOTIFY windowVisibleChanged())

public:
    Private();
    KDeclarativeMainWindow * window;
    QObject * receiver;
    const char * slot;
    bool showingSomething: 1;
    bool showingBusy: 1;

    enum Action {
        NoAction,
        PasswordAction,
        ChoiceAction
    } currentAction;

    void showWindow();
    bool isWindowVisible() const;

public Q_SLOTS:
    void onCurrentActivityChanged(const QString & activity);
    void returnPassword(const QString & password);
    void returnChoice(int index);
    void cancel();
    void close();
    void hideWindow();

Q_SIGNALS:
    void message(const QString & message);
    void askPassword(const QString & title, const QString & message, bool newPassword, bool unlockMode);
    void ask(const QString & title, const QString & message, const QStringList & choices);
    void hideAll();
    void windowVisibleChanged();

    friend class DeclarativeUiHandler;
};

#endif // UI_PLUGINS_DECLARATIVEUI_DECLARATIVE_UI_P_H

