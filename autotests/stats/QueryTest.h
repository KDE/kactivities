/*
 *   Copyright (C) 2015 Ivan Cukic <ivan.cukic(at)kde.org>
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

#ifndef OFFLINETEST_H
#define OFFLINETEST_H

#include <common/test.h>

#include <controller.h>

#include <QScopedPointer>

class QueryTest : public Test {
    Q_OBJECT
public:
    QueryTest(QObject *parent = nullptr);

private Q_SLOTS:
    void initTestCase();

    void testDefaults();
    void testDebuggingOutput();

    void testDerivationFromDefault();
    void testDerivationFromCustom();

    void testNormalSyntaxAgentManipulation();
    void testNormalSyntaxTypeManipulation();
    void testNormalSyntaxActivityManipulation();
    void testNormalSyntaxOrderingManipulation();

    void testFancySyntaxBasic();
    void testFancySyntaxAgentDefinition();
    void testFancySyntaxTypeDefinition();
    void testFancySyntaxActivityDefinition();
    void testFancySyntaxOrderingDefinition();

    void cleanupTestCase();

private:

};


#endif /* OFFLINETEST_H */

