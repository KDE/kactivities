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

// KDE
#include <kconfiggroup.h>

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
     * Sets for which activities should the resources be shown for.
     * Coma-separated values.
     * Special values are:
     *  - ":current" for the current activity
     *  - ":any" show resources that are linked to any activity, including "global"
     *  - ":global" show resources that are globally linked
     */
    Q_PROPERTY(QString shownActivities READ shownActivities WRITE setShownActivities NOTIFY shownActivitiesChanged)

    /**
     * Sets for which agents should the resources be shown for.
     * Coma-separated values.
     * Special values are:
     *  - ":current" for the current application
     *  - ":any" show resources that are linked to any agent, including "global"
     *  - ":global" show resources that are globally linked
     */
    Q_PROPERTY(QString shownAgents READ shownAgents WRITE setShownAgents NOTIFY shownAgentsChanged)

public:
    ResourceModel(QObject *parent = 0);
    virtual ~ResourceModel();

    enum Roles {
        ResourceRole = Qt::UserRole,
        ActivityRole = Qt::UserRole + 1,
        AgentRole    = Qt::UserRole + 2
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
    void setShownActivities(const QString &activities);
    QString shownActivities() const;

    void setShownAgents(const QString &agents);
    QString shownAgents() const;

    void setOrder(const QStringList &resources);

    KConfigGroup config() const;

    int count() const;
    QString displayAt(int row) const;
    QString resourceAt(int row) const;

protected:
    virtual bool lessThan(const QModelIndex &left, const QModelIndex &right) const Q_DECL_OVERRIDE;


Q_SIGNALS:
    void shownActivitiesChanged();
    void shownAgentsChanged();

private Q_SLOTS:
    void setCurrentActivity(const QString &activity);

    void resourceLinkedToActivity(const QString &initiatingAgent,
                                  const QString &targettedResource,
                                  const QString &usedActivity);
    void resourceUnlinkedFromActivity(const QString &initiatingAgent,
                                      const QString &targettedResource,
                                      const QString &usedActivity);


private:
    KActivities::Consumer m_service;

    inline
    QVariant dataForColumn(const QModelIndex &index, int column) const;

    QString activityToWhereClause(const QString &activity) const;
    QString agentToWhereClause(const QString &activity) const;

    QSqlDatabase m_database;
    QSqlTableModel *m_databaseModel;

    QStringList m_shownActivities;
    QStringList m_shownAgents;
    QStringList m_sorting;

    void reloadData();

    class LinkerService;
    std::shared_ptr<LinkerService> m_linker;

    KConfigGroup m_config;
};

} // namespace Models
} // namespace KActivities

#endif // KACTIVITIES_MODELS_RESOURCE_MODEL_H
