#ifndef HELPEROBJECT_H
#define HELPEROBJECT_H

#include <QObject>

class HelperObject : public QObject
{
    Q_OBJECT
public:
    explicit HelperObject(QObject * reportObject, QObject *parent = 0);
signals:

public slots:
    void slotMemoBeforePrint();
private:
    QObject * m_reportObject;
};

#endif // HELPEROBJECT_H
