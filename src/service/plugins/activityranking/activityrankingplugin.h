/*
 *   Copyright (C) 2011, 2012 Ivan Cukic <ivan.cukic@kde.org>
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2 of
 *   the License, or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PLUGINS_ACTIVITY_RANKING_ACTIVITY_RANKING_PLUGIN_H
#define PLUGINS_ACTIVITY_RANKING_ACTIVITY_RANKING_PLUGIN_H

#include <Plugin.h>

#include <utils/nullptr.h>
#include <utils/override.h>
#include <utils/d_ptr.h>

class ActivityRankingPlugin: public Plugin
{
    Q_OBJECT

public:
    explicit ActivityRankingPlugin(QObject *parent = nullptr, const QVariantList & args = QVariantList());
    ~ActivityRankingPlugin();

    virtual bool init(const QHash < QString, QObject * > & modules) _override;

private:
    D_PTR;
};

#endif // PLUGINS_ACTIVITY_RANKING_ACTIVITY_RANKING_PLUGIN_H

