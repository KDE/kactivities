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

namespace KActivities {
namespace Experimental {
namespace Stats {

using namespace Terms;

typedef ResultSet::const_iterator iterator;

// Iterator

class iterator::Private {
public:
    Private(const ResultSet *resultSet, int currentRow = -1)
        : resultSet(resultSet)
        , currentRow(currentRow)
    {
        updateValue();
    }

    const ResultSet *resultSet;
    int currentRow;
    boost::optional<Result> currentValue;

    inline void moveTo(int row)
    {
        if (row == currentRow) return;
        currentRow = row;
        updateValue();
    }

    inline void moveBy(int row)
    {
        moveTo(currentRow + row);
    }

    void updateValue()
    {
        if (!resultSet || !resultSet->d->query.seek(currentRow)) {
            currentValue = boost::optional<Result>();

        } else {
            currentValue = boost::make_optional(std::move(resultSet->d->currentResult()));

        }
    }

    friend void swap(Private &left, Private &right)
    {
        using namespace std;
        swap(left.resultSet,    right.resultSet);
        swap(left.currentRow,   right.currentRow);
        swap(left.currentValue, right.currentValue);
    }

    bool operator==(const Private &other) const
    {
        bool thisValid  = currentValue;
        bool otherValid = other.currentValue;

        return
            // If one is valid, and the other is not,
            // they are not equal
            thisValid != otherValid ? false :

            // If both are invalid, they are equal
            !thisValid              ? true  :

            // Otherwise, really compare
            resultSet  == other.resultSet &&
                currentRow == other.currentRow;
    }

    bool isValid() const
    {
        return currentValue;
    }

    static bool sameSource(const Private &left, const Private &right)
    {
        return left.resultSet == right.resultSet &&
               left.resultSet != Q_NULLPTR;
    }

};

iterator::const_iterator(const ResultSet *resultSet, int currentRow)
    : d(new Private(resultSet, currentRow))
{
}

iterator::const_iterator()
    : d(new Private(Q_NULLPTR, -1))
{
}

iterator::const_iterator(const const_iterator &source)
    : d(new Private(source.d->resultSet, source.d->currentRow))
{
}

bool iterator::isSourceValid() const
{
    return d->resultSet != Q_NULLPTR;
}

iterator &iterator::operator=(const const_iterator &source)
{
    const_iterator temp(source);
    swap(*d, *temp.d);
    return *this;
}

iterator::~const_iterator()
{
    delete d;
}

iterator::reference iterator::operator*() const
{
    return d->currentValue.get();
}

iterator::pointer iterator::operator->() const
{
    return &d->currentValue.get();
}

// prefix
iterator& iterator::operator++()
{
    d->currentRow++;
    d->updateValue();

    return *this;
}

// postfix
iterator iterator::operator++(int)
{
    return const_iterator(d->resultSet, d->currentRow + 1);
}

// prefix
iterator& iterator::operator--()
{
    d->currentRow--;
    d->updateValue();

    return *this;
}

// postfix
iterator iterator::operator--(int)
{
    return const_iterator(d->resultSet, d->currentRow - 1);
}

iterator ResultSet::begin() const
{
    return const_iterator(this, 0);
}

iterator ResultSet::end() const
{
    return const_iterator(this, -1);
}

iterator iterator::operator+(iterator::difference_type n) const
{
    return const_iterator(d->resultSet, d->currentRow + n);
}

iterator &iterator::operator+=(iterator::difference_type n)
{
    d->moveBy(n);
    return *this;
}

iterator iterator::operator-(iterator::difference_type n) const
{
    return const_iterator(d->resultSet, d->currentRow - n);
}

iterator &iterator::operator-=(iterator::difference_type n)
{
    d->moveBy(-n);
    return *this;
}

iterator::reference iterator::operator[](iterator::difference_type n) const
{
    return *(*this + n);
}

// bool iterator::operator==(const const_iterator &right) const
// {
//     return *d == *right.d;
// }
//
// bool iterator::operator!=(const const_iterator &right) const
// {
//     return !(*d == *right.d);
// }

bool operator==(const iterator &left, const iterator &right)
{
    return *left.d == *right.d;
}

bool operator!=(const iterator &left, const iterator &right)
{
    return !(*left.d == *right.d);
}

#define COMPARATOR_IMPL(OP)                                                    \
    bool operator OP(const iterator &left, const iterator &right)              \
    {                                                                          \
        return iterator::Private::sameSource(*left.d, *right.d)                \
                   ? left.d->currentRow OP right.d->currentRow                 \
                   : false;                                                    \
    }

COMPARATOR_IMPL(<)
COMPARATOR_IMPL(>)
COMPARATOR_IMPL(<=)
COMPARATOR_IMPL(>=)

#undef COMPARATOR_IMPL

iterator::difference_type operator-(const iterator &left, const iterator &right)
{
    return iterator::Private::sameSource(*left.d, *right.d)
               ? left.d->currentRow - right.d->currentRow
               : 0;
}

} // namespace Stats
} // namespace Experimental
} // namespace KActivities

