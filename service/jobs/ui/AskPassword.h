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

#ifndef PASSWORD_ASK_H_
#define PASSWORD_ASK_H_

#include "../Job.h"
#include "../JobFactory.h"

namespace Jobs {
namespace Ui {

/**
 * AskPassword
 */
class AskPassword: public Job {
    Q_OBJECT
    Q_PROPERTY(QString message READ message WRITE setMessage)
    Q_PROPERTY(QString title   READ title   WRITE setTitle)
    Q_PROPERTY(bool    shouldVerify READ shouldVerify WRITE setShouldVerify)

public:
    DECLARE_JOB_FACTORY(AskPassword, (const QString & title, const QString & message, bool shouldVerify = false));

    QString message() const;
    void setMessage(const QString & message);

    QString title() const;
    void setTitle(const QString & title);

    bool shouldVerify() const;
    void setShouldVerify(bool value);

    virtual void start();

    static const char * passwordField();

private Q_SLOTS:
    void passwordReturned(const QString & password);

private:
    QString m_message;
    QString m_title;
    bool    m_shouldVerify;

};

inline AskPassword::Factory * askPassword(const QString & title, const QString & message, bool shouldVerify = false) {
    return new AskPassword::Factory(title, message, shouldVerify);
}

} // namespace Ui
} // namespace Jobs

#endif // PASSWORD_ASK_H_

