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

#ifndef KACTIVITIES_STATS_RESULT
#define KACTIVITIES_STATS_RESULT

#include <query.h>

namespace KActivities {
namespace Experimental {
namespace Stats {

class KACTIVITIESSTATS_EXPORT ResultSet {
public:
    struct Result {
        QString uri;
        double score;
    };

    typedef Result value_type;

    ResultSet(Query query);
    ~ResultSet();

    Result at(int index) const;

    // TODO: REMOVE!!!
    QStringList _results_() const;

    // Iterators

    class const_iterator {
    public:
        // TODO: Consider making this to be more than just forward iterator.
        //       Maybe even a random-access one.
        typedef std::forward_iterator_tag iterator_category;
        typedef int difference_type;

        typedef const Result value_type;
        typedef const Result& reference;
        typedef const Result* pointer;

        const_iterator();
        const_iterator(const const_iterator &source);
        const_iterator &operator=(const const_iterator &source);

        ~const_iterator();

        bool operator==(const const_iterator &other) const;
        bool operator!=(const const_iterator &other) const;

        const Result& operator*() const;
        const Result* operator->() const;

        // prefix
        const_iterator& operator++();
        // postfix
        const_iterator operator++(int);

    private:
        const_iterator(const ResultSet *resultSet, int currentRow);
        // const_iterator();

        friend class ResultSet;

        class Private;
        Private* const d;
    };

    const_iterator begin() const;
    const_iterator end() const;

    inline const_iterator cbegin() const { return begin(); }
    inline const_iterator cend() const   { return end(); }

    inline const_iterator constBegin() const { return cbegin(); }
    inline const_iterator constEnd() const   { return cend(); }

private:
    class Private;
    Private* const d;
};

} // namespace Stats
} // namespace Experimental
} // namespace KActivities

#endif // KACTIVITIES_STATS_RESULT

