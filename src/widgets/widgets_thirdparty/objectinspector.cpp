/***************************************************************************
 *   This file is part of the eXaro project                                *
 *   Copyright (C) 2008 by BogDan Vatra                                    *
 *   bog_dan_ro@yahoo.com                                                  *
 **                   GNU General Public License Usage                    **
 *   Copyright (C) 2016-2017 by Mikhalov Alexander                              *
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
 *   it under the terms of the GNU Lesser General Public License as        *.
 *   published by the Free Software Foundation, either version 3 of the    *
 *   License, or (at your option) any later version.                       *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library.                                      *.
 *   If not, see <http://www.gnu.org/licenses/>.                           *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 ****************************************************************************/

#include "objectinspector.h"
#include "objectmodel.h"

#include <QDebug>
#include <QKeyEvent>

ObjectInspector::ObjectInspector(QWidget *parent) :
    QTreeView(parent)
{
	m_objectModel = new ObjectModel(this);
	setModel( m_objectModel );

	setHeaderHidden( true );
    setSelectionMode( QAbstractItemView::ExtendedSelection );

	m_selectionModel = new QItemSelectionModel( m_objectModel );
	setSelectionModel( m_selectionModel );

    connect( m_selectionModel, SIGNAL( currentChanged( const QModelIndex & , const QModelIndex & ) ), SLOT( slotObjectChanged( const QModelIndex & , const QModelIndex & ) ) );
    connect( m_selectionModel, SIGNAL( selectionChanged(QItemSelection,QItemSelection) ), this, SLOT(slotSelectionChanged(QItemSelection,QItemSelection)));
}

ObjectInspector::~ObjectInspector()
{
}


void ObjectInspector::setRootObject( QObject * rootObject)
{
    bool blocked = signalsBlocked();
    if (!blocked)
        this->blockSignals(true);
    m_objectModel->setRootObject(rootObject);
//    expand(m_objectModel->index(0,0));
    expandAll();
    this->blockSignals(blocked);
}


void  ObjectInspector::selectObject( QObject * object, QItemSelectionModel::SelectionFlag selFlag)
{
	selectObject(object, m_objectModel->index( 0, 0 ), selFlag );
}


bool ObjectInspector::selectObject( QObject * object, QModelIndex index, QItemSelectionModel::SelectionFlag selFlag)
{
	if (!object)
		return false;

	if ( index.isValid() && reinterpret_cast<ObjectModel::ObjectStruct *>( index.internalPointer() )->object == object )
	{
		this->selectionModel()->setCurrentIndex( index, selFlag );
		return true;
	}

	for ( int i = 0; i < m_objectModel->rowCount( index );i++ )	{
		if ( selectObject( object, m_objectModel->index( i, 0, index ), selFlag ) )
			return true;
	}
    return false;
}


void ObjectInspector::keyPressEvent(QKeyEvent *event)
{
    switch (event->key())
    {
        case Qt::Key_Delete:
            emit deletePressed();
            return;
    }

    QTreeView::keyPressEvent(event);
}


void ObjectInspector::slotObjectChanged( const QModelIndex & index, const QModelIndex & )
{
    if (index.isValid()) {
        ObjectModel::ObjectStruct * object = reinterpret_cast<ObjectModel::ObjectStruct *>( index.internalPointer() );
        emit objectChanged(object->object);
    }
}


void ObjectInspector::slotSelectionChanged(QItemSelection,QItemSelection)
{
    if (!signalsBlocked())
        emit selectionChanged();
}


QObjectList ObjectInspector::selectedObjects()
{
    QObjectList list;
    foreach (const QModelIndex & index, m_selectionModel->selectedIndexes())
        if ( index.isValid() ) {
            ObjectModel::ObjectStruct * objStruct = reinterpret_cast<ObjectModel::ObjectStruct *>( index.internalPointer() );
            list.append(objStruct->object);
        }
    return list;
}
