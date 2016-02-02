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

#include "activitysettings.h"

#include <QMessageBox>
#include <QProcess>

#include <KLocalizedString>

#include "dialog.h"

#include <kactivities/info.h>
#include <kactivities/controller.h>

ActivitySettings::ActivitySettings(QObject *parent)
    : QObject(parent)
{
}

ActivitySettings::~ActivitySettings()
{
}

void ActivitySettings::configureActivity(const QString &id)
{
    auto dialog = new Dialog(id);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
}

void ActivitySettings::newActivity()
{
    auto dialog = new Dialog();
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
}

void ActivitySettings::deleteActivity(const QString &id)
{
    KActivities::Info info(id);

    if (QMessageBox::question(Q_NULLPTR, i18n("Delete activity"),
                              i18n("Are you sure you want to delete '%1'?",
                                   info.name())) == QMessageBox::Yes) {
        KActivities::Controller().removeActivity(id);
    }
}

void ActivitySettings::configureActivities()
{
    QProcess::startDetached("kcmshell5", { "activities" });
}


#include "activitysettings.moc"


