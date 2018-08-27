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
 ***************************************************************************/
#ifndef MEMOITEM_P_H
#define MEMOITEM_P_H

#include "iteminterface_p.h"
#include "iteminterface.h"
#include "memo.h"

SUIT_BEGIN_NAMESPACE

class TemplateRenderingStruct;

class MemoItemPrivate : public CuteReport::ItemInterfacePrivate
{
public:
    MemoItemPrivate()
        :ItemInterfacePrivate(),
          stretchMode(MemoItem::DontStretch),
          delimiters("[ , ]"),
          stretchFont(false),
          allowHtml(false),
          allowExpressions(true),
          textMargin(1,0), //mm
          lineSpacing(100),
          textIndent(0),
          textDocument(0),
          fontScale(0),
          originalHeight(-1),
          showStretchability(true)
    {
    }

    MemoItemPrivate(const MemoItemPrivate & p)
        :ItemInterfacePrivate(p),
          stretchMode(p.stretchMode),
          textFlags(p.textFlags),
          text(p.text),
          font(p.font),
          textColor(p.textColor),
          delimiters(p.delimiters),
          stretchFont(p.stretchFont),
          allowHtml(p.allowHtml),
          allowExpressions(p.allowExpressions),
          textMargin(p.textMargin),
          lineSpacing(p.lineSpacing),
          textIndent(p.textIndent),
          textDocument(0),
          fontScale(p.fontScale),
          originalHeight(p.originalHeight),
          showStretchability(p.showStretchability),
          absoluteRect(p.absoluteRect),
          //
          textPos(p.textPos)
    {
    }

    virtual ~MemoItemPrivate(){
        delete textDocument;
    }

    MemoItem::StretchMode stretchMode;
    MemoItem::TextFlags textFlags;
    QString text;
    QFont font;
    QColor textColor;
    QString delimiters;
    bool stretchFont;
    bool allowHtml;
    bool allowExpressions;
    QPointF textMargin;
    QTextDocument * textDocument;
    qreal fontScale;
    qreal originalHeight;
    bool showStretchability;
    qreal lineSpacing;
    qreal textIndent;

    // rendering
    QPointF posDeltaMM;
    QRectF absoluteRect;
    QPointF textPos;
    QRectF textClipRect;
    qreal sw;
};


QDataStream &operator<<(QDataStream &s, const MemoItemPrivate &p);
QDataStream &operator>>(QDataStream &s, MemoItemPrivate &p);


SUIT_END_NAMESPACE

#endif // MEMOITEM_P_H
