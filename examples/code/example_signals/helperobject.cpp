#include "helperobject.h"
#include <QDebug>
#include <CuteReport>

HelperObject::HelperObject(QObject *reportObject, QObject *parent) : QObject(parent)
  ,m_reportObject(reportObject)
{

}

void HelperObject::slotMemoBeforePrint()
{
    CuteReport::BaseItemInterface * item = qobject_cast<CuteReport::BaseItemInterface *>(m_reportObject);
    if (!item)
        return;

    int r = qrand() % 255;
    int g = qrand() % 255;
    int b = qrand() % 255;
    item->setProperty("backgroundBrush", QColor(r,g,b));
}
