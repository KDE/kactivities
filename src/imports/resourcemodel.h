/*
    SPDX-FileCopyrightText: 2012, 2013, 2014 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KACTIVITIES_IMPORTS_RESOURCE_MODEL_H
#define KACTIVITIES_IMPORTS_RESOURCE_MODEL_H

// Qt
#include <QJSValue>
#include <QObject>
#include <QSortFilterProxyModel>
#include <QSqlDatabase>
#include <QSqlTableModel>

// KDE
#include <KConfigGroup>

// STL and Boost
#include <memory>

// Local
#include <lib/consumer.h>
#include <lib/controller.h>
#include <lib/info.h>

class QModelIndex;
class QDBusPendingCallWatcher;

namespace KActivities
{
namespace Imports
{
/**
 * ResourceModel
 */

class ResourceModel : public QSortFilterProxyModel
{
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

    /**
     * If the model is empty, use this config file to read the default items.
     * The default items are automatically linked globally, not per-activity.
     * It needs to have the following format: 'config-namerc/ConfigGroup/ConfigEntry'.
     * The config entry needs to be a list of strings.
     */
    Q_PROPERTY(QString defaultItemsConfig READ defaultItemsConfig WRITE setDefaultItemsConfig)

public:
    explicit ResourceModel(QObject *parent = nullptr);
    ~ResourceModel() override;

    enum Roles {
        ResourceRole = Qt::UserRole,
        ActivityRole = Qt::UserRole + 1,
        AgentRole = Qt::UserRole + 2,
        DescriptionRole = Qt::UserRole + 3,
    };

    QHash<int, QByteArray> roleNames() const override;

    virtual QVariant data(const QModelIndex &proxyIndex, int role = Qt::DisplayRole) const override;

public Q_SLOTS:
    // Resource linking control methods
    void linkResourceToActivity(const QString &resource, const QJSValue &callback) const;
    void linkResourceToActivity(const QString &resource, const QString &activity, const QJSValue &callback) const;
    void linkResourceToActivity(const QString &agent, const QString &resource, const QString &activity, const QJSValue &callback) const;

    void unlinkResourceFromActivity(const QString &resource, const QJSValue &callback);
    void unlinkResourceFromActivity(const QString &resource, const QString &activity, const QJSValue &callback);
    void unlinkResourceFromActivity(const QString &agent, const QString &resource, const QString &activity, const QJSValue &callback);
    void unlinkResourceFromActivity(const QStringList &agents, const QString &resource, const QStringList &activities, const QJSValue &callback);

    bool isResourceLinkedToActivity(const QString &resource);
    bool isResourceLinkedToActivity(const QString &resource, const QString &activity);
    bool isResourceLinkedToActivity(const QString &agent, const QString &resource, const QString &activity);
    bool isResourceLinkedToActivity(const QStringList &agents, const QString &resource, const QStringList &activities);

    // Model property getters and setters
    void setShownActivities(const QString &activities);
    QString shownActivities() const;

    void setShownAgents(const QString &agents);
    QString shownAgents() const;

    QString defaultItemsConfig() const;
    void setDefaultItemsConfig(const QString &defaultItemsConfig);

    void setOrder(const QStringList &resources);
    void move(int sourceItem, int destinationItem);
    void sortItems(Qt::SortOrder sortOrder);

    KConfigGroup config() const;

    int count() const;
    QString displayAt(int row) const;
    QString resourceAt(int row) const;

protected:
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;

Q_SIGNALS:
    void shownActivitiesChanged();
    void shownAgentsChanged();

private Q_SLOTS:
    void onCurrentActivityChanged(const QString &activity);

    void onResourceLinkedToActivity(const QString &initiatingAgent, const QString &targettedResource, const QString &usedActivity);
    void onResourceUnlinkedFromActivity(const QString &initiatingAgent, const QString &targettedResource, const QString &usedActivity);

private:
    KActivities::Consumer m_service;

    inline QVariant dataForColumn(const QModelIndex &index, int column) const;

    QString activityToWhereClause(const QString &activity) const;
    QString agentToWhereClause(const QString &agent) const;
    QString whereClause(const QStringList &activities, const QStringList &agents) const;

    void loadDefaultsIfNeeded() const;

    bool loadDatabase();
    QString m_databaseFile;
    QSqlDatabase m_database;
    QSqlTableModel *m_databaseModel;

    QStringList m_shownActivities;
    QStringList m_shownAgents;
    QStringList m_sorting;

    QString m_defaultItemsConfig;
    mutable bool m_defaultItemsLoaded;

    void reloadData();
    QString validateResource(const QString &resource) const;

    class LinkerService;
    std::shared_ptr<LinkerService> m_linker;

    mutable KConfigGroup m_config;
};

} // namespace Imports
} // namespace KActivities

#endif // KACTIVITIES_IMPORTS_RESOURCE_MODEL_H
