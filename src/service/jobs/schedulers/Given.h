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

#ifndef JOBS_SCHEDULER_GIVEN_H
#define JOBS_SCHEDULER_GIVEN_H

#include <jobs/Job.h>
#include <jobs/JobFactory.h>
#include <jobs/schedulers/Abstract.h>

#define DO_OR_DIE(Task, Death) new Jobs::Schedulers::Given(Task, nullptr, Death, true)

namespace Jobs {
namespace Schedulers {

/**
 * Given
 */
class Given: public Abstract {
    Q_OBJECT

public:
    Given(JobFactory * _condition, JobFactory * _then, JobFactory * _else, bool failOnElse, QObject * parent = nullptr);
    virtual ~Given();

protected:
    virtual void jobFinished(int result);

private:
    Given(const Given & original);
    Given & operator = (const Given & original);

    bool m_failOnElse;
};

} // namespace Schedulers
} // namespace Jobs

#endif // JOBS_SCHEDULER_GIVEN_H

