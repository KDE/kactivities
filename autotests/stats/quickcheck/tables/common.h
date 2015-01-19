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

#ifndef QUICKCHECK_DATABASE_COMMON_H
#define QUICKCHECK_DATABASE_COMMON_H

struct PrimaryKeyOrdering {
    template <typename T>
    bool operator() (const T &left,
                     const T &right) const
    {
        return left.primaryKey() < right.primaryKey();
    }
};

#define TABLE(Table) std::set<Table::Item, PrimaryKeyOrdering>

#define DECL_COLUMN(ColumnType, ColumnName)                                    \
    inline Column<Item, ColumnType> ColumnName()                               \
    {                                                                          \
        return Column<Item, ColumnType>(&Item::ColumnName);                    \
    }

template <typename Type, typename ColumnType>
class Column {
    typedef Column<Type, ColumnType> ThisType;

public:
    Column(const ColumnType Type::* memberptr)
        : memberptr(memberptr)
    {
    }

    // Column comparator functor {{{
    class Comparator {
    public:
        Comparator(const ColumnType Type::* memberptr, bool invert)
            : memberptr(memberptr)
            , invert(invert)
        {
        }

        inline bool operator()(const Type &left, const Type &right) const
        {
            return (invert) ? right.*memberptr < left.*memberptr
                            : left.*memberptr < right.*memberptr;
        }

    private:
        const ColumnType Type::* memberptr;
        const bool invert;
    };
    // }}}

    inline Comparator asc() const
    {
        return Comparator(memberptr, false);
    }

    inline Comparator desc() const
    {
        return Comparator(memberptr, true);
    }

    // Column filtering functor {{{
    enum ComparisonOperation {
        Less,
        LessOrEqual,
        Equal,
        GreaterOrEqual,
        Greater
    };

    template <typename T>
    class Filterer {
    public:
        Filterer(const ColumnType Type::*memberptr,
                 ComparisonOperation comparison,
                 const T &value)
            : memberptr(memberptr)
            , comparison(comparison)
            , value(value)
        {
        }

        inline bool operator()(const Type &item) const
        {
            return
                comparison == Less           ? item.*memberptr <  value :
                comparison == LessOrEqual    ? item.*memberptr <= value :
                comparison == Equal          ? item.*memberptr == value :
                comparison == GreaterOrEqual ? item.*memberptr >= value :
                comparison == Greater        ? item.*memberptr >  value :
                                                                  false;
        }

    private:
        const ColumnType Type::* memberptr;
        const ComparisonOperation comparison;
        const T value;
    };
    // }}}

    template <typename T>
    inline Filterer<T> operator <(const T &value) const
    {
        return Filterer<T>(memberptr, Less, value);
    }

    template <typename T>
    inline Filterer<T> operator <=(const T &value) const
    {
        return Filterer<T>(memberptr, LessOrEqual, value);
    }

    template <typename T>
    inline Filterer<T> operator ==(const T &value) const
    {
        return Filterer<T>(memberptr, Equal, value);
    }

    template <typename T>
    inline Filterer<T> operator >=(const T &value) const
    {
        return Filterer<T>(memberptr, GreaterOrEqual, value);
    }

    template <typename T>
    inline Filterer<T> operator >(const T &value) const
    {
        return Filterer<T>(memberptr, Greater, value);
    }

    // Column stuff

private:
    const ColumnType Type::* memberptr;
};

#endif // QUICKCHECK_DATABASE_COMMON_H

// vim: set foldmethod=marker:
