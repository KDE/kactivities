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

#include "Ordered.h"

#include <QDebug>

namespace Jobs {
namespace Schedulers {

Ordered::Ordered(QObject * parent)
    : Abstract(parent)
{
}

Ordered::~Ordered()
{
}

Ordered & Ordered::operator << (JobFactory * job)
{
    addJob(job);

    return *this;
}

Ordered & Ordered::operator << (Job * job)
{
    addJob(job);

    return *this;
}

void Ordered::jobFinished(int result)
{
    if (result != 0) {
        qDebug() << "So... we got an error" << result;
        setError(result);
        emitResult();

    } else if (lastJobStarted() == jobCount() - 1) {
        qDebug() << "no error, no jobs";
        emitResult();

    } else {
        qDebug() << "next job please";
        startJob(lastJobStarted() + 1);

    }
}



} // namespace Schedulers
} // namespace Jobs

