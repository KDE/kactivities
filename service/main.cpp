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

#include <KAboutData>
#include <KCmdLineArgs>

int main(int argc, char ** argv)
{
    KAboutData about("kactivitymanagerd", 0, ki18n("KDE Activity Manager"), "1.0",
            ki18n("KDE Activity Management Service"),
            KAboutData::License_GPL,
            ki18n("(c) 2010 Ivan Cukic, Sebastian Trueg"), KLocalizedString(),
            "http://www.kde.org/");

    KCmdLineArgs::init(argc, argv, &about);

    ActivityManager application;

    return application.exec();
}
