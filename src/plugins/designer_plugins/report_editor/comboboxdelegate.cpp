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
 ***************************************************************************/

#include "comboboxdelegate.h"

#include <QComboBox>
#include <QWidget>
#include <QModelIndex>
#include <QApplication>
#include <QString>
#include <QDebug>

//#include <iostream>

ComboBoxDelegate::ComboBoxDelegate(QObject *parent)
    :QItemDelegate(parent)
{
}


ComboBoxDelegate::ComboBoxDelegate(const ListPairIntStr &variants, QObject *parent )
    :QItemDelegate(parent)
{
    m_items = variants;
    for(unsigned int i = 0; i < m_items.size(); ++i) {
        const PairIntStr pair = m_items.at(i);
        m_hash.insert(pair.first, pair.second);
    }
}


QWidget *ComboBoxDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &/* option */, const QModelIndex &/* index */) const
{
    QComboBox* editor = new QComboBox(parent);
    for(unsigned int i = 0; i < m_items.size(); ++i) {
        const PairIntStr pair = m_items.at(i);
        editor->addItem(pair.second, pair.first);
    }
    return editor;
}


void ComboBoxDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QComboBox *comboBox = static_cast<QComboBox*>(editor);
    int value = index.model()->data(index, Qt::DisplayRole).toInt();
    comboBox->setCurrentIndex(comboBox->findData(value));
}


void ComboBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QComboBox *comboBox = static_cast<QComboBox*>(editor);
    QVariant itemData = comboBox->itemData(comboBox->currentIndex());
//    qDebug() << comboBox->currentIndex() << itemData;
    model->setData(index, itemData, Qt::EditRole);
//    qDebug() << model->data(index, Qt::DisplayRole) << model->data(index, Qt::EditRole);
    //model->setData(index, comboBox->currentText(), Qt::DisplayRole);
}


void ComboBoxDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
    editor->setGeometry(option.rect);
}


void ComboBoxDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItemV4 myOption = option;
    int value = index.model()->data(index, Qt::DisplayRole).toInt();
    QString text = m_hash.value(value);

    myOption.text = text;

    QApplication::style()->drawControl(QStyle::CE_ItemViewItem, &myOption, painter);
}
