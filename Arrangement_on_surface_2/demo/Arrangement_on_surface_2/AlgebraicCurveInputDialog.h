#ifndef ALGEBRAICCURVEINPUTDIALOG_H
#define ALGEBRAICCURVEINPUTDIALOG_H

#include <QDialog>

namespace Ui {
class AlgebraicCurveInputDialog;
}

class AlgebraicCurveInputDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AlgebraicCurveInputDialog(QWidget *parent = 0);
    ~AlgebraicCurveInputDialog();

private:
    Ui::AlgebraicCurveInputDialog *ui;
};

#endif // ALGEBRAICCURVEINPUTDIALOG_H
