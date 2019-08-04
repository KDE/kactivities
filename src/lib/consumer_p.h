/*
 *   Copyright (C) 2010 - 2016 by Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) version 3, or any
 *   later version accepted by the membership of KDE e.V. (or its
 *   successor approved by the membership of KDE e.V.), which shall
 *   act as a proxy defined in Section 6 of version 3 of the license.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library.
 *   If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ACTIVITIES_CONSUMER_P_H
#define ACTIVITIES_CONSUMER_P_H

#include "consumer.h"

#include <memory>


#include "activitiescache_p.h"

namespace KActivities {

class ConsumerPrivate : public QObject {
    Q_OBJECT

public:
    ConsumerPrivate();

    std::shared_ptr<ActivitiesCache> cache;

public Q_SLOTS:
    void setServiceStatus(Consumer::ServiceStatus status);

Q_SIGNALS:
    void serviceStatusChanged(Consumer::ServiceStatus status);

};

} // namespace KActivities

#endif // ACTIVITIES_CONSUMER_P_H
