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

#ifndef JOBSCHEDULER_H_
#define JOBSCHEDULER_H_

#include "Job.h"
#include "JobFactory.h"

#define JOB_SCHEDULER(Name) JobScheduler & Name = * (new JobScheduler())

#define DO_JOB &JobScheduler::ifJob(
#define OR_DIE(ErrorJob) , NULL, ErrorJob)

#define FALLIBLE_JOB &JobScheduler::fallibleJob
#define RETRY_JOB &JobScheduler::retryJob

void JobSchedulerTestExecute();

/**
 * JobScheduler
 */
class JobScheduler: public Job {
    Q_OBJECT

public:
    JobScheduler(QObject * parent = 0);
    virtual ~JobScheduler();

    static JobScheduler & orderedJob(JobFactory * job1, JobFactory * job2 = 0, JobFactory * job3 = 0, JobFactory * job4 = 0);
    static JobScheduler & orderedJob(QList < JobFactory * > jobs);

    static JobScheduler & ifJob(JobFactory * test, JobFactory * onSuccess, JobFactory * onFailed);

    static JobScheduler & fallibleJob(JobFactory * job1, JobFactory * job2 = 0, JobFactory * job3 = 0, JobFactory * job4 = 0);
    static JobScheduler & fallibleJob(QList < JobFactory * > jobs);

    static JobScheduler & retryJob(JobFactory * input, JobFactory * test, JobFactory * error);

    JobScheduler & operator << (JobFactory * other);
    JobScheduler & operator << (Job * other);

    virtual void start();

private:
    JobScheduler(const JobScheduler & original);
    JobScheduler & operator = (const JobScheduler & original);

    class Private;
    Private * const d;
    friend class Private;
};

// class JobScheduler


#endif // JOBSCHEDULER_H_

