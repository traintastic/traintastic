#ifndef ADDTRAINDIALOG_H
#define ADDTRAINDIALOG_H

#include <QDialog>

class QLabel;
class QLineEdit;
class QSpinBox;

class TrainsModel;

class AddTrainDialog : public QDialog
{
    Q_OBJECT
public:
    AddTrainDialog(size_t segmentIndex, const QString &segName,
                   TrainsModel *trainsModel,
                   QWidget *parent = nullptr);

protected:
    void done(int result);

private:
    QLabel *mLabel;
    QLineEdit *mTrainEdit;
    QSpinBox *mNumWagonsSpin;

    TrainsModel *mTrainsModel = nullptr;
    size_t mSegmentIndex = 0;
};

#endif // ADDTRAINDIALOG_H
