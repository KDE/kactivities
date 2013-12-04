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

// Qt
#include <QObject>
#include <QAbstractListModel>

// STL and Boost
#include <boost/container/flat_set.hpp>
#include <memory>

// Local
#include <lib/core/consumer.h>
#include <lib/core/info.h>

class QModelIndex;
class QDBusPendingCallWatcher;

namespace KActivities {
namespace Models {

/**
 * ActivityModel
 */

class ActivityModel : public QAbstractListModel {
    Q_OBJECT

public:
    ActivityModel(QObject *parent = 0);
    virtual ~ActivityModel();

    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;

    QHash<int, QByteArray> roleNames() const Q_DECL_OVERRIDE;

    enum Roles {
        ActivityId = Qt::UserRole,
        ActivityState = Qt::UserRole + 1
    };

    enum State {
        Invalid = 0,
        Running = 2,
        Starting = 3,
        Stopped = 4,
        Stopping = 5
    };

private Q_SLOTS:
    void setActivityName(const QString &name);
    void setActivityIcon(const QString &icon);
    void setActivityState(KActivities::Info::State state);

    void replaceActivities(const QStringList &activities);
    void addActivity(const QString &id);
    void removeActivity(const QString &id);

    void setServiceStatus(KActivities::Consumer::ServiceStatus status);

private:
    KActivities::Consumer m_consumer;

    typedef std::unique_ptr<Info> InfoPtr;

    struct InfoPtrComparator {
        bool operator() (const InfoPtr& left, const InfoPtr& right) const
        {
            return left->name().toLower() < right->name().toLower();
        }
    };

    boost::container::flat_set<InfoPtr, InfoPtrComparator> m_activities;

    unsigned int addActivitySilently(const QString &id);

    class Private;
    friend class Private;
};

} // namespace Models
} // namespace KActivities

#endif // KACTIVITIES_MODELS_ACTIVITY_MODEL_H
