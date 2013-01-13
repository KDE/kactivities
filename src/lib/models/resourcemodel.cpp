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
#include <QDebug>

#include <KIcon>
#include <KLocalizedString>
#include <KFileItem>

#include <Nepomuk2/Query/Query>
#include <Nepomuk2/Query/QueryServiceClient>
#include <Nepomuk2/Query/Result>
#include <Nepomuk2/Resource>

#include <Soprano/Vocabulary/NAO>
#include <Nepomuk2/Vocabulary/NIE>

#include <common/dbus/org.kde.ActivityManager.Activities.h>

#include <utils/merge_into.h>

// from libkactivities (core)
#include "../core/manager_p.h"
#include "../core/utils_p.h"

#include "utils_p.h"

namespace Nepomuk = Nepomuk2;
namespace NQuery  = Nepomuk2::Query;

using namespace Soprano::Vocabulary;
using namespace Nepomuk::Vocabulary;

namespace KActivities {
namespace Models {

template <typename T>
inline short sign(T value)
{
    return value >= 0 ? 1 : -1;
}

template <typename T>
inline T abs(T value)
{
    return value >= 0 ? value : -value;
}

struct ResourceInfo {
    QUrl resource;
    QString url;
    QString title;
    QString icon;
    double score;

    bool operator >= (const ResourceInfo & other) const {
        short this_sign  = sign(score);
        short other_sign = sign(other.score);

        if (this_sign != other_sign) {
            return (other_sign == -1);
        }

        double this_abs  = abs(score);
        double other_abs = abs(other.score);

        if (this_abs < other_abs) return true;

        return title < other.title;
    }

    bool operator < (const ResourceInfo & other) const {
        return !(*this >= other);
    }
};

typedef QList < ResourceInfo > ResourceInfoList;

class ResourceModel::Private {
public:
    DECLARE_RAII_MODEL_UPDATERS(ResourceModel)

    Private(ResourceModel * parent)
        : limit(10), service(0), q(parent), valid(false), showCurrentActivity(true)
    {
        servicePresenceChanged(Manager::isServicePresent());

        connect(Manager::self(), SIGNAL(servicePresenceChanged(bool)),
                q, SLOT(servicePresenceChanged(bool)));
    }

    void reload();
    void servicePresenceChanged(bool present);
    void resourceScoreUpdated(const QString & activity, const QString & client, const QString & resource, double score);

    void newEntries(const QList < NQuery::Result > & entries);
    void entriesRemoved(const QList < QUrl > & entries);
    void error(const QString & errorMessage);

    static
    ResourceInfo infoFromResult(const NQuery::Result & result);

    void loadFromQuery(const QString & query);
    void loadLinked();
    void loadTopRated();
    void loadRecent();

    QString activityToShowN3() const;
    void setCurrentActivity(const QString & activity);

    QString activity;
    QString currentActivity;
    QString application;
    int limit;
    ResourceModel::ContentMode contentMode;

    QSet < QString > resourceSet;
    ResourceInfoList resources;
    QList < NQuery::QueryServiceClient * > queries;

    QDBusInterface * service;

