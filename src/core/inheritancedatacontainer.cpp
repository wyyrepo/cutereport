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

#include "inheritancedatacontainer.h"

namespace CuteReport
{


InheritanceDataContainer::InheritanceDataContainer()
    :m_isDirty(false), m_baseReportFound(true)
{

}


QString InheritanceDataContainer::baseReportPath() const
{
    return m_baseReportPath;
}


void InheritanceDataContainer::setBaseReportPath(const QString &value)
{
    if (m_baseReportPath == value)
        return;
    m_baseReportPath = value;
    m_isDirty = true;
}


QString InheritanceDataContainer::baseReportFileName() const
{
    return m_baseReportFileName;
}


void InheritanceDataContainer::setBaseReportFileName(const QString &value)
{
    if (m_baseReportFileName == value)
        return;
    m_baseReportFileName = value;
    m_isDirty = true;
}


QString InheritanceDataContainer::baseReportName() const
{
    return m_baseReportName;
}


void InheritanceDataContainer::setBaseReportName(const QString &value)
{
    if (m_baseReportName == value)
        return;
    m_baseReportName = value;
    m_isDirty = true;
}


bool InheritanceDataContainer::baseReportFound() const
{
    return m_baseReportFound;
}


void InheritanceDataContainer::setBaseReportFound(bool value)
{
    if (m_baseReportFound == value)
        return;
    m_baseReportFound = value;
    m_isDirty = true;
}


bool InheritanceDataContainer::isDirty() const
{
    return m_isDirty;
}


void InheritanceDataContainer::setIsDirty(bool isDirty)
{
    m_isDirty = isDirty;
}


QVariantHash InheritanceDataContainer::toHash(bool includeEmptyFields) const
{
    QVariantHash data;
    if (includeEmptyFields || !baseReportName().isEmpty())
        data.insert("baseReportName", baseReportName());
    if (includeEmptyFields || !baseReportPath().isEmpty())
        data.insert("baseReportPath", baseReportPath());
    if (includeEmptyFields || !baseReportFileName().isEmpty())
        data.insert("baseReportFileName", baseReportFileName());
    return data;
}


void InheritanceDataContainer::fromHash(const QVariantHash &hash)
{
    setBaseReportName(hash.value("baseReportName").toString());
    setBaseReportPath(hash.value("baseReportPath").toString());
    setBaseReportFileName(hash.value("baseReportFileName").toString());
}


void CuteReport::InheritanceDataContainer::copyFrom(const CuteReport::InheritanceDataContainer &data)
{
    setBaseReportName(data.baseReportName());
    setBaseReportPath(data.baseReportPath());
    setBaseReportFileName(data.baseReportFileName());
}

} //namespace
