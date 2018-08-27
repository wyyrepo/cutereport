#include "painterdelegate.h"
#include <QPainter>

PainterDelegate::PainterDelegate():
    CuteReport::AbstractPainterDelegate(0)
{

}


void PainterDelegate::paint(QPainter *painter, const QRectF &paintRect, qint16 dpi)
{
    QFont f;
    qreal pixels = f.pointSizeF()/72*dpi;
    f.setPixelSize(pixels);
    painter->setFont(f);
    painter->drawText(paintRect, Qt::AlignCenter | Qt::TextWordWrap, "Delegate Text");
}
