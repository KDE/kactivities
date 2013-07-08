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

#include "SlcPlugin.h"
#include "slcadaptor.h"

#include <QDBusConnection>
#include <kdbusconnectionpool.h>

#include <utils/val.h>

SlcPlugin::SlcPlugin(QObject * parent, const QVariantList & args)
    : Plugin(parent)
{
    Q_UNUSED(args)

    new SLCAdaptor(this);
    KDBusConnectionPool::threadConnection().registerObject("/SLC", this);
}

SlcPlugin::~SlcPlugin()
{
}

QString SlcPlugin::focussedResourceURI() const
{
    return m_focussedResource;
}

QString SlcPlugin::focussedResourceMimetype() const
{
    return m_resources[m_focussedResource].mimetype;
}

QString SlcPlugin::focussedResourceTitle() const
{
    return m_resources[m_focussedResource].title;
}

void SlcPlugin::registeredResourceEvent(const Event & event)
{
    switch (event.type) {
        case Event::FocussedIn:

            if (!event.uri.startsWith(QLatin1String("about"))) {
                if (m_focussedResource != event.uri) {
                    m_focussedResource = event.uri;
                    val & info = m_resources[m_focussedResource];
                    emit focusChanged(event.uri, info.mimetype, info.title);
                }
            } else {
                m_focussedResource.clear();
                emit focusChanged(QString(), QString(), QString());
            }

            break;

        case Event::FocussedOut:

            if (m_focussedResource == event.uri) {
                m_focussedResource.clear();
                emit focusChanged(QString(), QString(), QString());
            }

            break;

        case Event::Closed:
            m_resources.remove(event.uri);

            break;

        default:
            break;
    }
}

void SlcPlugin::registeredResourceMimeType(const QString & uri, const QString & mimetype)
{
    m_resources[uri].mimetype = mimetype;
}

void SlcPlugin::registeredResourceTitle(const QString & uri, const QString & title)
{
    m_resources[uri].title = title;
}

bool SlcPlugin::init(const QHash < QString, QObject * > & modules)
{
    connect(modules["resources"], SIGNAL(RegisteredResourceEvent(Event)),
            this, SLOT(registeredResourceEvent(Event)),
            Qt::QueuedConnection);
    connect(modules["resources"], SIGNAL(RegisteredResourceMimeType(QString, QString)),
            this, SLOT(registeredResourceMimeType(QString, QString)),
            Qt::QueuedConnection);
    connect(modules["resources"], SIGNAL(RegisteredResourceTitle(QString, QString)),
            this, SLOT(registeredResourceTitle(QString, QString)),
            Qt::QueuedConnection);

    return true;
}

KAMD_EXPORT_PLUGIN(SlcPlugin, "activitymanger_plugin_slc")
