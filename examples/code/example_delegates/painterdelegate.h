#ifndef PAINTERDELEGATE_H
#define PAINTERDELEGATE_H

#include <abstractpainterdelegate.h>

class PainterDelegate : public CuteReport::AbstractPainterDelegate
{
public:
    PainterDelegate();

    virtual void paint(QPainter * painter, const QRectF &paintRect, qint16 dpi);
};

#endif // PAINTERDELEGATE_H
