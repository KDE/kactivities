/*
 *   Copyright (C) 2012 Ivan Cukic <ivan.cukic(at)kde.org>
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

#ifndef NEPOMUK_MOVE_H_
#define NEPOMUK_MOVE_H_

#include "config-features.h"
#ifdef HAVE_NEPOMUK

#include "../Job.h"
#include "../JobFactory.h"

#include <KUrl>
#include <QThread>
#include <QSet>

namespace Nepomuk {
    class Resource;
    class File;
} // namespace Nepomuk

namespace Jobs {
namespace Nepomuk {

/**
 * Move
 */
class Move: public Job {
    Q_OBJECT
    Q_PROPERTY(QString activity READ activity WRITE setActivity)
    Q_PROPERTY(bool    toEncrypted READ toEncrypted WRITE setToEncrypted)

public:
    DECLARE_JOB_FACTORY(Move, (const QString & activity, bool toEncrypted));

    QString activity() const;
    void setActivity(const QString & activity);

    bool toEncrypted() const;
    void setToEncrypted(bool value);

    virtual void start();

public Q_SLOTS:
    void moveFiles(const KUrl::List & results);
    void emitResult();

private:
    QString destination() const;

    QString m_activity;
    bool    m_toEncrypted;

};

class CollectFilesToMove: public QThread {
    Q_OBJECT

public:
    CollectFilesToMove(const QString & activity, const QString & destination);

    void replaceUrl(::Nepomuk::File & file, const QString & destination);
    void unlinkOtherActivities(::Nepomuk::Resource & resource);
    void removeSensitiveData(::Nepomuk::Resource & resource);

    void scheduleMoveDir(::Nepomuk::File & dir);
    void scheduleMoveFile(::Nepomuk::File & file);
    void scheduleMove(::Nepomuk::File & item);
    void run();

Q_SIGNALS:
    void result(const KUrl::List & list);

private:
    QString m_activity;
    QString m_destination;
    KUrl::List m_scheduledForMoving;
    QSet < QString > m_movedDirs;
};

inline Move::Factory * move(const QString & activity, bool toEncrypted) {
    return new Move::Factory(activity, toEncrypted);
}

} // namespace Nepomuk
} // namespace Jobs

#endif // HAVE_NEPOMUK
#endif // NEPOMUK_MOVE_H_

