/*
    SPDX-FileCopyrightText: 2011-2015 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

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
        m_resourceInstance->setUri(m_uri);
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

    m_uri = uri.adjusted(QUrl::StripTrailingSlash);
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


