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

#include "Retry.h"

namespace Jobs {
namespace Schedulers {

Retry::Retry(JobFactory * _input, JobFactory * _test, JobFactory * _error, QObject * parent)
    : Abstract(parent)
{
    addJob(_input);
    addJob(_test);
    addJob(_error);
}

Retry::~Retry()
{
}

void Retry::jobFinished(int result)
{
    switch (lastJobStarted()) {
        case 0: {
            // We have executed the input, and it has finished. If the input
            // failed, we want to exit with an error

            if (result != 0) {
                returnResult(1);
                return;
            }

            startJob(1);

        } break;

        case 1: {
            // The test finished. If it fails, executing the error job

            if (result == 0) {
                returnResult(0);
                return;
            }

            startJob(2);
        } break;

        case 2: {
            // error executed, restarting
            startJob(0);
        } break;
    }
}



} // namespace Schedulers
} // namespace Jobs

