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

#include "ActivityRankingPlugin.h"
#include "ActivityRanking.h"

#include <QThread>

#include <memory>

#include <utils/d_ptr_implementation.h>

class ActivityRankingPlugin::Private {
public:
    ActivityRanking * ranking;
    QThread * rankingThread;
};

ActivityRankingPlugin::ActivityRankingPlugin(QObject * parent, const QVariantList & args)
    : Plugin(parent)
{
    Q_UNUSED(args)
}

bool ActivityRankingPlugin::init(const QHash < QString, QObject * > & modules)
{
    d->ranking = new ActivityRanking();
    d->ranking->init(modules["activities"]);

    class Thread: public QThread {
    public:
        Thread(ActivityRanking * ptr = nullptr)
            : QThread(), object(ptr)
        {
        }

        void run() _override
        {
            std::unique_ptr<ActivityRanking> o(object);
            exec();
        }

    private:
        ActivityRanking * object;

    } * thread = new Thread(d->ranking);

    d->rankingThread = thread;
    d->ranking->moveToThread(thread);
    thread->start();

    qDebug() << "running in thread" << d->ranking->metaObject()->className();

    return true;
}

ActivityRankingPlugin::~ActivityRankingPlugin()
{
    d->rankingThread->exit();
    d->rankingThread->wait();
}

KAMD_EXPORT_PLUGIN(ActivityRankingPlugin, "activitymanger_plugin_activityranking")

#include "ActivityRankingPlugin.moc"
