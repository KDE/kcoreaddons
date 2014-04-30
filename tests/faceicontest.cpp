/*
 *  Copyright (C) 2014 Nicolás Alvarez <nicolas.alvarez@gmail.com>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation;
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#include "faceicontest.h"

#include <QApplication>
#include <QVBoxLayout>
#include <QListWidget>
#include <QDebug>
#include <kuser.h>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    FaceIconTest *mainWin = new FaceIconTest();
    mainWin->show();
    return app.exec();
}
FaceIconTest::FaceIconTest()
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    listWidget = new QListWidget(this);
    layout->addWidget(listWidget);

    QList<KUser> users = KUser::allUsers();
    Q_FOREACH (const KUser &u, users) {
        QPixmap pixmap(u.faceIconPath());
        if (pixmap.isNull()) {
            pixmap = QPixmap(QSize(48, 48));
            pixmap.fill();
        } else {
            pixmap = pixmap.scaled(48, 48, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        }

        QListWidgetItem *item = new QListWidgetItem(u.loginName(), listWidget);
        item->setData(Qt::DecorationRole, pixmap);
    }
}
