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

#include "query.h"
#include <QStringList>
#include <QDebug>

int main()
{
    using namespace KActivities::Stats;
    using namespace KActivities::Stats::Terms;

    {
        auto fancyQuery = AllResources
                              | Type("text")
                              | Type("image")
                              | Agent("okular")
                              | Ordering::HighScore;

        Query normalQuery(AllResources);
        normalQuery.addAgents(QStringList() << "okular");
        normalQuery.addTypes(QStringList() << "text" << "image");
        normalQuery.setOrdering(Ordering::HighScore);

        qDebug() << normalQuery;
        qDebug() << fancyQuery;
    }

    {
        auto fancyQuery = AllResources
                              | Type{"text", "image"}
                              | Agent{"okular"}
                              | Ordering::HighScore;

        Query normalQuery(AllResources);
        normalQuery.addAgents({"okular"});
        normalQuery.addTypes({"text", "image"});
        normalQuery.setOrdering(Ordering::HighScore);

        qDebug() << normalQuery;
        qDebug() << fancyQuery;
    }


    // for (const auto &result: query) {
    //
    // }

    // connect(query, SIGNAL(added(...)),
    //         ..., ...);
    // connect(query, SIGNAL(added(...)),
    //         ..., ...);
    // connect(query, SIGNAL(added(...)),
    //         ..., ...);

}

