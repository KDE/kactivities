/*
 * Copyright (c) 2010 Ivan Cukic <ivan.cukic(at)kde.org>
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

#ifndef ACTIVITIES_CONSUMER_P_H
#define ACTIVITIES_CONSUMER_P_H

#include "activitymanager_interface.h"
#include "consumer.h"

namespace KActivities {

class ConsumerPrivate: public QObject {
    Q_OBJECT
public:

Q_SIGNALS:
    void serviceStatusChanged(KActivities::Consumer::ServiceStatus status);

public Q_SLOTS:
    void setServicePresent(bool present);

};

} // namespace KActivities

#endif // ACTIVITIES_CONSUMER_P_H

