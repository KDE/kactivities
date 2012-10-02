/*
 *   Copyright (C) 2012 Ivan Cukic ivan.cukic(at)kde.org
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

#include "KDialogUi.h"

#include <knewpassworddialog.h>
#include <kpassworddialog.h>

#include <QMessageBox>
#include <QThread>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QSignalMapper>

#include <KDebug>

#include <utils/d_ptr_implementation.h>

class KDialogUiHandler::Private {
public:
    class AskPassword: public QThread {
    public:
        AskPassword(const QString & _title, const QString & _message, bool _newPassword, bool _unlockMode,
                 QObject * _receiver, const char * _slot)
            : title(_title), message(_message), newPassword(_newPassword), unlockMode(_unlockMode),
              receiver(_receiver), slot(_slot)
        {
        }

        QString askPassword(const QString & title, const QString & message, bool newPassword, bool unlockMode)
        {
            Q_UNUSED(unlockMode)

            #define ShowDialog(Type)                                                      \
                Type dialog;                                                              \
                dialog.setPrompt(message);                                                \
                dialog.setWindowTitle(title);                                             \
                dialog.setWindowFlags(Qt::WindowStaysOnTopHint | dialog.windowFlags());   \
                if (dialog.exec()) return dialog.password();                              \
                return QString();

            if (newPassword) {
                ShowDialog(KNewPasswordDialog);

            } else {
                ShowDialog(KPasswordDialog);

            }

            #undef ShowDialog
        }

        void run() {
            const QString & password = askPassword(title, message, newPassword, unlockMode);

            kDebug() << "Got password .... sending it to" << receiver << slot;

            QMetaObject::invokeMethod(receiver, slot, Qt::QueuedConnection,
                    Q_ARG(QString, password));

            deleteLater();
        }

    private:
        QString title;
        QString message;
        bool newPassword;
        bool unlockMode;
        QObject * receiver;
        const char * slot;
    };

    /**
     *
     */
    class Ask {
    public:
        Ask(const QString & _title, const QString & _message, const QStringList & _choices,
                 QObject * _receiver, const char * _slot)
            : title(_title), message(_message), choices(_choices),
              receiver(_receiver), slot(_slot)
        {
        }

        void run() {
            kDebug();

            KDialog dialog;
            dialog.setWindowFlags(Qt::WindowStaysOnTopHint | dialog.windowFlags());
            dialog.setButtons(KDialog::None);
            dialog.setCaption(title);

            QVBoxLayout * layout = new QVBoxLayout();

            QLabel * labelMessage = new QLabel(message);
            labelMessage->setWordWrap(true);
            layout->addWidget(labelMessage);

            QSignalMapper * signalMapper = new QSignalMapper(&dialog);

            int i = 0;

            foreach (const QString & choice, choices) {
                QPushButton * button = new QPushButton(choice);
                button->setMinimumSize(200, 32);
                layout->addWidget(button);

                connect(
                        button, SIGNAL(clicked()),
                        signalMapper, SLOT(map()),
                        Qt::QueuedConnection
                    );

                // Mapping to a negative i so that it is not mixed up with
                // QDialog::Accepted and QDialog::Rejected
                ++i;
                signalMapper->setMapping(button, -i);
            }

            connect(
                    signalMapper, SIGNAL(mapped(int)),
                    &dialog, SLOT(done(int)),
                    Qt::QueuedConnection
                );

            dialog.mainWidget()->setLayout(layout);

            dialog.setMinimumSize(300, 32 * (choices.size() + 1));

            // TODO: inspect this a bit more. It doesn't seem that it shows the
            // problematic behaviour described here: http://blogs.kde.org/node/3919
            dialog.exec(); // krazy:skip

            QMetaObject::invokeMethod(receiver, slot, Qt::QueuedConnection,
                    Q_ARG(int, dialog.result()));
        }


    private:
        QString title;
        QString message;
        QStringList choices;
        QObject * receiver;
        const char * slot;
    };
};

KDialogUiHandler::KDialogUiHandler(QObject * parent, const QVariantList & args)
    : UiHandler(parent), d()
{
    Q_UNUSED(args);
}

KDialogUiHandler::~KDialogUiHandler()
{
}

void KDialogUiHandler::askPassword(const QString & title, const QString & message,
            bool newPassword, bool unlockMode, QObject * receiver, const char * slot)
{
    (new Private::AskPassword(title, message, newPassword, unlockMode, receiver, slot))->run();
}

void KDialogUiHandler::ask(const QString & title, const QString & message,
        const QStringList & choices, QObject * receiver, const char * slot)
{
    (new Private::Ask(title, message, choices, receiver, slot))->run();
}

void KDialogUiHandler::message(const QString & title, const QString & message)
{
    QMessageBox::information(nullptr, title, message);
}

void KDialogUiHandler::setBusy(bool value)
{
    Q_UNUSED(value);
}


KAMD_EXPORT_UI_HANDLER(KDialogUiHandler, "activitymanager_uihandler_kdialog")
