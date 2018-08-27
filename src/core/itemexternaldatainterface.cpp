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
#include "itemexternaldatainterface.h"

namespace CuteReport
{

ItemExternalDataInterface::ItemExternalDataInterface(QObject * parent) :
    QObject(parent)
{
    initMe();
}


ItemExternalDataInterface::~ItemExternalDataInterface()
{
}


void ItemExternalDataInterface::initMe()
{
    m_currentProperty = -1;
}


QByteArray ItemExternalDataInterface::serialize()
{
    return QByteArray();
}


void ItemExternalDataInterface::deserialize(QByteArray &data)
{
    Q_UNUSED(data);
}


QString ItemExternalDataInterface::_current_property_description() const
{
    return QString();
}


int ItemExternalDataInterface::_current_property_precision() const
{
    return 1;
}


} //namespace
