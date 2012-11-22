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

#include <QThread>
#include <QCoreApplication>
#include <QMetaObject>

#include <KDebug>
#include <KConfigGroup>
#include <kdeclarative.h>

#include <Plugin.h>

#include <config-features.h>
#include <utils/nullptr.h>
#include <utils/d_ptr_implementation.h>

class Ui::Private {
public:
    static Ui * s_instance;
    static UiHandler * ui;
};

Ui * Ui::Private::s_instance = nullptr;
UiHandler * Ui::Private::ui = nullptr;

Ui * Ui::self()
{
    if (!Private::s_instance) {
        // Making sure Ui object is in the GUI thread
        Private::s_instance = new Ui(nullptr);
        Private::s_instance->moveToThread(QCoreApplication::instance()->thread());
        qRegisterMetaType<const char *>("const char *");
    }

    return Private::s_instance;
}


Ui::Ui(QObject * parent)
    : QObject(parent), d()
{
    QString handlerLibrary = KDIALOG_UI_HANDLER;

    const QString target = KDeclarative::componentsTarget();
    if (target != KDeclarative::defaultComponentsTarget()) {
        handlerLibrary = DECLARATIVE_UI_HANDLER;
    }

    KPluginFactory * factory = KPluginLoader(handlerLibrary).factory();

    if (factory) {
        d->ui = factory->create < UiHandler > (this);
    }
}

Ui::~Ui()
{
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
    // Ui most probably doesn't live in the same thread as the caller
    // Ui::self()->_askPassword(title, message, newPassword, unlockMode, receiver, slot);
    QMetaObject::invokeMethod(Ui::self(), "_askPassword", Qt::QueuedConnection,
            Q_ARG(QString, title),
            Q_ARG(QString, message),
            Q_ARG(bool, newPassword),
            Q_ARG(bool, unlockMode),
            Q_ARG(QObject *, receiver),
            Q_ARG(const char *, slot)
        );
}

void Ui::ask(const QString & title, const QString & message,
        const QStringList & choices,
        QObject * receiver, const char * slot)
{
    // Ui most probably doesn't live in the same thread as the caller
    // Ui::self()->_ask(title, message, choices, receiver, slot);
    QMetaObject::invokeMethod(Ui::self(), "_ask", Qt::QueuedConnection,
            Q_ARG(QString, title),
            Q_ARG(QString, message),
            Q_ARG(QStringList, choices),
            Q_ARG(QObject *, receiver),
            Q_ARG(const char *, slot)
        );
}

void Ui::message(const QString & title, const QString & message)
{
    // Ui most probably doesn't live in the same thread as the caller
    // Ui::self()->_message(title, message);
    QMetaObject::invokeMethod(Ui::self(), "_message", Qt::QueuedConnection,
            Q_ARG(QString, title),
            Q_ARG(QString, message)
        );
}

void Ui::setBusy(bool value)
{
    // Ui most probably doesn't live in the same thread as the caller
    // Ui::self()->_setBusy(value);
    QMetaObject::invokeMethod(Ui::self(), "_setBusy", Qt::QueuedConnection,
            Q_ARG(bool, value)
        );
}

void Ui::unsetBusy()
{
    _setBusy(false);
}

