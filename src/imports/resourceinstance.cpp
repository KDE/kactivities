/***************************************************************************
 *   Copyright 2011-2015 Marco Martin <mart@kde.org>                            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU Library General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "resourceinstance.h"

#include <QQuickWindow>
#include <QTimer>

#include <KActivities/ResourceInstance>
#include <QDebug>

namespace KActivities {
namespace Imports {

ResourceInstance::ResourceInstance(QQuickItem *parent)
    : QQuickItem(parent)
{
    m_syncTimer = new QTimer(this);
    m_syncTimer->setSingleShot(true);
    connect(m_syncTimer, SIGNAL(timeout()), this, SLOT(syncWid()));
}

ResourceInstance::~ResourceInstance()
{
}

void ResourceInstance::syncWid()
{
    QWindow *w = window();
    if (!w) {
        return;
    }

    WId wid = w->winId();
    if (!m_resourceInstance || m_resourceInstance->winId() != wid) {
        // qDebug() << "Creating a new instance of the resource" << m_uri << "window id" << wid;
        m_resourceInstance.reset(new KActivities::ResourceInstance(wid, m_uri, m_mimetype, m_title));
    } else {

        if (m_uri.scheme().startsWith(QLatin1String("http")) && !m_uri.hasQuery() && m_uri.path().endsWith(QLatin1Char('/'))) {
            const QString &oldPath = m_uri.path();
            m_uri.setPath(oldPath.left(oldPath.length() - 1));

            // qDebug() << "Old and new path" << oldPath << m_uri;

        } else {
            m_resourceInstance->setUri(m_uri);
        }

        // qDebug() << "Setting" << m_uri << m_mimetype << "to window" << wid;

        m_resourceInstance->setMimetype(m_mimetype);
        m_resourceInstance->setTitle(m_title);
    }
}

QUrl ResourceInstance::uri() const
{
    return m_uri;
}

void ResourceInstance::setUri(const QUrl &uri)
{
    if (m_uri == uri) {
        return;
    }

    m_uri = uri;
    m_syncTimer->start(100);
}

QString ResourceInstance::mimetype() const
{
    return m_mimetype;
}

void ResourceInstance::setMimetype(const QString &mimetype)
{
    if (m_mimetype == mimetype) {
        return;
    }
    m_mimetype = mimetype;
    m_syncTimer->start(100);
}

QString ResourceInstance::title() const
{
    return m_title;
}

void ResourceInstance::setTitle(const QString &title)
{
    if (m_title == title) {
        return;
    }
    m_title = title;
    m_syncTimer->start(100);
}

void ResourceInstance::notifyModified()
{
    //ensure the resource instance exists
    syncWid();
    m_resourceInstance->notifyModified();
}

void ResourceInstance::notifyFocusedIn()
{
    //ensure the resource instance exists
    syncWid();
    m_resourceInstance->notifyFocusedIn();
}

void ResourceInstance::notifyFocusedOut()
{
    //ensure the resource instance exists
    syncWid();
    m_resourceInstance->notifyFocusedOut();
}

}
}


