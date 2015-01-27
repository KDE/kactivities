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

// Iterator

class ResultSet::const_iterator::Private {
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

    void updateValue()
    {
        if (!resultSet || !resultSet->d->query.seek(currentRow)) {
            currentValue = boost::optional<Result>();

        } else {
            Result result;
            result.resource = resultSet->d->query.value("resource").toString();
            result.title    = resultSet->d->query.value("title").toString();
            result.score    = resultSet->d->query.value("score").toDouble();
            currentValue    = boost::make_optional(result);

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
};

ResultSet::const_iterator::const_iterator(const ResultSet *resultSet, int currentRow)
    : d(new Private(resultSet, currentRow))
{
}

ResultSet::const_iterator::const_iterator()
    : d(new Private(Q_NULLPTR, -1))
{
}

ResultSet::const_iterator::const_iterator(const const_iterator &source)
    : d(new Private(source.d->resultSet, source.d->currentRow))
{
}

ResultSet::const_iterator &ResultSet::const_iterator::operator=(const const_iterator &source)
{
    const_iterator result(source);
    swap(*source.d, *result.d);
    return *this;
}

bool ResultSet::const_iterator::operator==(const const_iterator &right) const
{
    return *d == *right.d;
}

bool ResultSet::const_iterator::operator!=(const const_iterator &right) const
{
    return !(*d == *right.d);
}

ResultSet::const_iterator::~const_iterator()
{
    delete d;
}

const ResultSet::Result& ResultSet::const_iterator::operator*() const
{
    return d->currentValue.get();
}

const ResultSet::Result* ResultSet::const_iterator::operator->() const
{
    return &d->currentValue.get();
}


// prefix
ResultSet::const_iterator& ResultSet::const_iterator::operator++()
{
    d->currentRow++;
    d->updateValue();

    return *this;
}

// postfix
ResultSet::const_iterator ResultSet::const_iterator::operator++(int)
{
    return const_iterator(d->resultSet, d->currentRow + 1);
}

ResultSet::const_iterator ResultSet::begin() const
{
    return const_iterator(this, 0);
}

ResultSet::const_iterator ResultSet::end() const
{
    return const_iterator(this, -1);
}

} // namespace Stats
} // namespace Experimental
} // namespace KActivities

