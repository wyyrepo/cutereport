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
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 ****************************************************************************/

#include "aboutdialog.h"

#include <QDateTime>
#ifdef PRO
    #include "../../../addons_pro/plugins/common/functions_ext.h"
#endif

#if QT_VERSION < 0x050000
AboutDialog::AboutDialog(QWidget* parent, Qt::WFlags fl): QDialog(parent, fl), Ui::aboutDialog()
#else
AboutDialog::AboutDialog(QWidget* parent, Qt::WindowFlags fl): QDialog(parent, fl), Ui::aboutDialog()
#endif
{
	setupUi(this);

    labelVersion->setText(REPORT_VERSION);
    labelLibraryLicense->setText(REPORT_LICENSE);
    labelDesignerLicense->setText(DESIGNER_LICENSE);
    labelBuildDate->setText(QString::fromUtf8(BUILD_DATETIME));
    labelQtVersion->setText(QT_VERSION_STR);
#ifdef STATICBUILD
    labelBuildType->setText(QString("Static (%1, %2)").arg(COMPILER_VER).arg(PROCESSOR_TYPE));
#else
    labelBuildType->setText(QString("Dynamic (%1, %2)").arg(COMPILER_VER).arg(PROCESSOR_TYPE));
#endif

#ifdef PRO
    CuteReportExt::fillLicenseeName(labelLicensee);
    CuteReportExt::fillIssueDate(labelLicenseIssued);
    CuteReportExt::fillExpirationDate(labelLicenseExpires);
#else
    labelLicenseIssuedTitle->hide();
    labelLicenseIssued->hide();
    labelLicenseExpiresTitle->hide();
    labelLicenseExpires->hide();
    labelRegisteredTo->hide();
    labelLicensee->hide();
#endif

#if !defined(GPL) && !defined(LGPL)
    delete tabLicense;
#endif

    tabWidget->setCurrentIndex(0);
}

AboutDialog::~AboutDialog()
{
}



