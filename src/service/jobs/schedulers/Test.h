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

#ifndef JOBS_SCHEDULER_TEST_H
#define JOBS_SCHEDULER_TEST_H

#include <jobs/Job.h>
#include <jobs/JobFactory.h>
#include <jobs/schedulers/Abstract.h>

#define TEST_JOB(TestJob, Expect) new Jobs::Schedulers::Test(TestJob, Expect)

namespace Jobs {
namespace Schedulers {

/**
 * Test
 */
class Test: public Abstract {
    Q_OBJECT

public:
    Test(JobFactory * job, int expectedResult, QObject * parent = nullptr);
    virtual ~Test();

protected:
    virtual void jobFinished(int result);

private:
    Test(const Test & original);
    Test & operator = (const Test & original);

    int m_expectedResult;
};

} // namespace Schedulers
} // namespace Jobs

#endif // JOBS_SCHEDULER_TEST_H

