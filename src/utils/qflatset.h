/*
 *   Copyright (C) 2016 by Ivan Čukić <ivan.cukic(at)kde.org>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) version 3, or any
 *   later version accepted by the membership of KDE e.V. (or its
 *   successor approved by the membership of KDE e.V.), which shall
 *   act as a proxy defined in Section 6 of version 3 of the license.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library.
 *   If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef KACTIVITIES_STATS_QFLATSET_H
#define KACTIVITIES_STATS_QFLATSET_H

#include <QVector>
#include <QPair>
#include <QDebug>

namespace KActivities {

template <typename T, typename LessThan>
class QFlatSet: public QVector<T> {
public:
    QFlatSet()
    {
    }

    inline
    // QPair<typename QVector<T>::iterator, bool> insert(const T &value)
    std::tuple<typename QVector<T>::iterator, int, bool> insert(const T &value)
    {
        auto lessThan = LessThan();
        auto begin    = this->begin();
        auto end      = this->end();

        if (begin == end) {
            QVector<T>::insert(0, value);

            return std::make_tuple(QVector<T>::begin(), 0, true);

        } else {
            auto iterator = std::lower_bound(begin, end, value, lessThan);

            if (iterator != end) {
                if (!lessThan(value, *iterator)) {
                    // Already present
                    return std::make_tuple(iterator, iterator - begin, false);
                }
            }

            QVector<T>::insert(iterator, value);

            return std::make_tuple(iterator, iterator - begin, true);
        }
    }

private:
    QFlatSet(const QFlatSet &original); // = delete

};


} // namespace KActivities

#endif // KACTIVITIES_STATS_QFLATSET_H

