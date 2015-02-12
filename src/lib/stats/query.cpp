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
#include <QDebug>

namespace KActivities {
namespace Experimental {
namespace Stats {

class Query::Private {
public:
    Terms::Select   selection;
    QStringList     types;
    QStringList     agents;
    QStringList     activities;
    Terms::Order    ordering;
};

Query::Query(Terms::Select selection)
    : d(new Private())
{
    d->selection = selection;
}

Query::Query(Query &&source)
    : d(nullptr)
{
    std::swap(d, source.d);
}

Query::Query(const Query &source)
    : d(new Private(*source.d))
{
}

Query &Query::operator= (Query source)
{
    std::swap(d, source.d);
    return *this;
}


Query::~Query()
{
    delete d;
}

bool Query::operator== (const Query &right) const
{
    return selection()  == right.selection() &&
           types()      == right.types() &&
           agents()     == right.agents() &&
           activities() == right.activities() &&
           selection()  == right.selection();
}

bool Query::operator!= (const Query &right) const
{
    return !(*this == right);
}




void Query::addTypes(const QStringList &types)
{
    d->types << types;
}

void Query::addAgents(const QStringList &agents)
{
    d->agents << agents;
}

void Query::addActivities(const QStringList &activities)
{
    d->activities << activities;
}

void Query::setOrdering(Terms::Order ordering)
{
    d->ordering = ordering;
}

void Query::setSelection(Terms::Select selection)
{
    d->selection = selection;
}


QStringList Query::types() const
{
    return d->types.size() ? d->types : QStringList(":any");
}

QStringList Query::agents() const
{
    return d->agents.size() ? d->agents : QStringList(":current");
}

QStringList Query::activities() const
{
    return d->activities.size() ? d->activities : QStringList(":current");
}

Terms::Order Query::ordering() const
{
    return d->ordering;
}

Terms::Select Query::selection() const
{
    return d->selection;
}


void Query::clearTypes()
{
    d->types.clear();
}

void Query::clearAgents()
{
    d->agents.clear();
}

void Query::clearActivities()
{
    d->activities.clear();
}

} // namespace Stats
} // namespace Experimental
} // namespace KActivities

namespace KAStats = KActivities::Experimental::Stats;

QDebug operator<<(QDebug dbg, const KAStats::Query &query)
{
    using namespace KAStats::Terms;

    dbg.nospace()
        << "Query { "
        << query.selection()
        << ", " << Type(query.types())
        << ", " << Agent(query.agents())
        << ", " << Activity(query.activities())
        << ", " << query.ordering()
        << " }";
    return dbg;
}
