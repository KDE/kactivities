/*
 *   Copyright (C) 2010 Ivan Cukic <ivan.cukic(at)kde.org>
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

#include <ActivityManager.h>

//#include <QDebug>

#include <KAboutData>
#include <KCmdLineArgs>

#include <signal.h>
#include <stdlib.h>

#include "jobs/encryption/Common.h"

static void initSignalCatching();

int main(int argc, char ** argv)
{
    KAboutData about("kactivitymanagerd", 0, ki18n("KDE Activity Manager"), "2.0",
            ki18n("KDE Activity Management Service"),
            KAboutData::License_GPL,
            ki18n("(c) 2010, 2011, 2012 Ivan Cukic, Sebastian Trueg"), KLocalizedString(),
            "http://www.kde.org/");

    KCmdLineArgs::init(argc, argv, &about);

    ActivityManager application;

    initSignalCatching();

    return application.exec();
}

// Signal handling
static void signalHandler(int sig)
{
    Jobs::Encryption::Common::unmountAll();

    // something (probably ksmserver) has asked us to terminate.
    // If it is really ksmserver then the user is probably logging out, so we
    // had better gently stop now than be killed.
    if (sig == SIGTERM) {
        //qDebug() << "signalHandler(SIGTERM): stopping ActivityManager\n";

        ActivityManager::self()->Stop();
    }

    ::exit(EXIT_SUCCESS);
}

static void initSignalCatching() {
#ifndef Q_OS_WIN32
    struct sigaction action;

    ::sigemptyset(&action.sa_mask);
    action.sa_flags = 0;

    /* Use the sa_sigaction field because the handles has two additional parameters */
    action.sa_handler = signalHandler;

    ::sigaction(SIGINT,  &action, NULL);
    ::sigaction(SIGHUP,  &action, NULL);
    ::sigaction(SIGTERM, &action, NULL);
    ::sigaction(SIGSEGV, &action, NULL);
#endif
}
