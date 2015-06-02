/*
 *   Copyright (C) 2012, 2013, 2014 Ivan Cukic <ivan.cukic(at)kde.org>
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

#ifndef PLUGINS_EVENT_SPY_PLUGIN_H
#define PLUGINS_EVENT_SPY_PLUGIN_H

#include <memory>
#include <Plugin.h>

#include <QStringList>

class KDirWatch;

class EventSpyPlugin : public Plugin {
    Q_OBJECT

public:
    EventSpyPlugin(QObject *parent = Q_NULLPTR, const QVariantList &args = QVariantList());
    virtual ~EventSpyPlugin();

    bool init(QHash<QString, QObject *> &modules) Q_DECL_OVERRIDE;

private Q_SLOTS:
    void directoryUpdated(const QString &dir);
    void addDocument(const QString &document);

private:
    QObject *m_resources;
    std::unique_ptr<KDirWatch> m_dirWatcher;
    QDateTime m_lastUpdate;
};

#endif // PLUGINS_EVENT_SPY_PLUGIN_H
