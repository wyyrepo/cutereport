/***************************************************************************
 *   This file is part of the propertyEditor project                       *
 *   Copyright (C) 2008 by BogDan Vatra                                    *
 *   bog_dan_ro@yahoo.com                                                  *
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

#include <QColorDialog>

#include "changegradient.h"
#include "creategradientdialog.h"


ChangeGradient::ChangeGradient(QWidget *parent)
		: QPushButton(parent)
{
    connect(this, SIGNAL(pressed()), this, SLOT(onClick()));
    setText(tr("Change gradient"));
}


void ChangeGradient::onClick()
{
    CreateGradientDialog d(this);
    d.setGradient(m_gradient);
    if (d.exec()) {
        m_gradient = d.createGradient();
        emit(gradientChanged(m_gradient));
    }
}


void ChangeGradient::setGradient(const QGradient &gradient)
{
    m_gradient = gradient;
}


QGradient ChangeGradient::gradient()
{
    return m_gradient;
}