    ResourceModel * const q;
    bool valid : 1;
    bool showCurrentActivity : 1;
};

void ResourceModel::Private::reload()
{
    servicePresenceChanged(Manager::isServicePresent());
}

void ResourceModel::Private::resourceScoreUpdated(const QString & activity, const QString & client, const QString & resource, double score)
{
    qDebug() << activity << client << resource << score;
}

void ResourceModel::Private::servicePresenceChanged(bool present)
{
    qDebug() << present;
    model_reset m(q);

    resources.clear();
    resourceSet.clear();

    valid = present;

    if (service) {
        delete service;
        service = 0;
    }

    if (!valid) return;

    if (showCurrentActivity && currentActivity.isEmpty()) {
        // we need to show the current activity, but don't know which it is

        bool res = Manager::activities()->callWithCallback("CurrentActivity", QVariantList(), q, SLOT(setCurrentActivity(QString)));
        qDebug() << "CALLING" << res;

        connect(Manager::activities(), SIGNAL(CurrentActivityChanged(QString)),
                q, SLOT(setCurrentActivity(QString)));

        return;
    }

    service = new QDBusInterface(
            "org.kde.ActivityManager",
            "/ActivityManager/Resources/Scoring",
            "org.kde.ActivityManager.Resources.Scoring"
        );

    // connect(service, SIGNAL(resourceScoreUpdated(QString, QString, QString, double)),
    //         q, SLOT(resourceScoreUpdated(QString, QString, QString, double)));

    qDeleteAll(queries);
    queries.clear();

    contentMode = Recent;

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

void ResourceModel::Private::loadFromQuery(const QString & query)
{
    qDebug() << query;

    NQuery::QueryServiceClient * queryClient = new NQuery::QueryServiceClient(q);

    NQuery::RequestPropertyMap requestPropertyMap;
    requestPropertyMap.insert("url",   NIE::url());
    requestPropertyMap.insert("title", NAO::prefLabel());
    requestPropertyMap.insert("score", NAO::numericRating());
    requestPropertyMap.insert("icon",  NAO::iconName());

    queryClient->sparqlQuery(query, requestPropertyMap);

    connect(queryClient, SIGNAL(newEntries(QList<Nepomuk2::Query::Result>)),
            q, SLOT(newEntries(QList<Nepomuk2::Query::Result>)));
    connect(queryClient, SIGNAL(entriesRemoved(QList<QUrl>)),
            q, SLOT(entriesRemoved(QList<QUrl>)));
    connect(queryClient, SIGNAL(error(QString)),
            q, SLOT(error(QString)));

    queries << queryClient;
}

void ResourceModel::Private::loadLinked()
{
    static const QString & _query = QString::fromLatin1(
            "select distinct ?r, ?url, -1 as ?score, ?title, ?icon where { "
                "?activity nao:isRelated ?r . "
                "?activity kao:activityIdentifier %1. "
                "?r nie:url ?url . "
                "OPTIONAL { ?r nao:prefLabel ?title } . "
                "OPTIONAL { ?r nao:iconName ?icon } . "
                "%2 "
            "}"
        );

    static const QString & _applicationFilter = QString::fromLatin1(
            "?scoreCache a kao:ResourceScoreCache . "
            "?scoreCache kao:usedActivity ?activity . "
            "?scoreCache kao:targettedResource ?r . "
            "?scoreCache kao:initiatingAgent ?agent . "
            "?agent nao:identifier %1 ."
        );

    loadFromQuery(_query.arg(
            activityToShowN3(),
            (application.isEmpty() ?
                QString() :
                _applicationFilter.arg(Soprano::Node::literalToN3(application))
            )
        ));
}

void ResourceModel::Private::loadRecent()
{
    static const QString & _query = QString::fromLatin1(
            "select distinct ?r, ?url, "
            // "(bif:datediff ('second', \"1970-01-01\"^^<http://www.w3.org/2001/XMLSchema#date>, ?lastModified)) as ?score, "
            "(bif:datediff ('second', \"1970-01-01\"^^<http://www.w3.org/2001/XMLSchema#date>, ?lastModified)) as ?score, "
            "?title, ?icon where { "
                "?scoreCache a kao:ResourceScoreCache . "
                "?scoreCache kao:usedActivity ?activity . "
                "?activity kao:activityIdentifier %1. "
                "?scoreCache kao:targettedResource ?r . "
                "?scoreCache nao:lastModified ?lastModified . "
                "?r nie:url ?url . "
                "OPTIONAL { ?r nao:prefLabel ?title } . "
                "OPTIONAL { ?r nao:iconName ?icon } . "
                "%2 "
            "} order by desc(?score) limit %3"
        );

    static const QString & _applicationFilter = QString::fromLatin1(
            "?scoreCache kao:initiatingAgent ?agent . "
            "?agent nao:identifier %1 ."
        );

    qDebug() << Soprano::Node::literalToN3(QDate(1970,1,1));

    loadFromQuery(_query.arg(
            activityToShowN3(),
            (application.isEmpty() ?
                QString() :
                _applicationFilter.arg(Soprano::Node::literalToN3(application))
            ),
            QString::number(limit)
        ));
}

void ResourceModel::Private::loadTopRated()
{
    static const QString & _query = QString::fromLatin1(
            "select distinct ?r, ?url, ?score, ?title, ?icon where { "
                "?scoreCache a kao:ResourceScoreCache . "
                "?scoreCache kao:usedActivity ?activity . "
                "?activity kao:activityIdentifier %1. "
                "?scoreCache kao:targettedResource ?r . "
                "?scoreCache kao:cachedScore ?score . "
                "?r nie:url ?url . "
                "OPTIONAL { ?r nao:prefLabel ?title } . "
                "OPTIONAL { ?r nao:iconName ?icon } . "
                "%2 "
            "} order by desc(?score) limit %3"
        );

    static const QString & _applicationFilter = QString::fromLatin1(
            "?scoreCache kao:initiatingAgent ?agent . "
            "?agent nao:identifier %1 ."
        );

    loadFromQuery(_query.arg(
            activityToShowN3(),
            (application.isEmpty() ?
                QString() :
                _applicationFilter.arg(Soprano::Node::literalToN3(application))
            ),
            QString::number(limit)
        ));
}

QString ResourceModel::Private::activityToShowN3() const
{
    return Soprano::Node::literalToN3(
            activity.isEmpty() ?
                currentActivity :
                activity
        );
}

ResourceInfo ResourceModel::Private::infoFromResult(const NQuery::Result & result)
{
    ResourceInfo info;
    info.resource = result.resource().uri();

    QHash < Nepomuk::Types::Property, Soprano::Node > props = result.requestProperties();

    info.url   = props[NIE::url()].toString();
    info.title = props[NAO::prefLabel()].toString();
    info.icon  = props[NAO::iconName()].toString();
    info.score = props[NAO::numericRating()].toString().toDouble();

    qDebug()
        << info.url
        << info.title
        << info.icon
        << info.score;

    if (info.title.isEmpty() /*&& info.url.startsWith("file://")*/) {
        KFileItem fileItem(KFileItem::Unknown, KFileItem::Unknown, info.url);

        if (fileItem.isFile() || fileItem.isDir()) {
            info.title = fileItem.text();
            info.icon  = fileItem.iconName();

            qDebug() << "## 1 ##" << info.title << info.icon;

        } else {
            info.title = info.url;

        }
    }

    return info;
}

void ResourceModel::Private::newEntries(const QList < NQuery::Result > & entries)
{
    // model_insert m(q, QModelIndex(), 0, entries.size());
    model_reset m(q);

    ResourceInfoList newEntries;

    foreach (const NQuery::Result & result, entries) {
        const ResourceInfo & entry = infoFromResult(result);

        if (entry.title.isEmpty()
            || resourceSet.contains(entry.url)
            || entry.url.startsWith(QLatin1String("filex://")))
                continue;

        resourceSet << entry.url;
        newEntries << entry;

    }

    kamd::utils::merge_into(resources, newEntries);

    valid = 1;
}

void ResourceModel::Private::entriesRemoved(const QList < QUrl > & entries)
{
    model_reset m(q);

    foreach (const QUrl & entry, entries) {
        qDebug() << "Removing: " << entry;

        ResourceInfoList::iterator start = resources.begin();
        ResourceInfoList::iterator end = resources.end();

        while (start != end) {
            if (start->resource == entry) {
                start = resources.erase(start);
                end   = resources.end();

            } else {
                ++start;

            }
        }
    }
}

void ResourceModel::Private::setCurrentActivity(const QString & activity)
{
    if (currentActivity == activity) return;

    currentActivity = activity;

    reload();
}

void ResourceModel::Private::error(const QString & errorMessage)
{
    qDebug() << errorMessage;
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

    return qMin(d->limit, d->resources.size());
}

QVariant ResourceModel::data(const QModelIndex & index, int role) const
{
    if (!d->valid) return QVariant();

    const int row = index.row();

    if (row >= d->resources.size()) return QVariant();

    const ResourceInfo & info = d->resources[row];

    switch (role) {
        case Qt::DisplayRole:
            return info.title;
            // return QString(info.title + " " + QString::number(info.score));

        case Qt::DecorationRole:
            return KIcon(info.icon);

        case ResourceUrl:
            return info.url;

        case ResourceIconName:
            return info.icon;

        case ResourceScore:
            return info.score;

        default:
            return QVariant();
    }
}

QVariant ResourceModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(orientation)

    if (section == 0 && role == Qt::DisplayRole) {
        return i18nc("Header title for resource data model", "Resource");
    }

    return QVariant();
}

void ResourceModel::setActivity(const QString & activity)
{
    if (d->activity == activity) return;

    d->activity = activity;

    d->showCurrentActivity = d->activity.isEmpty();

    emit activityChanged(activity);

    d->reload();
}

QString ResourceModel::activity() const
{
    return d->activity;
}

void ResourceModel::setApplication(const QString & application)
{
    if (d->application == application) return;

    qDebug() << "Setting the application to:" << application;

    d->application = application;

    emit applicationChanged(application);

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

    emit limitChanged(count);

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
