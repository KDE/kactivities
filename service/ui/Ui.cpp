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

#include "Ui.h"
#include "UiHandler.h"
#include <config-features.h>

#include <QThread>

#include <KDebug>

class Ui::Private {
public:
    class Function: public QThread {
    public:
        Function(const QString & _title, const QString & _message, bool _newPassword,
                 QObject * _receiver, const char * _slot)
            : title(_title), message(_message), newPassword(_newPassword),
              receiver(_receiver), slot(_slot)
        {
        }

        void run() {
            const QString & password = ui->askPassword(title, message, newPassword);

            kDebug() << "Got password .... sending it to" << receiver << slot;

            QMetaObject::invokeMethod(receiver, slot, Qt::QueuedConnection,
                    Q_ARG(QString, password));

            deleteLater();
        }

    private:
        QString title;
        QString message;
        bool newPassword;
        QObject * receiver;
        const char * slot;
    };

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
    KPluginFactory * factory = KPluginLoader(UI_HANDLER).factory();

    if (factory) {
        d->ui = factory->create < UiHandler > (this);

        if (d->ui) {
            d->ui->init();
        }
    }
}

Ui::~Ui()
{
    delete d;
}

void Ui::_askPassword(const QString & title, const QString & message, bool newPassword,
        QObject * receiver, const char * slot)
{
    (new Private::Function(title, message, newPassword, receiver, slot))->run();
}

void Ui::_message(const QString & title, const QString & message)
{
    d->ui->message(title, message);
}

void Ui::askPassword(const QString & title, const QString & message,
        bool newPassword,
        QObject * receiver, const char * slot)
{
    Ui::self()->_askPassword(title, message, newPassword, receiver, slot);
}

void Ui::message(const QString & title, const QString & message)
{
    Ui::self()->_message(title, message);
}

