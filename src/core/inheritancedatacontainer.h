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

#ifndef INHERITANCEDATACONTAINER_H
#define INHERITANCEDATACONTAINER_H

#include <QString>
#include <QVariantHash>
#include "cutereport_globals.h"

namespace CuteReport
{

class CUTEREPORT_EXPORTS InheritanceDataContainer
{
public:
    InheritanceDataContainer();

    QString baseReportPath() const;
    void setBaseReportPath(const QString &value);

    QString baseReportFileName() const;
    void setBaseReportFileName(const QString &value);

    QString baseReportName() const;
    void setBaseReportName(const QString &value);

    bool baseReportFound() const;
    void setBaseReportFound(bool value);

    bool isDirty() const;
    void setIsDirty(bool isDirty);

    QVariantHash toHash(bool includeEmptyFields = true) const;
    void fromHash(const QVariantHash & hash);

    void copyFrom(const InheritanceDataContainer & data);

private:
    QString m_baseReportPath;
    QString m_baseReportFileName;
    QString m_baseReportName;
    bool m_isDirty;
    bool m_baseReportFound;
    /// not used yet
    /// QHash<QString, QHash<QString, QVariant> > itemData; //itemName, <propertyName, changedValue>
//    QHash<QString, QHash<QString, QPair<QVariant, QVariant> > > itemData; //itemName, <propertyName, pair(changedValue, origValue) >
};

} //namespace
#endif // INHERITANCEDATACONTAINER_H
