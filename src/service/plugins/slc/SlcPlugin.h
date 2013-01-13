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

#ifndef PLUGINS_SLC_PLUGIN_H
#define PLUGINS_SLC_PLUGIN_H

#include <Plugin.h>

#include <utils/override.h>
#include <utils/nullptr.h>

class SlcPlugin: public Plugin
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.ActivityManager.SLC")

public:
    explicit SlcPlugin(QObject * parent = nullptr, const QVariantList & args = QVariantList());
    ~SlcPlugin();

    virtual bool init(const QHash < QString, QObject * > & modules) _override;

private Q_SLOTS:
    void registeredResourceEvent(const Event & event);
    void registeredResourceMimeType(const QString & uri, const QString & mimetype);
    void registeredResourceTitle(const QString & uri, const QString & title);

public Q_SLOTS:
    QString focussedResourceURI() const;
    QString focussedResourceMimetype() const;
    QString focussedResourceTitle() const;

Q_SIGNALS:
    void focusChanged(const QString & uri, const QString & mimetype, const QString & title);

private:
    struct ResourceInfo {
        QString title;
        QString mimetype;
    };

    QHash < QString, ResourceInfo > m_resources;
    QString m_focussedResource;
};

#endif // PLUGINS_SLC_PLUGIN_H

