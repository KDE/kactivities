/*
 *   Copyright (C) 2012, 2013, 2014, 2015 Ivan Cukic <ivan.cukic(at)kde.org>
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

#ifndef KACTIVITIES_IMPORTS_ACTIVITY_INFO_H
#define KACTIVITIES_IMPORTS_ACTIVITY_INFO_H

// Qt
#include <QObject>

// STL
#include <memory>

// Local
#include <lib/controller.h>
#include <lib/info.h>

namespace KActivities {
namespace Imports {

/**
 * ActivityInfo
 */

class ActivityInfo : public QObject {
    Q_OBJECT

    /**
     * Unique identifier of the activity
     */
    Q_PROPERTY(QString activityId READ activityId WRITE setActivityId NOTIFY activityIdChanged)

    /**
     * Name of the activity
     */
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)

    /**
     * Name of the activity
     */
    Q_PROPERTY(QString description READ description WRITE setDescription NOTIFY descriptionChanged)

    /**
     * Activity icon
     */
    Q_PROPERTY(QString icon READ icon WRITE setIcon NOTIFY iconChanged)

    /**
     * Is the activity a valid one - does it exist?
     */
    Q_PROPERTY(bool valid READ valid NOTIFY validChanged)

public:
    ActivityInfo(QObject *parent = nullptr);
    virtual ~ActivityInfo();

public Q_SLOTS:
    void setActivityId(const QString &id);
    QString activityId() const;

    void setName(const QString &name);
    QString name() const;

    void setDescription(const QString &description);
    QString description() const;

    void setIcon(const QString &icon);
    QString icon() const;

    bool valid() const;

Q_SIGNALS:
    void activityIdChanged(const QString &id);
    void nameChanged(const QString &name);
    void descriptionChanged(const QString &description);
    void iconChanged(const QString &icon);
    void validChanged(bool valid);

private Q_SLOTS:
    void setCurrentActivity(const QString &id);

private:
    void setIdInternal(const QString &id);

    KActivities::Controller m_service;
    std::unique_ptr<KActivities::Info> m_info;
    bool m_showCurrentActivity;
};

} // namespace Imports
} // namespace KActivities

#endif // KACTIVITIES_IMPORTS_ACTIVITY_INFO_H

