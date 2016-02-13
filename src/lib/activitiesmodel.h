/*
 *   Copyright (C) 2012, 2013, 2014 Ivan Cukic <ivan.cukic(at)kde.org>
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

#ifndef ACTIVITIES_ACTIVITIESMODEL_H
#define ACTIVITIES_ACTIVITIESMODEL_H

// Qt
#include <QObject>
#include <QAbstractListModel>

// STL and Boost
#include <boost/container/flat_set.hpp>
#include <memory>

// Local
#include <lib/info.h>

class QModelIndex;
class QDBusPendingCallWatcher;

namespace KActivities {

class ActivitiesModelPrivate;

/**
 * Data model that shows existing activities
 */
class KACTIVITIES_EXPORT ActivitiesModel : public QAbstractListModel {
    Q_OBJECT

    Q_PROPERTY(QVector<Info::State> shownStates READ shownStates WRITE setShownStates NOTIFY shownStatesChanged)

public:
    ActivitiesModel(QObject *parent = 0);
    virtual ~ActivitiesModel();

    int rowCount(const QModelIndex &parent = QModelIndex()) const
        Q_DECL_OVERRIDE;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const
        Q_DECL_OVERRIDE;

    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;

    QHash<int, QByteArray> roleNames() const Q_DECL_OVERRIDE;

    enum Roles {
        ActivityId          = Qt::UserRole,
        ActivityDescription = Qt::UserRole + 1,
        ActivityIcon        = Qt::UserRole + 2,
        ActivityState       = Qt::UserRole + 3,
        ActivityBackground  = Qt::UserRole + 4,
        ActivityIsCurrent   = Qt::UserRole + 5,
    };

public Q_SLOTS:
    // Model property getters and setters
    void setShownStates(const QVector<Info::State> &states);
    QVector<Info::State> shownStates() const;

Q_SIGNALS:
    void shownStatesChanged(const QVector<Info::State> &state);

private:
    friend class ActivitiesModelPrivate;
    ActivitiesModelPrivate * const d;
};

} // namespace KActivities

#endif // ACTIVITIES_ACTIVITIESMODEL_H

