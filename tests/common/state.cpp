/*
 *   Copyright (C) 2014 David Edmundson <davidedmundson@kde.org>
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

#include <KActivities/Consumer>

#include <QCoreApplication>
#include <QDebug>

QString serviceStatusToString(KActivities::Consumer::ServiceStatus status)
{
    switch (status) {
        case KActivities::Consumer::NotRunning:
            return "not running";
        case KActivities::Consumer::Unknown:
            return "unknown";
        case KActivities::Consumer::Running:
            return "running";
    }
    return QString();
}

int main(int argc, char ** argv)
{
    QCoreApplication app(argc, argv);
    auto consumer = new KActivities::Consumer(0);

    qDebug() << "********** Initial state is " <<  serviceStatusToString(consumer->serviceStatus());

    QObject::connect(consumer, &KActivities::Consumer::serviceStatusChanged, [=]() {
        qDebug() << "********** State changed to " << serviceStatusToString(consumer->serviceStatus());
    });

    return app.exec();
}


