#ifndef CREATEGRADIENTDIALOG_H
#define CREATEGRADIENTDIALOG_H

#include <QDialog>
#include "gradient.h"

namespace Ui {
class CreateGradientDialog;
}

class CreateGradientDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CreateGradientDialog(QWidget *parent = 0);
    ~CreateGradientDialog();

    void setGradient(const QGradient & gradient);
    QGradient createGradient() const;

private slots:
    void addPoint();
    void delPoint();
    void chooseColor();
    void updatePreview();
    void slotCellDoubleClicked(int row, int column);

private:
    void createRow(const QString & posText = QString(), const QColor & color = QColor());
    Ui::CreateGradientDialog *ui;
};

#endif // CREATEGRADIENTDIALOG_H
