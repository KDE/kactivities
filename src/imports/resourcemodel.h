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

#ifndef KACTIVITIES_MODELS_RESOURCE_MODEL_H
#define KACTIVITIES_MODELS_RESOURCE_MODEL_H

// Qt
#include <QObject>
#include <QSortFilterProxyModel>
#include <QJSValue>
#include <QSqlTableModel>
#include <QSqlDatabase>

// STL and Boost
#include <boost/container/flat_set.hpp>
#include <memory>

// Local
#include <lib/core/controller.h>
#include <lib/core/consumer.h>
#include <lib/core/info.h>

class QModelIndex;
class QDBusPendingCallWatcher;

namespace KActivities {
namespace Models {

/**
 * ResourceModel
 */

class ResourceModel : public QSortFilterProxyModel {
    Q_OBJECT

    /**
     * Sets for which activity should the resources be shown for.
     * Special values are:
     *  - ":current" for the current activity
     *  - ":any" show resources that are linked to any activity, including "global"
     *  - ":global" show resources that are globally linked
     */
    Q_PROPERTY(QString shownActivity READ shownActivity WRITE setShownActivity NOTIFY shownActivityChanged)

    /**
     * Sets for which agent should the resources be shown for.
     * Special values are:
     *  - ":current" for the current application
     *  - ":any" show resources that are linked to any agent, including "global"
     *  - ":global" show resources that are globally linked
     */
    Q_PROPERTY(QString shownAgent READ shownAgent WRITE setShownAgent NOTIFY shownAgentChanged)

public:
    ResourceModel(QObject *parent = 0);
    virtual ~ResourceModel();

    enum Roles {
        Resource    = Qt::UserRole,
        Activity    = Qt::UserRole + 1,
        Agent       = Qt::UserRole + 2
    };

    QHash<int, QByteArray> roleNames() const Q_DECL_OVERRIDE;

    virtual QVariant data(const QModelIndex &proxyIndex,
                          int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;

public Q_SLOTS:
    // Resource linking control methods
    void linkResourceToActivity(const QString &resource,
                                const QJSValue &callback);
    void linkResourceToActivity(const QString &resource,
                                const QString &activity,
                                const QJSValue &callback);
    void linkResourceToActivity(const QString &agent,
                                const QString &resource,
                                const QString &activity,
                                const QJSValue &callback);

    void unlinkResourceFromActivity(const QString &resource,
                                    const QJSValue &callback);
    void unlinkResourceFromActivity(const QString &resource,
                                    const QString &activity,
                                    const QJSValue &callback);
    void unlinkResourceFromActivity(const QString &agent,
                                    const QString &resource,
                                    const QString &activity,
                                    const QJSValue &callback);

    // Model property getters and setters
    void setShownActivity(const QString &activityId);
    QString shownActivity() const;

    void setShownAgent(const QString &agent);
    QString shownAgent() const;


Q_SIGNALS:
    void shownActivityChanged();
    void shownAgentChanged();

private Q_SLOTS:
    void setCurrentActivity(const QString &activity);


private:
    KActivities::Consumer m_service;

    QSqlDatabase   m_database;
    QSqlTableModel *m_databaseModel;

    QString m_shownActivity;
    QString m_shownAgent;

    void reloadData();
};

} // namespace Models
} // namespace KActivities

#endif // KACTIVITIES_MODELS_RESOURCE_MODEL_H
