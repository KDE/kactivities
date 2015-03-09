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

// STL and Boost
#include <functional>
#include <boost/range/algorithm/find_if.hpp>
#include <boost/range/algorithm/lower_bound.hpp>

// Local
#include "resultset.h"
#include "resultwatcher.h"
#include "consumer.h"

#include <utils/member_matcher.h>
#include <utils/slide.h>
#include <common/specialvalues.h>

#define CHUNK_SIZE 10
#define DEFAULT_ITEM_COUNT_LIMIT 20

namespace KActivities {
namespace Experimental {
namespace Stats {

class ResultModel::Private {
public:
    Private(Query query, ResultModel *parent)
        : query(query)
        , results(query)
        , watcher(query)
        , itemCountLimit(DEFAULT_ITEM_COUNT_LIMIT)
        , q(parent)
    {
    }

    //_ findResource(...) -> (index, iterator)

    struct FindResult {
        FindResult(Private * const d, QList<ResultSet::Result>::iterator iterator)
            : d(d)
            , iterator(iterator)
            , index(iterator == d->cache.end() ? -1 : iterator - d->cache.begin())
        {
        }

        Private * const d;
        QList<ResultSet::Result>::iterator iterator;
        int index;

        operator bool() const
        {
            return iterator != d->cache.end();
        }
    };

    inline FindResult findResource(const QString &resource)
    {
        using namespace kamd::utils::member_matcher;
        using boost::find_if;

        return FindResult(
            this,
            find_if(cache, member(&ResultSet::Result::resource) == resource));
    }

    //^

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

    void onResultAdded(const QString &resource, double score)
    {
        using namespace kamd::utils::member_matcher;
        using kamd::utils::slide_one;
        using boost::lower_bound;

        // This can also be called when the resource score
        // has been updated, so we need to check whether
        // we already have it in the cache
        const auto result = findResource(resource);

        // TODO: We should also sort by the resource, not only on score
        const auto destination = lower_bound(
            cache, score, member(&ResultSet::Result::score) > _);

        const int destinationIndex = destination - cache.begin();

        if (result) {
            // We already have the resource in the cache
            // So, it is the time for a reshuffle
            const int currentIndex = result.index;

            q->beginMoveRows(QModelIndex(), currentIndex, currentIndex,
                             QModelIndex(), destinationIndex);

            slide_one(result.iterator, destination);

            q->endMoveRows();

        } else {
            // We do not have the resource in the cache

            q->beginInsertRows(QModelIndex(), destinationIndex,
                               destinationIndex + 1);

            ResultSet::Result result;
            result.resource = resource;
            result.score = score;

            // TODO: Add the resource title, if known
            // result.title;

            cache.insert(destinationIndex, result);

            q->endInsertRows();
        }
    }

    void onResultRemoved(const QString &resource)
    {
        const auto result = findResource(resource);

        if (!result) return;

        q->beginRemoveRows(QModelIndex(), result.index, result.index);

        cache.removeAt(result.index);

        q->endRemoveRows();
    }

    void onResourceTitleChanged(const QString &resource, const QString &title)
    {
        const auto result = findResource(resource);

        if (!result) return;

        result.iterator->title = title;

        q->dataChanged(q->index(result.index), q->index(result.index));
    }

    void onResourceMimetypeChanged(const QString &resource, const QString &mimetype)
    {
        // TODO: This can add or remove items from the model
    }

    void reset()
    {
        q->beginResetModel();

        results = ResultSet(query);
        cache.clear();

        init();

        q->endResetModel();
    }

    void init()
    {
        fetchMore(false);
    }

    void onCurrentActivityChanged(const QString &activity)
    {
        reset();
    }

    Query query;
    ResultSet results;
    ResultWatcher watcher;

    int itemCountLimit;

    ResultSet::const_iterator resultIt;
    QList<ResultSet::Result> cache;
    KActivities::Consumer activities;


private:
    ResultModel *const q;

};

ResultModel::ResultModel(Query query, QObject *parent)
    : d(new Private(query, this))
{
    using namespace std::placeholders;

    connect(&d->watcher, &ResultWatcher::resultAdded,
            this, std::bind(&Private::onResultAdded, d, _1, _2));
    connect(&d->watcher, &ResultWatcher::resultRemoved,
            this, std::bind(&Private::onResultRemoved, d, _1));

    connect(&d->watcher, &ResultWatcher::resourceTitleChanged,
            this, std::bind(&Private::onResourceTitleChanged, d, _1, _2));
    connect(&d->watcher, &ResultWatcher::resourceMimetypeChanged,
            this, std::bind(&Private::onResourceMimetypeChanged, d, _1, _2));

    connect(&d->watcher, &ResultWatcher::resultsInvalidated,
            this, std::bind(&Private::reset, d));

    if (query.activities().contains(CURRENT_ACTIVITY_TAG)) {
        connect(&d->activities, &KActivities::Consumer::currentActivityChanged,
                this, std::bind(&Private::onCurrentActivityChanged, d, _1));
    }

    d->init();
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

