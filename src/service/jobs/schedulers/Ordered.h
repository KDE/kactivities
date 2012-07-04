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

#ifndef JOBS_SCHEDULER_ORDERED_H
#define JOBS_SCHEDULER_ORDERED_H

#include <jobs/Job.h>
#include <jobs/JobFactory.h>
#include <jobs/schedulers/Abstract.h>

#define DEFINE_ORDERED_SCHEDULER(Name) Jobs::Schedulers::Ordered & Name \
    = * (new Jobs::Schedulers::Ordered())

namespace Jobs {
namespace Schedulers {

/**
 * Ordered
 */
class Ordered: public Abstract {
    Q_OBJECT

public:
    Ordered(QObject * parent = nullptr);
    virtual ~Ordered();

    Ordered & operator << (JobFactory * other);
    Ordered & operator << (Job * other);

protected:
    virtual void jobFinished(int result);

private:
    Ordered(const Ordered & original);
    Ordered & operator = (const Ordered & original);
};

} // namespace Schedulers
} // namespace Jobs

#endif // JOBS_SCHEDULER_ORDERED_H

