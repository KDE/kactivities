/*
 *   Copyright (C) 2010, 2011, 2012 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "ActivityManager.h"
#include "ActivityManager_p.h"
#include "NepomukActivityManager.h"

#include <KDebug>
#include "EventProcessor.h"

#ifdef HAVE_NEPOMUK
#include "jobs/encryption/Common.h"
#include "jobs/nepomuk/Move.h"
#endif

void ActivityManager::RegisterResourceEvent(QString application, uint _windowId,
        const QString & uri, uint event, uint reason)
{
    if (event > Event::LastEventType || reason > Event::LastEventReason)
        return;

    if (uri.isEmpty() || application.isEmpty())
        return;

    // Dirty way to skip special web browser URIs
    if (uri.startsWith("about:"))
        return;

    // Dirty way to skip invalid URIs (needed for akregator)
    QChar firstChar = uri[0];
    if (
            (firstChar < 'a' || firstChar > 'z') &&
            (firstChar < 'A' || firstChar > 'Z')
       ) return;

    KUrl kuri(uri);
    WId windowId = (WId) _windowId;

    kDebug() << "New event on the horizon" << application << windowId << event << uri;

    EXEC_NEPOMUK( toRealUri(kuri) );

    if (event == Event::Opened) {

        d->windows[windowId].resources << kuri;
        d->resources[kuri].activities << CurrentActivity();

    } else if (event == Event::Closed) {

        // TODO: Remove from d->resources if needed
        d->windows.remove(windowId);

    }

    EventProcessor::self()->addEvent(application, windowId,
            kuri.url(), (Event::Type) event, (Event::Reason) reason);

}


void ActivityManager::RegisterResourceMimeType(const QString & uri, const QString & mimetype)
{
    kDebug() << "Setting the mime for" << uri << "to be" << mimetype;
    KUrl kuri(uri);

    d->resources[kuri].mimetype = mimetype;

    EXEC_NEPOMUK( setResourceMimeType(KUrl(uri), mimetype) );
}


void ActivityManager::RegisterResourceTitle(const QString & uri, const QString & title)
{
    // A dirty saninty check for the title
    if (title.length() < 3) return;

    kDebug() << "Setting the title for" << uri << "to be" << title << title.length();
    KUrl kuri(uri);

    d->resources[kuri].title = title;

    EXEC_NEPOMUK( setResourceTitle(KUrl(uri), title) );
}


void ActivityManager::LinkResourceToActivity(const QString & uri, const QString & _activity)
{
    #ifdef HAVE_NEPOMUK
    const QString & activity = _activity.isEmpty() ? CurrentActivity() : _activity;

    EXEC_NEPOMUK( linkResourceToActivity(KUrl(uri), activity) );

    if (Jobs::Encryption::Common::isActivityEncrypted(activity)) {
        Jobs::Nepomuk::move(activity, true, QStringList() << uri)
            ->create(this)->start();
    }
    #endif
}


void ActivityManager::UnlinkResourceFromActivity(const QString & uri, const QString & _activity)
{
    #ifdef HAVE_NEPOMUK
    const QString & activity = _activity.isEmpty() ? CurrentActivity() : _activity;

    EXEC_NEPOMUK( unlinkResourceFromActivity(KUrl(uri), activity) );

    // if (Jobs::Encryption::Common::isActivityEncrypted(activity)) {
    //     Jobs::Nepomuk::move(activity, true, QStringList() << uri)
    //         ->create(this)->start();
    // }
    #endif
}


bool ActivityManager::IsResourceLinkedToActivity(const QString & uri, const QString & _activity) const
{
    #ifdef HAVE_NEPOMUK
    const QString & activity = _activity.isEmpty() ? CurrentActivity() : _activity;

    return EXEC_NEPOMUK( isResourceLinkedToActivity(KUrl(uri), activity) );

    #else
    return false;
    #endif
}


QStringList ActivityManager::ResourcesLinkedToActivity(const QString & activity) const
{
    QStringList result;

    foreach (const KUrl & uri, resourcesLinkedToActivity(activity.isEmpty() ? CurrentActivity() : activity)) {
        result << uri.url();
    }

    return result;
}


QList <KUrl> ActivityManager::resourcesLinkedToActivity(const QString & activity) const
{
    if (NEPOMUK_PRESENT) {
    #ifdef HAVE_NEPOMUK
        return EXEC_NEPOMUK(resourcesLinkedToActivity(activity.isEmpty() ? CurrentActivity() : activity));
    #endif
    } else return QList <KUrl>();
}

