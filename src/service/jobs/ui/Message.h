/*
 *   Copyright (C) 2012 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Ui Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Ui Public License for more details
 *
 *   You should have received a copy of the GNU Ui Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef JOBS_UI_MESSAGE_H
#define JOBS_UI_MESSAGE_H

#include "../Job.h"
#include "../JobFactory.h"

namespace Jobs {
namespace Ui {

/**
 * Message
 */
class Message: public Job {
    Q_OBJECT
    Q_PROPERTY(QString message READ message WRITE setMessage)
    Q_PROPERTY(QString title READ title WRITE setTitle)

public:
    enum Type {
        Information,
        Warning,
        Error
    };

    DECLARE_JOB_FACTORY(Message, (const QString & title, const QString & message, int type = Message::Information));

    QString message() const;
    void setMessage(const QString & message);

    QString title() const;
    void setTitle(const QString & title);

    virtual void start() _override;

private:
    QString m_message;
    QString m_title;

};

inline Message::Factory * message(const QString & title, const QString & message, int type = Message::Information)
{
    return new Message::Factory(title, message, type);
}

} // namespace Ui
} // namespace Jobs

#endif // JOBS_UI_MESSAGE_H

