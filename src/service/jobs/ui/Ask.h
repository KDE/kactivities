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

#ifndef JOBS_UI_ASK_H
#define JOBS_UI_ASK_H

#include "../Job.h"
#include "../JobFactory.h"

#include <QStringList>

namespace Jobs {
namespace Ui {

/**
 * Ask
 */
class Ask: public Job {
    Q_OBJECT
    Q_PROPERTY(QString message READ message WRITE setMessage)
    Q_PROPERTY(QString title   READ title   WRITE setTitle)
    Q_PROPERTY(QStringList choices READ choices WRITE setChoices)

public:
    DECLARE_JOB_FACTORY(Ask, (const QString & title, const QString & message, const QStringList & choices));

    QString message() const;
    void setMessage(const QString & message);

    QString title() const;
    void setTitle(const QString & title);

    QStringList choices() const;
    void setChoices(const QStringList & choices);

    virtual void start() _override;

private Q_SLOTS:
    void choiceChosen(int choice);

private:
    QString m_message;
    QString m_title;
    QStringList m_choices;

};

inline Ask::Factory * ask(const QString & title, const QString & message, const QStringList & choices) {
    return new Ask::Factory(title, message, choices);
}

} // namespace Ui
} // namespace Jobs

#endif // JOBS_UI_ASK_H

