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

#include <QApplication>
#include <QListView>
#include <QDebug>

#include <resultset.h>
#include <resultmodel.h>

namespace KAStats = KActivities::Experimental::Stats;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    using namespace KAStats;
    using namespace KAStats::Terms;

    ResultSet results(UsedResources | Agent{"gvim"});

    int count = 20;
    for (const auto& result: results) {
        qDebug() << "Result:" << result.title << result.resource;
        if (count -- == 0) break;
    }

    ResultModel model(UsedResources | Agent{"gvim"});
    model.setItemCountLimit(50);

    QListView view;
    view.setModel(&model);

    view.show();

    return app.exec();
}

