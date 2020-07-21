/*
    SPDX-FileCopyrightText: 2011-2015 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef RESOURCEINSTANCE_H
#define RESOURCEINSTANCE_H

//Qt
#include <QQuickItem>
#include <QUrl>

// STL
#include <memory>

namespace KActivities {
class ResourceInstance;
}

class QTimer;


namespace KActivities {
namespace Imports {

class ResourceInstance : public QQuickItem
{
    Q_OBJECT

    Q_PROPERTY(QUrl uri READ uri WRITE setUri NOTIFY uriChanged)
    Q_PROPERTY(QString mimetype READ mimetype WRITE setMimetype NOTIFY mimetypeChanged)
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)

public:
    explicit ResourceInstance(QQuickItem *parent = nullptr);
    ~ResourceInstance();

    QUrl uri() const;
    void setUri(const QUrl &uri);

    QString mimetype() const;
    void setMimetype(const QString &mimetype);

    QString title() const;
    void setTitle(const QString &title);

protected Q_SLOTS:
    void syncWid();

Q_SIGNALS:
    void uriChanged();
    void mimetypeChanged();
    void titleChanged();

public Q_SLOTS:
    /**
     * Call this method to notify the system that you modified
     * (the contents of) the resource
     */
    void notifyModified();

    /**
     * Call this method to notify the system that the resource
     * has the focus in your application
     * @note You only need to call this in MDI applications
     */
    void notifyFocusedIn();

    /**
     * Call this method to notify the system that the resource
     * lost the focus in your application
     * @note You only need to call this in MDI applications
     */
    void notifyFocusedOut();

private:
    std::unique_ptr<KActivities::ResourceInstance> m_resourceInstance;
    QUrl m_uri;
    QString m_mimetype;
    QString m_title;
    QTimer *m_syncTimer;
};

}
}

#endif
