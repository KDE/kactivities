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

// Self
#include "resultmodel.h"

// Qt
#include <QDebug>

// Local
#include "resultset.h"
#include "resultwatcher.h"

#define CHUNK_SIZE 10
#define DEFAULT_ITEM_COUNT_LIMIT 20

namespace KActivities {
namespace Experimental {
namespace Stats {

class ResultModel::Private {
public:
    Private(Query query, ResultModel *parent)
        : results(query)
        , watcher(query)
        , itemCountLimit(DEFAULT_ITEM_COUNT_LIMIT)
        , q(parent)
    {
    }

    ResultSet results;
    ResultWatcher watcher;

    int itemCountLimit;

    ResultSet::const_iterator resultIt;
    QList<ResultSet::Result> cache;

    void fetchMore(bool emitChanges)
    {
        if (!resultIt.isSourceValid()) {
            // We haven't loaded anything yet
            resultIt = results.begin();
            Q_ASSERT(resultIt.isSourceValid());

        };

        int chunkSize = CHUNK_SIZE;
        int insertedCount = 0;
        const int previousSize = cache.size();

        while ((chunkSize --> 0) && (cache.size() < itemCountLimit) &&
                resultIt != results.end()) {
            cache.append(*resultIt);
            ++resultIt;
            ++insertedCount;
        }

        if (emitChanges) {
            q->beginInsertRows(QModelIndex(),
                               previousSize,
                               cache.size() + 1);
            q->endInsertRows();
        }
    }

private:
    ResultModel *const q;

};

ResultModel::ResultModel(Query query, QObject *parent)
    : d(new Private(query, this))
{
    // d->fetchMore(false);
}

ResultModel::~ResultModel()
{
    delete d;
}

QVariant ResultModel::data(const QModelIndex &item, int role) const
{
    const auto row = item.row();

    if (row >= d->cache.size()) return QVariant();

    const auto &result = d->cache[row];

    return role == Qt::DisplayRole ? (result.title + " " + result.resource)
         : role == ResourceRole    ? result.resource
         : role == TitleRole       ? result.title
         : role == ScoreRole       ? result.score
         : QVariant()
         ;
}

QVariant ResultModel::headerData(int section, Qt::Orientation orientation,
                                 int role) const
{
    return QVariant();
}

int ResultModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : d->cache.size();
}

void ResultModel::fetchMore(const QModelIndex &parent)
{
    if (parent.isValid()) return;
    d->fetchMore(true);
}

bool ResultModel::canFetchMore(const QModelIndex &parent) const
{
    return parent.isValid()                     ? false
         : d->cache.size() >= d->itemCountLimit ? false
         : !d->resultIt.isSourceValid()         ? true
         : d->resultIt != d->results.end()
         ;
}

void ResultModel::setItemCountLimit(int count)
{
    d->itemCountLimit = count;
    // TODO: ...
}

int ResultModel::itemCountLimit() const
{
    return d->itemCountLimit;
}


} // namespace Stats
} // namespace Experimental
} // namespace KActivities

// #include "resourcemodel.moc"

