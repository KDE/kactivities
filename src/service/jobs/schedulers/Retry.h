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

#ifndef JOBS_SCHEDULER_RETRY_H
#define JOBS_SCHEDULER_RETRY_H

#include <jobs/Job.h>
#include <jobs/JobFactory.h>
#include <jobs/schedulers/Abstract.h>

#define RETRY_JOB(Input, Test, Error) new Jobs::Schedulers::Retry(Input, Test, Error)

namespace Jobs {
namespace Schedulers {

/**
 * Retry
 */
class Retry: public Abstract {
    Q_OBJECT

public:
    Retry(JobFactory * _input, JobFactory * _test, JobFactory * _error, QObject * parent = nullptr);
    virtual ~Retry();

protected:
    virtual void jobFinished(int result);

private:
    Retry(const Retry & original);
    Retry & operator = (const Retry & original);

    bool m_failOnElse;
};

} // namespace Schedulers
} // namespace Jobs

#endif // JOBS_SCHEDULER_RETRY_H

