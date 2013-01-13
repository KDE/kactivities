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

#ifndef KACTIVITIES_MODELS_RESOURCE_MODEL_H
#define KACTIVITIES_MODELS_RESOURCE_MODEL_H

#include <QAbstractListModel>

#include "kactivities_models_export.h"

class QModelIndex;

namespace KActivities {
namespace Models {

/**
 * ResourceModel
 */

class KACTIVITIES_MODELS_EXPORT ResourceModel: public QAbstractListModel {
    Q_OBJECT

    Q_PROPERTY(QString activity READ activity WRITE setActivity NOTIFY activityChanged)
    Q_PROPERTY(QString application READ application WRITE setApplication NOTIFY applicationChanged)
    Q_PROPERTY(int limit READ limit WRITE setLimit NOTIFY limitChanged)

public:
    ResourceModel(QObject * parent = 0);
    virtual ~ResourceModel();

    /**
     * What should the model display?
     */
    enum ContentMode {
        Favorites, // Show linked resources first, then the top rated (default)
        Linked,    // Show only linked resources
        TopRated,  // Show only top rated resources
        Recent     // Show recently used resources
    };

    enum Roles {
        ResourceUrl = Qt::UserRole,
        ResourceScore,
        ResourceIconName
    };

    virtual int rowCount(const QModelIndex & parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

public Q_SLOTS:
    /**
     * Shows resources related to specific activity
     * @param activity activity id.
     * @note if the activity id is empty, the model
     * will display resources linked to the current activity
     * (default)
     */
    void setActivity(const QString & activity);

    /**
     * @returns which activity the model displays
     */
    QString activity() const;

    /**
     * Shows resources related to a specific application
     * @param application application id
     * @note if empty, shows resources for all applications
     * aggregated (default)
     */
    void setApplication(const QString & application);

    /**
     * @returns for which application the resources are shown
     */
    QString application() const;

    /**
     * Limit the number of items to show
     * @param count number of items to show
     * @note default is 10, increasing it significantely
     * can lead to performance degradation
     */
    void setLimit(int count);

    /**
     * @returns the item count limit
     */
    int limit() const;

    /**
     * Sets the list mode
     */
    void setContentMode(ContentMode mode);

    /**
     * @returns display mode
     */
    ContentMode contentMode() const;

Q_SIGNALS:
    void applicationChanged(const QString & application);
    void activityChanged(const QString & activity);
    void limitChanged(int limit);


private:
    Q_PRIVATE_SLOT(d, void servicePresenceChanged(bool))
    Q_PRIVATE_SLOT(d, void resourceScoreUpdated(QString, QString, QString, double))

    Q_PRIVATE_SLOT(d, void newEntries(QList<Nepomuk2::Query::Result>))
    Q_PRIVATE_SLOT(d, void entriesRemoved(QList<QUrl>))
    Q_PRIVATE_SLOT(d, void error(QString))

    Q_PRIVATE_SLOT(d, void setCurrentActivity(QString))

    friend class Private;

    class Private;
    Private * const d;
};

} // namespace Models
} // namespace KActivities

#endif // KACTIVITIES_MODELS_RESOURCE_MODEL_H

