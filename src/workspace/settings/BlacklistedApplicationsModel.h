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

#ifndef BLACKLISTED_APPLICATIONS_MODEL_H
#define BLACKLISTED_APPLICATIONS_MODEL_H

#include <QAbstractListModel>

#include <utils/override.h>
#include <utils/nullptr.h>
#include <utils/d_ptr.h>

/**
 * BlacklistedApplicationsModel
 */
class BlacklistedApplicationsModel: public QAbstractListModel {
    Q_OBJECT

    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)

public:
    BlacklistedApplicationsModel(QObject * parent = nullptr);

    enum Roles {
        ApplicationIdRole = Qt::UserRole + 1,
        BlockedApplicationRole
    };

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const _override;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const _override;
    int rowCount(const QModelIndex & parent = QModelIndex()) const _override;

Q_SIGNALS:
    void changed();
    void enabledChanged(bool enabled);

public Q_SLOTS:
    void toggleApplicationBlocked(int index);

    void setEnabled(bool);
    bool enabled() const;

    void load();
    void save();
    void defaults();

private:
    D_PTR;
};

#endif // BLACKLISTED_APPLICATIONS_MODEL_H

