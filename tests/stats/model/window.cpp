/*
 *   Copyright (C) 2015 Ivan Cukic <ivan.cukic(at)kde.org>
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

#include "window.h"

#include "ui_window.h"

#include <QListView>
#include <QDebug>
#include <QCoreApplication>
#include <QItemDelegate>
#include <QPainter>
#include <QDateTime>

#include <resultset.h>
#include <resultmodel.h>
#include <consumer.h>

#include <boost/range/numeric.hpp>

namespace KAStats = KActivities::Experimental::Stats;

using namespace KAStats;
using namespace KAStats::Terms;

class Delegate: public QItemDelegate {
public:
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const Q_DECL_OVERRIDE
    {
        painter->save();

        const QString title = index.data(ResultModel::TitleRole).toString();

        QRect titleRect = painter->fontMetrics().boundingRect(title);
        int lineHeight = titleRect.height();

        // Header background
        titleRect.moveTop(option.rect.top());
        titleRect.setWidth(option.rect.width());

        painter->fillRect(titleRect.x(), titleRect.y(),
                          titleRect.width(), titleRect.height() + 16,
                          QColor(32, 32, 32));

        // Painting the title
        painter->setPen(QColor(255,255,255));
        int top = 2 * lineHeight;

        titleRect.moveTop(titleRect.top() + 8);
        titleRect.setLeft(8);
        titleRect.setWidth(titleRect.width() - 8);
        painter->drawText(titleRect, index.data(ResultModel::TitleRole).toString());

        // Painting the score
        painter->drawText(titleRect,
                          "Score: " + QString::number(index.data(ResultModel::ScoreRole).toDouble()),
                          QTextOption(Qt::AlignRight));

        // Painting the moification and creation times
        titleRect.moveTop(titleRect.bottom() + 16);

        painter->fillRect(titleRect.x() - 4, titleRect.y() - 8,
                          titleRect.width() + 8, titleRect.height() + 8 + lineHeight,
                          QColor(64, 64, 64));

        titleRect.moveTop(titleRect.top() - 4);

        painter->drawText(titleRect,
                          index.data(ResultModel::ResourceRole).toString()
                          );

        auto firstUpdate = QDateTime::fromTime_t(index.data(ResultModel::FirstUpdateRole).toUInt());
        auto lastUpdate = QDateTime::fromTime_t(index.data(ResultModel::LastUpdateRole).toUInt());

        titleRect.moveTop(titleRect.top() + lineHeight);

        painter->drawText(titleRect,
                          "Modified: " + lastUpdate.toString()
                          );
        painter->drawText(titleRect,
                          "Created: " + firstUpdate.toString(),
                          QTextOption(Qt::AlignRight));

        painter->restore();

    }

    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const Q_DECL_OVERRIDE
    {
        return QSize(0, 100);
    }
};

Window::Window()
    : ui(new Ui::MainWindow())
    , model(Q_NULLPTR)
    , activities(new KActivities::Consumer())
{
    ui->setupUi(this);
    ui->viewResults->setItemDelegate(new Delegate());
    // ui->viewResults->setUniformItemSizes(true);
    // ui->viewResults->setGridSize(QSize(200, 100));

    while (activities->serviceStatus() == KActivities::Consumer::Unknown) {
        QCoreApplication::processEvents();
    }

    connect(ui->buttonUpdate, SIGNAL(clicked()),
            this, SLOT(updateResults()));

    for (const auto &activity :
         (QStringList() << ":current"
                        << ":any"
                        << ":global") + activities->activities()) {
        ui->comboActivity->addItem(activity);
    }

    // ResultSet results(UsedResources | Agent{"gvim"});
    //
    // int count = 20;
    // for (const auto& result: results) {
    //     qDebug() << "Result:" << result.title << result.resource;
    //     if (count -- == 0) break;
    // }
    //
    // ResultModel model(UsedResources | Agent{"gvim"});
    // model.setItemCountLimit(50);
    //
    // QListView view;
    // view.setModel(&model);
    //
    // view.show();
}

Window::~Window()
{
    delete ui;
    delete model;
    delete activities;
}

void Window::updateResults()
{
    qDebug() << "Updating the results";

    ui->viewResults->setModel(Q_NULLPTR);
    delete model;

    auto query =
        // What should we get
        (
            ui->radioSelectUsedResources->isChecked()   ? UsedResources :
            ui->radioSelectLinkedResources->isChecked() ? LinkedResources :
                                                          AllResources
        ) |

        // How we should order it
        (
            ui->radioOrderHighScoredFirst->isChecked()      ? HighScoredFirst :
            ui->radioOrderRecentlyUsedFirst->isChecked()    ? RecentlyUsedFirst :
            ui->radioOrderRecentlyCreatedFirst->isChecked() ? RecentlyCreatedFirst :
            ui->radioOrderByUrl->isChecked()                ? OrderByUrl :
                                                              OrderByTitle
        ) |

        // Which agents?
        Agent(ui->textAgent->text().split(',')) |

        // Which mime?
        Type(ui->textMimetype->text().split(',')) |

        // Which activities?
        Activity(ui->comboActivity->currentText());

    // Log results
    using boost::accumulate;

    ui->textLog->setText(
        accumulate(ResultSet(query), QString(),
            [] (const QString &acc, const ResultSet::Result &result) {
                return acc + result.title() + " (" + result.resource() + ")\n";
            })
        );

    model = new ResultModel(query);

    qDebug() << "Limit" << ui->spinLimitCount->value();
    model->setItemCountLimit(ui->spinLimitCount->value());
    ui->viewResults->setModel(model);
}

