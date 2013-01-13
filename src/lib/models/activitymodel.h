/*
 *   Copyright (C) 2012 Ivan Cukic <ivan.cukic(at)kde.org>
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

#ifndef KACTIVITIES_MODELS_ACTIVITY_MODEL_H
#define KACTIVITIES_MODELS_ACTIVITY_MODEL_H

#include <QAbstractListModel>

#include "kactivities_models_export.h"

class QModelIndex;

namespace KActivities {
namespace Models {

/**
 * ActivityModel
 */

class KACTIVITIES_MODELS_EXPORT ActivityModel: public QAbstractListModel {
    Q_OBJECT

public:
    ActivityModel(QObject * parent = 0);
    virtual ~ActivityModel();

    virtual int rowCount(const QModelIndex & parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    enum Roles {
        ActivityId = Qt::UserRole,
        ActivityState = Qt::UserRole + 1
    };

    enum State {
        Invalid  = 0,
        Running  = 2,
        Starting = 3,
        Stopped  = 4,
        Stopping = 5
    };


private:
    Q_PRIVATE_SLOT(d, void listActivitiesCallFinished(QDBusPendingCallWatcher*))
    Q_PRIVATE_SLOT(d, void activityInfoCallFinished(QDBusPendingCallWatcher*))

    Q_PRIVATE_SLOT(d, void activityNameChanged(const QString &, const QString &))
    Q_PRIVATE_SLOT(d, void activityIconChanged(const QString &, const QString &))
    Q_PRIVATE_SLOT(d, void activityStateChanged(const QString &, int))

    Q_PRIVATE_SLOT(d, void activityAdded(const QString &))
    Q_PRIVATE_SLOT(d, void activityRemoved(const QString &))

    Q_PRIVATE_SLOT(d, void servicePresenceChanged(bool))

    friend class Private;

    class Private;
    Private * const d;
};

} // namespace Models
} // namespace KActivities

#endif // KACTIVITIES_MODELS_ACTIVITY_MODEL_H

