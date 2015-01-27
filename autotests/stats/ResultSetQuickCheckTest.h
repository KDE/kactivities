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

#ifndef RESULTSET_QUICKCHECK_TEST_H
#define RESULTSET_QUICKCHECK_TEST_H

#include <common/test.h>

#include <controller.h>
#include <memory>

#include <boost/container/flat_set.hpp>
#include <QScopedPointer>

#include "quickcheck/tables/ResourceScoreCache.h"
#include "quickcheck/tables/ResourceInfo.h"

using boost::container::flat_set;

class ResultSetQuickCheckTest : public Test {
    Q_OBJECT
public:
    ResultSetQuickCheckTest(QObject *parent = nullptr);

private Q_SLOTS:
    void initTestCase();

    void testUsedResourcesForAgents();
    void testUsedResourcesForActivities();

    void cleanupTestCase();

public:
    QScopedPointer<KActivities::Consumer> activities;

    struct PrimaryKeyOrder {
        template <typename T>
        bool operator() (const T &left,
                         const T &right) const
        {
            return left.primaryKey() < right.primaryKey();
        }
    };

    TABLE(ResourceScoreCache) resourceScoreCaches;
    TABLE(ResourceInfo)       resourceInfos;

    QString randItem(const QStringList &choices) const;

    QStringList activitiesList;
    QStringList agentsList;
    QStringList typesList;
    QStringList resourcesList;

    void generateActivitiesList();
    void generateAgentList();
    void generateTypesList();
    void generateResourcesList();

    void generateResourcesInfos();
    void generateResouceScoreCaches();

    void pushToDatabase();
    void pullFromDatabase();
};


#endif /* RESULTSET_QUICKCHECK_TEST_H */

