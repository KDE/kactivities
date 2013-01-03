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

#include "Test.h"

#include <QDebug>

namespace Jobs {
namespace Schedulers {

Test::Test(JobFactory * job, int expectedResult, QObject * parent)
    : Abstract(parent), m_expectedResult(expectedResult)
{
    addJob(job);
}

Test::~Test()
{
}

void Test::jobFinished(int result)
{
    qDebug() << "Returned" << result << "expected" << m_expectedResult;

    returnResult(m_expectedResult == result ? 0 : 1);
}



} // namespace Schedulers
} // namespace Jobs

