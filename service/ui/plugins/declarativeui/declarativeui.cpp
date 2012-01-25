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

#include "declarativeui.h"
#include "kdeclarativemainwindow.h"
#include "kdeclarativeview.h"

class DeclarativeUiHandler::Private {
public:
    KDeclarativeMainWindow * window;
};

DeclarativeUiHandler::DeclarativeUiHandler(QObject * parent, const QVariantList & args)
    : UiHandler(parent), d(new Private())
{
    Q_UNUSED(args);

    d->window = new KDeclarativeMainWindow();
    d->window->resize(800, 600);
    d->window->show();
    d->window->declarativeView()->setPackageName("org.kde.ActivityManager.UiHandler");
}

DeclarativeUiHandler::~DeclarativeUiHandler()
{
    delete d->window;
    delete d;
}

QString DeclarativeUiHandler::askPassword(const QString & title, const QString & message, bool newPassword)
{
    return QString("ivan");
}

void DeclarativeUiHandler::message(const QString & title, const QString & message)
{
}

KAMD_EXPORT_UI_HANDLER(DeclarativeUiHandler, "activitymanager_uihandler_declarative")
