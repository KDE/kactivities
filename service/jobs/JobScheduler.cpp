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

#include "JobScheduler.h"
#include "JobScheduler_p.h"

#include <unistd.h>
#include <QDebug>

JobScheduler::Private::Private(JobScheduler * parent)
    : q(parent)
{
}

void JobScheduler::Private::returnResult(KJob * job)
{
    if (type != FallibleJob && job->error() != 0 && q->error() == 0) {
        q->setError(job->error());
        q->setErrorText(job->errorText());
    }

    q->emitResult();
}

void JobScheduler::Private::jobFinished(KJob * job)
{
    if (lastJobStarted < 0) return;

    switch (type) {
        case OrderedJob:
        case FallibleJob:
            if (job->error() != 0) {
                returnResult(job);
            } else {
                startNextJob();
            }

            break;


        case IfElseJob:
            if (lastJobStarted == 0) {
                if (job->error() != 0) {
                    q->setError(job->error());
                    q->setErrorText(job->errorText());
                }

                // 0 - then branch
                // 1 - else branch
                startNextJob((job->error() == 0) ? 0 : 1);

            } else {
                returnResult(job);
            }

            break;


        case RetryJob:
            switch (lastJobStarted) {
                case 0:
                    // The test was started, if it fails, we are exiting
                    if (job->error() != 0) {
                        returnResult(job);

                    } else {
                        startNextJob();

                    }
                    break;

                case 1:
                    // If test fails, proceeding to exec the error process
                    // otherwise exiting
                    if (job->error() != 0) {
                        startNextJob();

                    } else {
                        returnResult(job);

                    }
                    break;

                case 2:
                    startNextJob(-3);
                    break;
            }

            break;
    }
}

void JobScheduler::Private::startNextJob(int skip)
{
    int index = lastJobStarted + 1 + skip;
    lastJobStarted = index;

    // If the index is not valid
    if (index < 0 || jobs.size() <= index) { q->emitResult(); return; }

    JobFactory * factory = jobs[index];

    // If the job factory is null, exit
    if (!factory) {
        if (type != IfElseJob) {
            q->setError(1);
            q->setErrorText("null job can't be executed");
        }
        q->emitResult();
        return;
    }

    // Starting the job
    KJob * job = jobs[index]->create(this);

    connect(job, SIGNAL(finished(KJob *)),
            this, SLOT(jobFinished(KJob *)));

    job->start();
}

JobScheduler::JobScheduler(QObject * parent)
    : Job(parent), d(new Private(this))
{
    d->lastJobStarted = -1;
    d->type = Private::OrderedJob;

    if (parent) {
        connect(
            this, SIGNAL(finished(KJob *)),
            this, SLOT(deleteLater()),
            Qt::QueuedConnection
        );
    }

    qDebug() << ">>>" << (void*)this << "created JobScheduler";
}

JobScheduler::~JobScheduler()
{
    qDebug() << ">>>" << (void*)this << "deleted JobScheduler";
    qDeleteAll(d->jobs);
    delete d;
}

JobScheduler & JobScheduler::ifJob(JobFactory * test, JobFactory * onSuccess, JobFactory * onFailed)
{
    JobScheduler * result = new JobScheduler();

    result->d->type = Private::IfElseJob;
    result->d->jobs << test << onSuccess << onFailed;

    return *result;
}

JobScheduler & JobScheduler::orderedJob(JobFactory * job1, JobFactory * job2, JobFactory * job3, JobFactory * job4)
{
    QList < JobFactory * > jobs;

    if (job1) jobs << job1;
    if (job2) jobs << job2;
    if (job3) jobs << job3;
    if (job4) jobs << job4;

    return orderedJob(jobs);
}

JobScheduler & JobScheduler::orderedJob(QList < JobFactory * > jobs)
{
    JobScheduler * result = new JobScheduler();

    result->d->type = Private::OrderedJob;
    result->d->jobs = jobs;

    return *result;
}

JobScheduler & JobScheduler::fallibleJob(JobFactory * job1, JobFactory * job2, JobFactory * job3, JobFactory * job4)
{
    QList < JobFactory * > jobs;

    if (job1) jobs << job1;
    if (job2) jobs << job2;
    if (job3) jobs << job3;
    if (job4) jobs << job4;

    return fallibleJob(jobs);
}

JobScheduler & JobScheduler::fallibleJob(QList < JobFactory * > jobs)
{
    JobScheduler * result = new JobScheduler();

    result->d->type = Private::FallibleJob;
    result->d->jobs = jobs;

    return *result;
}

JobScheduler & JobScheduler::retryJob(JobFactory * input, JobFactory * test, JobFactory * error)
{
    JobScheduler * result = new JobScheduler();

    result->d->type = Private::RetryJob;
    result->d->jobs << input << test << error;

    return *result;
}

JobScheduler & JobScheduler::operator << (Job * other)
{
    if (d->type == Private::OrderedJob ||
        d->type == Private::FallibleJob
            ) {
        d->jobs << JobFactory::wrap(other);
    }

    return *this;
}

JobScheduler & JobScheduler::operator << (JobFactory * other)
{
    if (d->type == Private::OrderedJob ||
        d->type == Private::FallibleJob
            ) {
        d->jobs << other;
    }

    return *this;
}

void JobScheduler::start()
{
    if (d->jobs.size() == 0) emitResult();

    d->startNextJob();
}

