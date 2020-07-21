/*
    SPDX-FileCopyrightText: 2016 Ivan Čukić <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef KACTIVITIES_STATS_QFLATSET_H
#define KACTIVITIES_STATS_QFLATSET_H

#include <QVector>
#include <QPair>

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

