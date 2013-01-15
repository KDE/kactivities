/*
 *   Copyright (C) 2011, 2012 Ivan Cukic <ivan.cukic@kde.org>
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

#ifndef PLUGINS_NEPOMUK_PLUGIN_H
#define PLUGINS_NEPOMUK_PLUGIN_H

#include <Plugin.h>

#include <memory>

#include <utils/d_ptr.h>
#include <utils/nullptr.h>
#include <utils/override.h>

class QFileSystemWatcher;

class NepomukPlugin: public Plugin {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.ActivityManager.ResourcesLinking")

public:
    explicit NepomukPlugin(QObject *parent = nullptr, const QVariantList & args = QVariantList());
    ~NepomukPlugin();

    static NepomukPlugin * self();

    virtual bool init(const QHash < QString, QObject * > & modules) _override;

public Q_SLOTS:
    // Resource linking slots
    void LinkResourceToActivity(const QString & uri, const QString & activity = QString());
    void UnlinkResourceFromActivity(const QString & uri, const QString & activity = QString());
    bool IsResourceLinkedToActivity(const QString & uri, const QString & activity = QString()) const;

    QStringList ResourcesLinkedToActivity(const QString & activity = QString()) const;

private Q_SLOTS:
    // Activity slots
    void setActivityName(const QString & activity, const QString & name);
    void setActivityIcon(const QString & activity, const QString & icon);
    void setCurrentActivity(const QString & activity);
    void addActivity(const QString & activity);
    void removeActivity(const QString & activity);

    // Resource score slots
    void resourceScoreUpdated(const QString & activity, const QString & client, const QString & resource, double score);
    void deleteRecentStats(const QString & activity, int count, const QString & what);
    void deleteEarlierStats(const QString & activity, int months);

    // Nepomuk slots
    void nepomukSystemStarted();
    void nepomukSystemStopped();

    // setResourceMimeType
    // setResourceTitle

public:
    virtual bool isFeatureOperational(const QStringList & feature) const _override;
    virtual bool isFeatureEnabled(const QStringList & feature) const _override;
    virtual void setFeatureEnabled(const QStringList & feature, bool value) _override;
    virtual QStringList listFeatures(const QStringList & feature) const _override;

private:
    D_PTR;
};

#endif // PLUGINS_NEPOMUK_PLUGIN_H
