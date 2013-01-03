/*
 * Copyright (c) 2010, 2011, 2012 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "controller.h"
#include "consumer_p.h"
#include "manager_p.h"

#include <QObject>
#include <QDebug>

namespace KActivities {

class ControllerPrivate: public QObject {
public:
    ControllerPrivate(Controller * parent)
        : q(parent)
    {
    }

private:
    Controller * const q;
};

Controller::Controller(QObject * parent)
    : Consumer(parent), d(new ControllerPrivate(this))
{
}

Controller::~Controller()
{
    delete d;
}

void Controller::setActivityName(const QString & id, const QString & name)
{
    Manager::activities()->SetActivityName(id, name);
}

void Controller::setActivityIcon(const QString & id, const QString & icon)
{
    Manager::activities()->SetActivityIcon(id, icon);
}

void Controller::setActivityEncrypted(const QString & id, bool encrypted)
{
    Q_UNUSED(id)
    Q_UNUSED(encrypted)
    // Manager::activities()->SetActivityEncrypted(id, encrypted);
}

bool Controller::setCurrentActivity(const QString & id)
{
    return Manager::activities()->SetCurrentActivity(id);
}

QString Controller::addActivity(const QString & name)
{
    return Manager::activities()->AddActivity(name);
}

void Controller::removeActivity(const QString & id)
{
    Manager::activities()->RemoveActivity(id);
}

void Controller::stopActivity(const QString & id)
{
    Manager::activities()->StopActivity(id);
}

void Controller::startActivity(const QString & id)
{
    Manager::activities()->StartActivity(id);
}

} // namespace KActivities

#include "controller.moc"
