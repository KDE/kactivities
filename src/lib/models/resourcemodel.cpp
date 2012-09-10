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

#include "resourcemodel.h"

#include <QByteArray>
#include <QDBusPendingCall>
#include <QDBusPendingCallWatcher>
#include <QHash>
#include <QList>
#include <QModelIndex>

#include <KIcon>
#include <KDebug>
#include <KLocalizedString>

#include <Nepomuk2/Query/QueryServiceClient>
#include <Nepomuk2/Resource>

#include <common/dbus/org.kde.ActivityManager.Activities.h>

// from libkactivities (core)
#include "../core/manager_p.h"
#include "../core/utils_p.h"

#include "utils_p.h"

namespace Nepomuk = Nepomuk2;
namespace NQuery  = Nepomuk2::Query;

namespace KActivities {
namespace Models {

template <typename T>
inline short sign(T value)
{
    return value >= 0 ? 1 : -1;
}

template <typename T>
inline int abs(T value)
{
    return value >= 0 ? value : -value;
}

struct ResourceInfo {
    QString resource;
    double score;

    bool operator < (const ResourceInfo & other) const {
        short this_sign  = sign(score);
        short other_sign = sign(other.score);

        if (this_sign != other_sign) {
            return (other_sign == -1);
        }

        double this_abs  = abs(score);
        double other_abs = abs(other.score);

        if (this_abs < other_abs) return true;

        return resource < other.resource;
    }
};

class ResourceModel::Private {
public:
    DECLARE_RAII_MODEL_UPDATERS(ResourceModel);

    Private(ResourceModel * parent)
        : service(0), q(parent), valid(false)
    {
        servicePresenceChanged(Manager::isServicePresent());

        connect(Manager::self(), SIGNAL(servicePresenceChanged(bool)),
                q, SLOT(servicePresenceChanged(bool)));
    }

    void reload();
    void servicePresenceChanged(bool present);
    void resourceScoreUpdated(const QString & activity, const QString & client, const QString & resource, double score);

    void loadLinked();
    void loadTopRated();
    void loadRecent();

    QString activity;
    QString application;
    int limit;
    ResourceModel::ContentMode contentMode;

    QList < ResourceInfo > resources;
    QList < NQuery::QueryServiceClient * > queries;

    QDBusInterface * service;

    ResourceModel * const q;
    bool valid : 1;
};

void ResourceModel::Private::reload()
{
    servicePresenceChanged(Manager::isServicePresent());
}

void ResourceModel::Private::resourceScoreUpdated(const QString & activity, const QString & client, const QString & resource, double score)
{
    kDebug() << activity << client << resource << score;
}

void ResourceModel::Private::servicePresenceChanged(bool present)
{
    kDebug() << present;
    model_reset m(q);

    valid = present;

    if (service) {
        delete service;
        service = 0;
    }

    if (!valid) return;

    service = new QDBusInterface(
            "org.kde.ActivityManager",
            "/ActivityManager/Resources/Scoring",
            "org.kde.ActivityManager.Resources.Scoring"
        );

    connect(service, SIGNAL(resourceScoreUpdated(QString, QString, QString, double)),
            q, SLOT(resourceScoreUpdated(QString, QString, QString, double)));

    qDeleteAll(queries);
    queries.clear();

    switch (contentMode) {
        case Favorites:
            loadTopRated();
            loadLinked();
            break;

        case Linked:
            loadLinked();
            break;

        case TopRated:
            loadTopRated();
            break;

        case Recent:
            loadRecent();
            break;
    }
}

void ResourceModel::Private::loadLinked()
{
    NQuery::QueryServiceClient * query = new NQuery::QueryServiceClient(q);

    QString query =
        "select ?r, ?url, ?application, ?score { "
            "?scoreCache a kao:ResourceScoreCache . "
            "?scoreCache kao:targettedResource ?r . "
            "?scoreCache kao:cachedScore ?score . "
            "?scoreCache kao:initiatingAgent ?application . "
            "OPTIONAL {?r nie:url ?url} . "
        "}";

    QueryServiceClient::RequestPropertyMap requestPropertyMap;
    requestPropertyMap.insert( "mtime", Vocabulary::NIE::lastModified() );

    query->sparqlQuery(query, requestPropertyMap);

    queries << query;
}

void ResourceModel::Private::loadRecent()
{
}

void ResourceModel::Private::loadTopRated()
{
}

ResourceModel::ResourceModel(QObject * parent)
    : QAbstractListModel(parent), d(new Private(this))
{
    d->valid = false;

    QHash<int, QByteArray> roles;

    roles[Qt::DisplayRole]    = "name";
    roles[Qt::DecorationRole] = "icon";

    setRoleNames(roles);
}

ResourceModel::~ResourceModel()
{
    delete d;
}

int ResourceModel::rowCount(const QModelIndex & parent) const
{
    Q_UNUSED(parent);

    if (!d->valid) return 0;

    return 1;
}

QVariant ResourceModel::data(const QModelIndex & index, int role) const
{
    if (!d->valid) return QVariant();

    const int row = index.row();

    switch (role) {
        case Qt::DisplayRole:
        case Qt::DecorationRole:

        default:
            return QVariant();
    }
}

QVariant ResourceModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (section == 0 && role == Qt::DisplayRole) {
        return i18nc("Header title for resource data model", "Resource");
    }

    return QVariant();
}

void ResourceModel::setActivity(const QString & activity)
{
    if (d->activity == activity) return;

    d->activity = activity;
    d->reload();
}

QString ResourceModel::activity() const
{
    return d->activity;
}

void ResourceModel::setApplication(const QString & application)
{
    if (d->application == application) return;

    d->application = application;
    d->reload();
}

QString ResourceModel::application() const
{
    return d->application;
}

void ResourceModel::setLimit(int count)
{
    if (d->limit == count) return;

    d->limit = count;
    d->reload();
}

int ResourceModel::limit() const
{
    return d->limit;
}

void ResourceModel::setContentMode(ResourceModel::ContentMode mode)
{
    if (d->contentMode == mode) return;

    d->contentMode = mode;
    d->reload();
}

ResourceModel::ContentMode ResourceModel::contentMode() const
{
    return d->contentMode;
}

} // namespace Models
} // namespace KActivities

#include "resourcemodel.moc"
