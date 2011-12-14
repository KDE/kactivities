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
#include <QSqlQuery>

#include <KDebug>

#include "ResourceScoreCache.h"
#include "DatabaseConnection.h"
#include "StatsPlugin.h"

#define RESULT_COUNT_LIMIT 10
#define COALESCE_ACTIVITY(Activity) ((Activity.isEmpty()) ? \
        (StatsPlugin::self()->sharedInfo()->currentActivity()) : (Activity))

Rankings * Rankings::s_instance = NULL;

/**
 *
 */
RankingsUpdateThread::RankingsUpdateThread(const QString & activity, QList < Rankings::ResultItem > * listptr,
        QHash < Rankings::Activity, qreal > * scoreTrashold)
    : m_activity(activity), m_listptr(listptr), m_scoreTrashold(scoreTrashold)
{
}

RankingsUpdateThread::~RankingsUpdateThread()
{
}

void RankingsUpdateThread::run() {
    kDebug() << "This is the activity we want the results for:" << m_activity;

    const QString & query = QString::fromLatin1(
            "SELECT targettedResource, score "
            "FROM kext_ResourceScoreCache "
            "WHERE usedActivity = '%1' "
            "AND score > 0 "
            "ORDER BY score DESC LIMIT 30"
        ).arg(m_activity);

    kDebug() << query;

    QSqlQuery it(query, DatabaseConnection::self()->database());

    while (it.next()) {
        QString url = it.value(0).toString();
        qreal score = it.value(1).toReal();

        kDebug() << "This is one result:\n"
            << url << score << "\n"
        ;

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
    // kDebug() << "%%%%%%%%%% We are in the Rankings %%%%%%%%%%";

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
    Q_UNUSED(type);
    kDebug() << client << "wants to get resources for" << activity;

    if (!m_clients.contains(activity)) {
        // kDebug() << "Initialising the resources for" << activity;
        initResults(COALESCE_ACTIVITY(activity));
    }

    if (!m_clients[activity].contains(client)) {
        // kDebug() << "Adding client";
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

    kDebug() << "Current activity is" << activity;
    initResults(activity);
}

void Rankings::initResults(const QString & _activity)
{
    const QString & activity = COALESCE_ACTIVITY(_activity);

    m_results[activity].clear();
    notifyResultsUpdated(activity);

    // Some debugging needed

    kDebug() << "Initializing the resources for:" << activity;

    // Async loading - first a synced initialize

    m_results[activity].clear();
    updateScoreTrashold(activity);

    // TODO:
    RankingsUpdateThread * thread = new RankingsUpdateThread(
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
    // kDebug() << activity << application << uri << score;

    if (score <= m_resultScoreTreshold[activity]) {
        // kDebug() << "This one didn't even qualify";
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
        // kDebug() << item.uri << item.score;
        data << item.uri.toString();
    }

    // kDebug() << "These are the clients" << m_clients << "We are gonna update this:" << clients;

    if (clients.isEmpty()) {
        clients = m_clients[activity];
        // kDebug() << "This is the current activity" << activity
        //          << "And the clients for it" << clients;

        if (activity == StatsPlugin::self()->sharedInfo()->currentActivity()) {
            // kDebug() << "This is the current activity, notifying all";
            clients.append(m_clients[QString()]);
        }
    }

    kDebug() << "Notify clients" << clients << data;

    foreach (const QString & client, clients) {
        QDBusInterface rankingsservice(client, "/RankingsClient", "org.kde.ActivityManager.RankingsClient");
        rankingsservice.asyncCall("updated", data);
    }
}

void Rankings::requestScoreUpdate(const QString & activity, const QString & application, const QString & resource)
{
    ResourceScoreCache(activity, application, resource).updateScore();
}
