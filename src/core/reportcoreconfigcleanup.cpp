/***************************************************************************
 *   This file is part of the CuteReport project                           *
 *   Copyright (C) 2012-2017 by Alexander Mikhalov                         *
 *   alexander.mikhalov@gmail.com                                          *
 *                                                                         *
 **                   GNU General Public License Usage                    **
 *                                                                         *
 *   This library is free software: you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation, either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 *                                                                         *
 **                  GNU Lesser General Public License                    **
 *                                                                         *
 *   This library is free software: you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License as        *
 *   published by the Free Software Foundation, either version 3 of the    *
 *   License, or (at your option) any later version.                       *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library.                                      *
 *   If not, see <http://www.gnu.org/licenses/>.                           *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 ****************************************************************************/

#include "reportcoreconfigcleanup.h"
#include <QSettings>
#include "reportcore.h"

namespace CuteReport {


void ReportCoreConfigCleanUp::process(QSettings *settings, CuteReport::ReportCore * reportCore)
{
    int configVersionMajor = settings->value("CuteReport/VersionMajor", -1).toInt();
    int configVersionMinor = settings->value("CuteReport/VersionMinor", -1).toInt();
    int reportVersionMajor = reportCore->versionMajor();
    int reportVersionMinor = reportCore->versionMinor();


    if (reportVersionMajor == 1 && reportVersionMinor == 3 && reportVersionMajor >= configVersionMajor && reportVersionMinor > configVersionMinor) {
        version_1_3(settings);
    }

    settings->setValue("CuteReport/VersionMajor", reportVersionMajor);
    settings->setValue("CuteReport/VersionMinor", reportVersionMinor);
}


void ReportCoreConfigCleanUp::version_1_3(QSettings *settings)
{
    /// set loading method to OS native dialog

    settings->setValue("CuteReport/ReportLoadingMethod", 1);

    /// switch second level fancytabwidget to upper tabs

    settings->setValue("CuteReport_ReportEditor/tabMode", 3);
    settings->setValue("CuteReport_PageEditor/tabMode", 3);
    settings->setValue("CuteReport_DatasetEditor/tabMode", 3);
    settings->setValue("CuteReport_FormEditor/tabMode", 3);

    /// error in prev verions where was not default value set, so it defaulted to 0

    settings->setValue("CuteReport_ReportEditor/prevReportsLimit", 7);

    settings->setValue("CuteReport_PageEditor/splitterState", QVariant());
    settings->setValue("CuteReport_PageEditor/splitter2State", QVariant());
}


}// namespace
