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

#include "Switch.h"

namespace Jobs {
namespace Schedulers {

Switch::Switch(QObject * parent)
    : Abstract(parent)
{
}

Switch::~Switch()
{
}

Switch & Switch::operator << (JobFactory * job)
{
    addJob(job);

    return *this;
}

Switch & Switch::operator << (Job * job)
{
    addJob(job);

    return *this;
}

void Switch::jobFinished(int result)
{
    if (lastJobStarted() != 0) {
        // We have executed the switch command
        returnResult(result);
        return;
    }

    if (result >= 0) {
        // The test job returned a positive value, this means it failed
        returnResult(1);
        return;

    } else {
        startJob(-result);

    }
}



} // namespace Schedulers
} // namespace Jobs

