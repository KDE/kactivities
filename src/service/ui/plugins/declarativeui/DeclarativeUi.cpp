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

#include "DeclarativeUi.h"
#include "DeclarativeUi_p.h"
#include "KDeclarativeMainWindow.h"
#include "KDeclarativeView.h"

#include <QMetaObject>
#include <QDeclarativeContext>
#include <QApplication>
#include <QX11Info>

#include <KDebug>
#include <KWindowSystem>

#ifdef Q_WS_X11
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#endif

#include <utils/d_ptr_implementation.h>

DeclarativeUiHandler::Private::Private()
    : window(nullptr), receiver(nullptr), slot(nullptr), showingSomething(false), showingBusy(false)
{
}

void DeclarativeUiHandler::Private::onCurrentActivityChanged(const QString & activity)
{
    Q_UNUSED(activity);

    kDebug() << activity;
    // close();
}

void DeclarativeUiHandler::Private::showWindow()
{
    kDebug() << "showing input window";

    window->show();
    showingSomething = true;

    // TODO: We need some magic here for kwin to know that this needs to be a top-level window
    // and that it doesn't depend on the current activity
    // TODO: Test whether this magic works

    window->setWindowState(Qt::WindowMaximized);
    KWindowSystem::setOnAllDesktops(window->effectiveWinId(), true);
    KWindowSystem::setState(window->effectiveWinId(), NET::SkipTaskbar | NET::SkipPager | NET::KeepAbove | NET::Sticky | NET::StaysOnTop);
    KWindowSystem::raiseWindow(window->effectiveWinId());
    KWindowSystem::forceActiveWindow(window->effectiveWinId());

#ifdef Q_WS_X11
    Atom activitiesAtom = XInternAtom(QX11Info::display(), "_KDE_NET_WM_ACTIVITIES", False);

    XChangeProperty(QX11Info::display(), window->effectiveWinId(),
            activitiesAtom, XA_STRING, 8,
            PropModeReplace, (const unsigned char *)"ALL", 3);
#endif
    emit windowVisibleChanged();
}

void DeclarativeUiHandler::Private::hideWindow()
{
    showingSomething = false;

    if (!showingBusy) {
        window->hide();
    }
    emit windowVisibleChanged();
}

bool DeclarativeUiHandler::Private::isWindowVisible() const
{
    return showingSomething;
}

void DeclarativeUiHandler::Private::cancel()
{
    kDebug();

    returnPassword(QString());
    returnChoice(0);
    close();

    hideWindow();
}

void DeclarativeUiHandler::Private::close()
{
    kDebug();

    hideAll();
}

void DeclarativeUiHandler::Private::returnPassword(const QString & password)
{
    if (currentAction != PasswordAction) return;

    if (receiver && slot) {
        kDebug() << "receiver" << receiver->metaObject()->className() << slot;

        QMetaObject::invokeMethod(receiver, slot, Qt::QueuedConnection,
                    Q_ARG(QString, password));
        emit hideAll();
    }

    receiver = nullptr;
    slot = nullptr;

    hideWindow();
}

void DeclarativeUiHandler::Private::returnChoice(int index)
{
    if (currentAction != ChoiceAction) return;

    if (receiver && slot) {
        kDebug() << "receiver" << receiver->metaObject()->className() << slot;

        QMetaObject::invokeMethod(receiver, slot, Qt::QueuedConnection,
                    Q_ARG(int, index));
        emit hideAll();
    }

    receiver = nullptr;
    slot = nullptr;
    currentAction = NoAction;

    hideWindow();
}

DeclarativeUiHandler::DeclarativeUiHandler(QObject * parent, const QVariantList & args)
    : UiHandler(parent), d()
{
    Q_UNUSED(args);

    d->window = new KDeclarativeMainWindow();
    d->window->resize(800, 600);
    d->window->declarativeView()->rootContext()->setContextProperty("uihandler", d.get());
    d->window->declarativeView()->setPackageName("org.kde.ActivityManager.UiHandler");
}

DeclarativeUiHandler::~DeclarativeUiHandler()
{
    delete d->window;
}

void DeclarativeUiHandler::askPassword(const QString & title, const QString & message,
            bool newPassword, bool unlockMode, QObject * receiver, const char * slot)
{
    kDebug() << title << message;

    d->currentAction = Private::PasswordAction;
    d->receiver = receiver;
    d->slot     = slot;
    d->showWindow();

    emit d->askPassword(title, message, newPassword, unlockMode);
}

void DeclarativeUiHandler::ask(const QString & title, const QString & message,
        const QStringList & choices, QObject * receiver, const char * slot)
{
    kDebug() << title << message;

    d->currentAction = Private::ChoiceAction;
    d->receiver = receiver;
    d->slot     = slot;
    d->showWindow();

    emit d->ask(title, message, choices);
}

void DeclarativeUiHandler::message(const QString & title, const QString & message)
{
    kDebug() << title << message;

    d->showWindow();
    emit d->message(message);
}

void DeclarativeUiHandler::setBusy(bool value)
{
    kDebug() << value << d->showingSomething;
    d->showingBusy = value;

    if (!value && !d->showingSomething) {
        d->window->hide();
    }
}


KAMD_EXPORT_UI_HANDLER(DeclarativeUiHandler, "activitymanager_uihandler_declarative")
