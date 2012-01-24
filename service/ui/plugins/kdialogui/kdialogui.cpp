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

#include "kdialogui.h"

#include <knewpassworddialog.h>
#include <kpassworddialog.h>

#include <QMessageBox>

KDialogUiHandler::KDialogUiHandler(QObject * parent, const QVariantList & args)
    : UiHandler(parent)
{
    Q_UNUSED(args)
}

KDialogUiHandler::~KDialogUiHandler()
{
}

QString KDialogUiHandler::askPassword(const QString & title, const QString & message, bool newPassword)
{
    if (newPassword) {
        KNewPasswordDialog dialog;

        dialog.setPrompt(message);
        dialog.setWindowTitle(title);

        if (dialog.exec()) {
            return dialog.password();
        }

        return QString();

    } else {
        KPasswordDialog dialog;

        dialog.setPrompt(message);
        dialog.setWindowTitle(title);

        if (dialog.exec()) {
            return dialog.password();
        }

        return QString();

    }
}

void KDialogUiHandler::message(const QString & title, const QString & message)
{
    QMessageBox::information(NULL, title, message);
}

KAMD_EXPORT_UI_HANDLER(KDialogUiHandler, "activitymanager_uihandler_kdialog")
