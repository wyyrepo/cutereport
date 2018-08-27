#include "creategradientdialog.h"
#include "ui_creategradientdialog.h"

#include <QColorDialog>
#include <QTableWidgetItem>
#include <QGradient>

CreateGradientDialog::CreateGradientDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CreateGradientDialog)
{
    ui->setupUi(this);

    connect(ui->bAddPoint, SIGNAL(clicked(bool)), SLOT(addPoint()));
    connect(ui->bDeletePoint, SIGNAL(clicked(bool)), SLOT(delPoint()));
    connect(ui->bChooseColor, SIGNAL(clicked(bool)), SLOT(chooseColor()));
    connect(ui->tableWidget, SIGNAL(cellChanged(int,int)), this, SLOT(updatePreview()));
    connect(ui->tableWidget, SIGNAL(cellDoubleClicked(int,int)), this, SLOT(slotCellDoubleClicked(int,int)));
}


CreateGradientDialog::~CreateGradientDialog()
{
    delete ui;
}


void CreateGradientDialog::setGradient(const QGradient &gradient)
{
    ui->tableWidget->clearContents();
    if (gradient.type() == QGradient::NoGradient)
        return;
    foreach (const QGradientStop &stop, gradient.stops()) {
        createRow(QString::number(stop.first), stop.second );
    }
}


QGradient CreateGradientDialog::createGradient() const
{
    QGradient::Type type = QGradient::LinearGradient;

    QGradient gradient;
    switch (type) {
        case QGradient::LinearGradient:
            gradient = QLinearGradient(0,0,0,1);
            break;
        default:
            break;
    }

    gradient.setCoordinateMode(QGradient::ObjectBoundingMode);

    for (int i=0; i<ui->tableWidget->rowCount(); ++i) {
        QTableWidgetItem * itemPos = ui->tableWidget->item(i, 0);
        QTableWidgetItem * itemColor = ui->tableWidget->item(i, 1);
        if (!itemPos || !itemColor)
            continue;
        qreal pos = itemPos->text().toDouble();
        QColor color = itemColor->backgroundColor();
        gradient.setColorAt(pos, color);
    }

    return gradient;
}


void CreateGradientDialog::addPoint()
{
    createRow();
}


void CreateGradientDialog::delPoint()
{
    ui->tableWidget->removeRow(ui->tableWidget->currentRow());
}


void CreateGradientDialog::chooseColor()
{
    int row = ui->tableWidget->currentRow();
    if (row < 0)
        return;

    QTableWidgetItem * itemColor =  ui->tableWidget->item(row, 1);

    QColor color = QColorDialog::getColor(itemColor ? itemColor->backgroundColor() : Qt::white);

    if (color.isValid()) {
        if (!itemColor) {
            itemColor = new QTableWidgetItem();
            ui->tableWidget->setItem(row, 1, itemColor);
        }
        itemColor->setText(color.name());
        itemColor->setBackgroundColor(color);
    }
}


void CreateGradientDialog::updatePreview()
{
    QGradient gradient = createGradient();
    QPalette palette = ui->preview->palette();
    palette.setBrush(QPalette::Window, gradient);
    ui->preview->setPalette(palette);
}


void CreateGradientDialog::slotCellDoubleClicked(int row, int column)
{
    Q_UNUSED(row);
    if (column == 0)
        return;
    chooseColor();
}


void CreateGradientDialog::createRow(const QString &posText, const QColor &color)
{
    QString pText = posText.isEmpty() ? (ui->tableWidget->rowCount() == 0 ? "0" : "1") : posText;
    ui->tableWidget->insertRow(ui->tableWidget->rowCount());
    int row = ui->tableWidget->rowCount() -1;
    QTableWidgetItem * itemPos = new QTableWidgetItem(pText);
    QTableWidgetItem * itemColor = new QTableWidgetItem(color.name());
    itemColor->setBackgroundColor(color);
    itemColor->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    ui->tableWidget->setItem(row, 0, itemPos);
    ui->tableWidget->setItem(row, 1, itemColor);
}
