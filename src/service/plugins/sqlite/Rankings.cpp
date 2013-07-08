/*
 *   Copyright (C) 2011, 2012 Ivan Cukic ivan.cukic(at)kde.org
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
#include <QSqlQuery>
#include <QDebug>

#include "ResourceScoreCache.h"
#include "DatabaseConnection.h"
#include "StatsPlugin.h"

#include <utils/for_each_assoc.h>
#include <utils/remove_if.h>
#include <utils/val.h>

#define RESULT_COUNT_LIMIT 10
#define COALESCE_ACTIVITY(Activity) ((Activity.isEmpty()) ? \
        (StatsPlugin::self()->currentActivity()) : (Activity))

Rankings * Rankings::s_instance = nullptr;

/**
 *
 */
RankingsUpdateThread::RankingsUpdateThread(
        const QString & activity, QList < Rankings::ResultItem > * listptr,
        QHash < Rankings::Activity, qreal > * scoreTrashold)
    : m_activity(activity), m_listptr(listptr), m_scoreTrashold(scoreTrashold)
{
}

RankingsUpdateThread::~RankingsUpdateThread()
{
}

void RankingsUpdateThread::run() {
    qDebug() << "This is the activity we want the results for:" << m_activity;

    val & query = QString::fromLatin1(
            "SELECT targettedResource, cachedScore "
            "FROM kext_ResourceScoreCache " // this should be kao_ResourceScoreCache, but lets leave it
            "WHERE usedActivity = '%1' "
            "AND cachedScore > 0 "
            "ORDER BY cachedScore DESC LIMIT 30"
        ).arg(m_activity);

    qDebug() << query;

    auto result = DatabaseConnection::self()->database().exec(query);

    while (result.next()) {
        val url   = result.value(0).toString();
        val score = result.value(1).toReal();

        if (score > (*m_scoreTrashold)[m_activity]) {
            (*m_listptr) << Rankings::ResultItem(url, score);
        }
    }

    emit loaded(m_activity);
}

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
    new RankingsAdaptor(this);
    KDBusConnectionPool::threadConnection().registerObject("/Rankings", this);

    initResults(QString());
}

Rankings::~Rankings()
{
}

void Rankings::registerClient(const QString & client,
        const QString & activity, const QString & type)
{
    Q_UNUSED(type);

    if (!m_clients.contains(activity)) {
        initResults(COALESCE_ACTIVITY(activity));
    }

    if (!m_clients[activity].contains(client)) {
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

void Rankings::initResults(const QString & _activity)
{
    val & activity = COALESCE_ACTIVITY(_activity);

    m_results[activity].clear();
    notifyResultsUpdated(activity);
    updateScoreTrashold(activity);

    val thread = new RankingsUpdateThread(
            activity,
            &(m_results[activity]),
            &m_resultScoreTreshold
        );
    connect(thread, SIGNAL(loaded(QString)),
            this, SLOT(notifyResultsUpdated(QString)));
    connect(thread, SIGNAL(terminated()),
            thread, SLOT(deleteLater()));

    thread->start();
}

void Rankings::resourceScoreUpdated(const QString & activity,
        const QString & application, const QUrl & uri, qreal score)
{
    Q_UNUSED(application);

    if (score <= m_resultScoreTreshold[activity]) {
        return;
    }

    auto & list = m_results[activity];

    // Removing the item from the list if it is already in it

    kamd::utils::remove_if(list, [&uri] (const ResultItem & item) {
        return item.uri == uri;
    });

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
    m_resultScoreTreshold[activity] =
        (m_results[activity].size() >= RESULT_COUNT_LIMIT)
            ? m_results[activity].last().score
            : 0
        ;
}

void Rankings::notifyResultsUpdated(const QString & _activity, QStringList clients)
{
    val & activity = COALESCE_ACTIVITY(_activity);

    updateScoreTrashold(activity);

    QVariantList data;
    foreach (val & item, m_results[activity]) {
        data << item.uri.toString();
    }

    if (clients.isEmpty()) {
        clients = m_clients[activity];

        if (activity == StatsPlugin::self()->currentActivity()) {
            clients.append(m_clients[QString()]);
        }
    }

    foreach (val & client, clients) {
        QDBusInterface rankingsservice(client, "/RankingsClient", "org.kde.ActivityManager.RankingsClient");
        rankingsservice.asyncCall("updated", data);
    }
}

void Rankings::requestScoreUpdate(const QString & activity, const QString & application, const QString & resource)
{
    ResourceScoreCache(activity, application, resource).updateScore();
}

