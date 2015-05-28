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

#include "window.h"

#include "ui_window.h"

#include <QDBusConnection>

Window::Window()
    : ui(new Ui::MainWindow())
    , slc(new org::kde::ActivityManager::SLC(
            "org.kde.ActivityManager",
            "/SLC",
            QDBusConnection::sessionBus(),
            this))
{
    ui->setupUi(this);

    connect(slc,  &org::kde::ActivityManager::SLC::focusChanged,
            this, &Window::focusChanged);
}

Window::~Window()
{
    delete ui;
}

void Window::focusChanged(const QString &uri, const QString &mimetype,
                          const QString &title)
{
    Q_UNUSED(mimetype);
    Q_UNUSED(title);
    ui->textCurrentResource->setText(uri);

}

