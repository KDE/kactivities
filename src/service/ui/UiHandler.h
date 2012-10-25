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

#ifndef UI_UI_HANDLER_H
#define UI_UI_HANDLER_H

#include <kdemacros.h>
#include <KPluginFactory>
#include <KPluginLoader>

#include <utils/override.h>
#include <utils/d_ptr.h>

#define KAMD_EXPORT_UI_HANDLER(ClassName, AboutData)                   \
    K_PLUGIN_FACTORY(ClassName##Factory, registerPlugin<ClassName>();) \
    K_EXPORT_PLUGIN(ClassName##Factory(AboutData))

class KDE_EXPORT UiHandler: public QObject {
    Q_OBJECT

public:
    UiHandler(QObject * parent);
    virtual ~UiHandler();

    virtual void message(const QString & title, const QString & message) = 0;
    virtual void askPassword(const QString & title, const QString & message,
            bool newPassword, bool unlockMode, QObject * receiver, const char * slot) = 0;
    virtual void ask(const QString & title, const QString & message,
            const QStringList & choices, QObject * receiver, const char * slot) = 0;
    virtual void setBusy(bool value) = 0;

private:
    D_PTR;
};

#endif // UI_UI_HANDLER_H

