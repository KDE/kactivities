/*
 *   Copyright (C) 2011 Ivan Cukic ivan.cukic(at)kde.org
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2 of
 *   the License, or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Rankings.h"
#include "rankingsadaptor.h"

#include <QDBusConnection>
#include <QVariantList>
#include <KDebug>

#include <Nepomuk/Resource>
#include <Nepomuk/ResourceManager>
#include <Nepomuk/Variant>

#include <Nepomuk/Query/Query>
#include <Nepomuk/Query/ResourceTerm>
#include <Nepomuk/Query/ResourceTypeTerm>
#include <Nepomuk/Query/ComparisonTerm>
#include <Nepomuk/Query/LiteralTerm>
#include <Nepomuk/Query/NegationTerm>

#include <Soprano/QueryResultIterator>
#include <Soprano/Node>
#include <Soprano/Model>

#include <Nepomuk/Vocabulary/NUAO>
#include <Soprano/Vocabulary/NAO>
#include "kext.h"

#include "NepomukCommon.h"

#define RESULT_COUNT_LIMIT 10
#define COALESCE_ACTIVITY(Activity) ((Activity.isEmpty()) ? \
        (NepomukPlugin::self()->sharedInfo()->currentActivity()) : (Activity))

using namespace Soprano::Vocabulary;
using namespace Nepomuk::Vocabulary;
using namespace Nepomuk::Query;

Rankings * Rankings::s_instance = NULL;

void Rankings::init(QObject * parent)
{
    if (s_instance) return;

    s_instance = new Rankings(parent);
}

Rankings * Rankings::self()
{
    return s_instance;
}

Rankings::Rankings(QObject * parent)
    : QObject(parent)
{
    kDebug() << "%%%%%%%%%% We are in the Rankings %%%%%%%%%%";

    QDBusConnection dbus = QDBusConnection::sessionBus();
    new RankingsAdaptor(this);
    dbus.registerObject("/Rankings", this);

    initResults(QString());
}

Rankings::~Rankings()
{
}

void Rankings::registerClient(const QString & client,
        const QString & activity, const QString & type)
{
    kDebug() << client << "wants to get resources for" << activity;

    if (!m_clients.contains(activity)) {
        kDebug() << "Initialising the resources for" << activity;
        initResults(COALESCE_ACTIVITY(activity));
    }

    if (!m_clients[activity].contains(client)) {
        kDebug() << "Adding client";
        m_clients[activity] << client;
    }

    notifyResultsUpdated(activity, QStringList() << client);
}

void Rankings::deregisterClient(const QString & client)
{
    QMutableHashIterator < Activity, QStringList > i(m_clients);

    while (i.hasNext()) {
        i.next();

        i.value().removeAll(client);
        if (i.value().isEmpty()) {
            i.remove();
        }
    }
}

void Rankings::setCurrentActivity(const QString & activity)
{
    // We need to update scores for items that have no
    // activity specified

    initResults(activity);
}

QUrl Rankings::urlFor(const Nepomuk::Resource & resource)
{
    if (resource.hasProperty(NIE::url())) {
        kDebug() << "Returning URI" << resource.property(NIE::url()).toUrl();
        return resource.property(NIE::url()).toUrl();
    } else {
        kWarning() << "Returning nepomuk URI" << resource.resourceUri();
        return resource.resourceUri();
    }
}

void Rankings::initResults(const QString & _activity)
{
    const QString & activity = COALESCE_ACTIVITY(_activity);

    m_results[activity].clear();
    updateScoreTrashold(activity);

#define QUERY_DEBUGGING
#ifndef QUERY_DEBUGGING
    const QString query = QString::fromLatin1(
        "select distinct ?resource, "
        "( "
            "( "
                "SUM ( "
                    "?lastScore * bif:exp( "
                        "- bif:datediff('day', ?lastUpdate, %1) "
                    ") "
                ") "
            ") "
            "as ?score "
        ") where { "
            "?cache kext:targettedResource ?resource . "
            "?cache a kext:ResourceScoreCache . "
            "?cache nao:lastModified ?lastUpdate . "
            "?cache kext:cachedScore ?lastScore . "
            // "?resource nao:prefLabel ?label . "
            // "?resource nie:url ?icon . "
            // "?resource nie:url ?description . "
            "?cache kext:usedActivity %2 . "
        "} "
        "GROUP BY (?resource) ORDER BY DESC (?score) LIMIT 10"
    ).arg(
        litN3(QDateTime::currentDateTime()),
        resN3(activityResource(activity))
    );

#else
    kDebug() << "\n\nvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv\n\n";

    const QString query = QString::fromLatin1(
    "    select distinct ?resource, \n"
    "    ( \n"
    "        ( \n"
    "            SUM ( \n"
    "                ?lastScore * bif:exp( \n"
    "                    - bif:datediff('day', ?lastUpdate, %1) \n"
    "                ) \n"
    "            ) \n"
    "        ) \n"
    "        as ?score \n"
    "    ) where { \n"
    "        ?cache kext:targettedResource ?resource . \n"
    "        ?cache a kext:ResourceScoreCache . \n"
    "        ?cache nao:lastModified ?lastUpdate . \n"
    "        ?cache kext:cachedScore ?lastScore . \n"
    // "        ?resource nao:prefLabel ?label . \n"
    // "        ?resource nie:url ?icon . \n"
    // "        ?resource nie:url ?description . \n"
    "        ?cache kext:usedActivity %2 . \n"
    "    } \n"
    "    GROUP BY (?resource) ORDER BY DESC (?score) LIMIT 10\n"
    ).arg(
        litN3(QDateTime::currentDateTime()),
        resN3(activityResource(activity))
    );

    kDebug() << query;
#endif

    Soprano::QueryResultIterator it
        = Nepomuk::ResourceManager::instance()->mainModel()
                ->executeQuery(query, Soprano::Query::QueryLanguageSparql);

    while (it.next()) {
        Nepomuk::Resource result(it[0].uri());

        kDebug() << "This is one result:\n"
            << " URI:" << result.property(NIE::url()).toString() << "\n"
            // << it[0].uri() << result << " - " << "\n"
            << it[1].literal().toDouble() << "\n"
            // << " - ### uri:" << result.resourceUri() << "\n"
            << " - label:" << result.label() << "\n"
            << " - icon:" << result.genericIcon() << "\n"
            << " IDS:" << result.identifiers() << "\n"
            << " Type:" << result.types() << "\n"
        ;

        qreal score = it[1].literal().toDouble();

        if (score > m_resultScoreTreshold[activity]) {
            m_results[activity] << ResultItem(
                    urlFor(result), score);
        }
    }

    it.close();

    notifyResultsUpdated(activity);
}

void Rankings::resourceScoreUpdated(const QString & activity,
        const QString & application, const QUrl & uri, qreal score)
{
    kDebug() << activity << application << uri << score;

    if (score <= m_resultScoreTreshold[activity]) {
        kDebug() << "This one didn't even qualify";
        return;
    }

    QList < ResultItem > & list = m_results[activity];

    // Removing the item from the list if it is already in it

    for (int i = 0; i < list.size(); i++) {
        if (list[i].uri == uri) {
            list.removeAt(i);
            break;
        }
    }

    // Adding the item

    ResultItem item(uri, score);

    if (list.size() == 0) {
        list << item;

    } else {
        int i;

        for (i = 0; i < list.size(); i++) {
            if (list[i].score < score) {
                list.insert(i, item);
                break;
            }
        }

        if (i == list.size()) {
            list << item;
        }
    }

    while (list.size() > RESULT_COUNT_LIMIT) {
        list.removeLast();
    }

    notifyResultsUpdated(activity);
}

void Rankings::updateScoreTrashold(const QString & activity)
{
    if (m_results[activity].size() >= RESULT_COUNT_LIMIT) {
        m_resultScoreTreshold[activity] = m_results[activity].last().score;
    } else {
        m_resultScoreTreshold[activity] = 0;
    }
}

void Rankings::notifyResultsUpdated(const QString & _activity, QStringList clients)
{
    const QString & activity = COALESCE_ACTIVITY(_activity);

    updateScoreTrashold(activity);

    QVariantList data;
    foreach (const ResultItem & item, m_results[activity]) {
        kDebug() << item.uri << item.score;
        data << item.uri.toString();
    }

    kDebug() << "These are the clients" << m_clients << "We are gonna update this:" << clients;

    if (clients.isEmpty()) {
        clients = m_clients[activity];
        kDebug() << "This is the current activity" << activity
                 << "And the clients for it" << clients;

        if (activity == NepomukPlugin::self()->sharedInfo()->currentActivity()) {
            kDebug() << "This is the current activity, notifying all";
            clients.append(m_clients[QString()]);
        }
    }

    kDebug() << "Notify clients" << clients << data;

    foreach (const QString & client, clients) {
        QDBusInterface rankingsservice(client, "/RankingsClient", "org.kde.ActivityManager.RankingsClient");
        rankingsservice.call("updated", data);
    }
}
