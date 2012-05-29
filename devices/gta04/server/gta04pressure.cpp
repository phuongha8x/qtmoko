/****************************************************************************
**
** This file is part of the Qt Extended Opensource Package.
**
** Copyright (C) 2009 Trolltech ASA.
** Copyright (C) 2012 Radek Polak.
**
** Contact: Qt Extended Information (info@qtextended.org)
**
** This file may be used under the terms of the GNU General Public License
** version 2.0 as published by the Free Software Foundation and appearing
** in the file LICENSE.GPL included in the packaging of this file.
**
** Please review the following information to ensure GNU General Public
** Licensing requirements will be met:
**     http://www.fsf.org/licensing/licenses/info/GPLv2.html.
**
**
****************************************************************************/

#include "gta04pressure.h"

#include <QSocketNotifier>
#include <QTimer>
#include <QLabel>
#include <QDesktopWidget>
#include <QProcess>
#include <QValueSpaceObject>
#include <QValueSpaceItem>

#include <qcontentset.h>
#include <qtopiaapplication.h>
#include <qtopialog.h>
#include <qtopiaipcadaptor.h>

#include <qbootsourceaccessory.h>
#include <qtopiaipcenvelope.h>

#include <qtopiaserverapplication.h>

#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <linux/input.h>

#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <linux/types.h>
#include <linux/netlink.h>
#include <QTcpSocket>
#include <QtDebug>

QTOPIA_TASK(Gta04Pressure, Gta04Pressure);

static QByteArray readFile(const char *path)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
        qLog(Hardware) << "file open failed" << path << ":" <<
            f.errorString();
        return QByteArray();
    }
    QByteArray content = f.readAll();
    f.close();
    return content;
}

Gta04Pressure::Gta04Pressure()
:
timer(this)
{
    qLog(Hardware) << "gta04 pressure";

    QProcess::execute("sh",
		      QStringList()
		      <<
		      "-c"
		      <<
		      "echo bmp085 0x77 > /sys/bus/i2c/devices/i2c-2/new_device");

    pressure_space = new QValueSpaceObject("/UI/Pressure", this);

    // Arrange to update pressure every 10 minutes.
    connect(&timer, SIGNAL(timeout()), this, SLOT(updateStatus()));
    timer.start(10 * 60 * 1000);

    // Also take an initial pressure measurement right now. 
    QTimer::singleShot(1, this, SLOT(updateStatus()));
}

Gta04Pressure::~Gta04Pressure()
{
    delete pressure_space;
}

void Gta04Pressure::updateStatus()
{
    QString pressureStr = readFile(
        "/sys/devices/platform/omap/omap_i2c.2/i2c-2/2-0077/pressure0_input");

    pressureStr = pressureStr.trimmed();
    pressure_space->setAttribute("Pa", pressureStr);
    pressure_space->setAttribute("mb",
				 QString::number((pressureStr.toInt() + 50) / 100));
}
