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

#ifndef JOBS_SCHEDULER_SWITCH_H
#define JOBS_SCHEDULER_SWITCH_H

#include <jobs/Job.h>
#include <jobs/JobFactory.h>
#include <jobs/schedulers/Abstract.h>

namespace Jobs {
namespace Schedulers {

/**
 * Switch
 */
class Switch: public Abstract {
    Q_OBJECT

public:
    Switch(QObject * parent = nullptr);
    virtual ~Switch();

    Switch & operator << (JobFactory * other);
    Switch & operator << (Job * other);

protected:
    virtual void jobFinished(int result);

private:
    Switch(const Switch & original);
    Switch & operator = (const Switch & original);
};

} // namespace Schedulers
} // namespace Jobs

#endif // JOBS_SCHEDULER_SWITCH_H

