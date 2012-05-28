/*
 *   Copyright (C) 2012 Ivan Cukic <ivan.cukic(at)kde.org>
 *   Copyright (C) 2011 Marco Martin <mart@kde.org>
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

#include "Ui.h"
#include "UiHandler.h"
#include <config-features.h>
#include <SharedInfo.h>

#include <KDebug>

class Ui::Private {
public:
    static Ui * s_instance;
    static UiHandler * ui;
};

Ui * Ui::Private::s_instance = NULL;
UiHandler * Ui::Private::ui = NULL;

Ui * Ui::self()
{
    if (!Private::s_instance) {
        Private::s_instance = new Ui(NULL);
    }

    return Private::s_instance;
}


Ui::Ui(QObject * parent)
    : QObject(parent), d(new Private())
{
    QString handlerLibrary = KDIALOG_UI_HANDLER;

    QString platform = getenv("KDE_PLASMA_COMPONENTS_PLATFORM"); // krazy:exclude=syscalls
    if (platform.isEmpty()) {
        KConfigGroup cg(KSharedConfig::openConfig("kdeclarativerc"), "Components-platform");
        platform = cg.readEntry("name", "desktop");
    }

    if (platform != "desktop")
        handlerLibrary = DECLARATIVE_UI_HANDLER;

    KPluginFactory * factory = KPluginLoader(handlerLibrary).factory();

    if (factory) {
        d->ui = factory->create < UiHandler > (this);
    }
}

Ui::~Ui()
{
    delete d;
}

void Ui::_askPassword(const QString & title, const QString & message, bool newPassword, bool unlockMode,
        QObject * receiver, const char * slot)
{
    d->ui->askPassword(title, message, newPassword, unlockMode, receiver, slot);
}

void Ui::_ask(const QString & title, const QString & message, const QStringList & choices,
        QObject * receiver, const char * slot)
{
    d->ui->ask(title, message, choices, receiver, slot);
}

void Ui::_message(const QString & title, const QString & message)
{
    d->ui->message(title, message);
}

void Ui::_setBusy(bool value)
{
    d->ui->setBusy(value);
}

void Ui::askPassword(const QString & title, const QString & message,
        bool newPassword, bool unlockMode,
        QObject * receiver, const char * slot)
{
    Ui::self()->_askPassword(title, message, newPassword, unlockMode, receiver, slot);
}

void Ui::ask(const QString & title, const QString & message,
        const QStringList & choices,
        QObject * receiver, const char * slot)
{
    Ui::self()->_ask(title, message, choices, receiver, slot);
}

void Ui::message(const QString & title, const QString & message)
{
    Ui::self()->_message(title, message);
}

void Ui::setBusy(bool value)
{
    Ui::self()->_setBusy(value);
}

void Ui::unsetBusy()
{
    _setBusy(false);
}

