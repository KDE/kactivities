/*
 *   Copyright (C) 2013 Ivan Cukic <ivan.cukic(at)kde.org>
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

#ifndef RESULTSETTEST_H
#define RESULTSETTEST_H

#include <common/test.h>

#include <controller.h>
#include <memory>

#include <QScopedPointer>

class ResultSetTest : public Test {
    Q_OBJECT
public:
    ResultSetTest(QObject *parent = nullptr);

private Q_SLOTS:
    void initTestCase();

    void testLinkedResources();
    void testUsedResources();

    void cleanupTestCase();

};


#endif /* RESULTSETTEST_H */

