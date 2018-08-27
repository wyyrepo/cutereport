#ifndef OPTIONSDIALOGDESIGNER_H
#define OPTIONSDIALOGDESIGNER_H

#include <optionsdialog.h>

namespace Ui {
class OptionsDialogDesigner;
}

class OptionsDialogDesigner : public OptionsDialogPageInterface
{
    Q_OBJECT

public:
    explicit OptionsDialogDesigner(CuteDesigner::Core *core);
    ~OptionsDialogDesigner();

    virtual void activate();
    virtual void deactivate();

    virtual QListWidgetItem * createButton(QListWidget * listWidget);

private:
    Ui::OptionsDialogDesigner *ui;
    CuteDesigner::Core *m_core;
};

#endif // OPTIONSDIALOGDESIGNER_H
