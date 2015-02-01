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

#ifndef KACTIVITIES_STATS_RESULTSET
#define KACTIVITIES_STATS_RESULTSET

#include <query.h>

namespace KActivities {
namespace Experimental {
namespace Stats {

/**
 * Class that can query the KActivities usage tracking mechanism
 * for resources.
 */
class KACTIVITIESSTATS_EXPORT ResultSet {
public:
    /**
     * Structure containing data of one of the results
     */
    struct Result {
        QString resource; ///< URL of the resource
        QString title;    ///< Title of the resource, or URL if title is not known
        double score;     ///< The score calculated based on the usage statistics
    };

    /**
     * ResultSet is a container. This notifies the generic algorithms
     * from STLboost, and others of the contained type.
     */
    typedef Result value_type;

    /**
     * Creates the ResultSet from the specified query
     */
    ResultSet(Query query);
    ~ResultSet();

    /**
     * @returns a result at the specified index
     * @param index of the result
     * @note You should use iterators instead
     */
    Result at(int index) const;

    // Iterators

    /**
     * An STL-style constant forward iterator for accessing the results in a ResultSet
     * TODO: Consider making this to be more than just forward iterator.
     *       Maybe even a random-access one.
     */
    class const_iterator {
    public:
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

        friend class ResultSet;

        class Private;
        Private* const d;
    };

    /**
     * @returns a constant iterator pointing to the start of the collection
     * (to the first item)
     * @note as usual in C++, the range of the collection is [begin, end)
     */
    const_iterator begin() const;
    /**
     * @returns a constant iterator pointing to the end of the collection
     * (after the last item)
     * @note as usual in C++, the range of the collection is [begin, end)
     */
    const_iterator end() const;

    /**
     * Alias for begin
     */
    inline const_iterator cbegin() const { return begin(); }
    /**
     * Alias for end
     */
    inline const_iterator cend() const   { return end(); }

    /**
     * Alias for begin
     */
    inline const_iterator constBegin() const { return cbegin(); }
    /**
     * Alias for end
     */
    inline const_iterator constEnd() const   { return cend(); }

private:
    class Private;
    Private* const d;
};

} // namespace Stats
} // namespace Experimental
} // namespace KActivities

#endif // KACTIVITIES_STATS_RESULTSET

