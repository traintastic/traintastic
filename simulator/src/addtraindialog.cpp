#include "addtraindialog.h"

#include <QSpinBox>
#include <QLineEdit>
#include <QLabel>

#include <QFormLayout>

#include <QDialogButtonBox>

#include <QMessageBox>

#include "trainsmodel.h"

AddTrainDialog::AddTrainDialog(size_t segmentIndex, const QString& segName, TrainsModel *trainsModel, QWidget *parent)
    : QDialog{parent}
    , mTrainsModel(trainsModel)
    , mSegmentIndex(segmentIndex)
{
    QFormLayout *lay = new QFormLayout(this);

    mLabel = new QLabel;
    lay->addRow(mLabel);

    mTrainEdit = new QLineEdit;
    lay->addRow(tr("Train name:"), mTrainEdit);

    mNumWagonsSpin = new QSpinBox;
    mNumWagonsSpin->setRange(1, 100);
    mNumWagonsSpin->setValue(5);
    lay->addRow(tr("Num wagons:"), mNumWagonsSpin);

    QDialogButtonBox *butBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                                                    Qt::Horizontal);
    connect(butBox, &QDialogButtonBox::accepted,
            this, &QDialog::accept);
    connect(butBox, &QDialogButtonBox::rejected,
            this, &QDialog::reject);
    lay->addRow(butBox);

    QString segName_ = segName;
    if(segName_.isEmpty())
        segName_ = tr("<i>(index %1)</i>").arg(segmentIndex);

    mLabel->setText(tr("Add Train on segment %1")
                    .arg(segName_));

    setWindowTitle(tr("Add Train"));
}

void AddTrainDialog::done(int result)
{
    if(result == QDialog::Accepted)
    {
        QString errStr;
        if(!mTrainsModel->addTrain(mTrainEdit->text(),
                               Color::Blue, mNumWagonsSpin->value(),
                               mSegmentIndex, &errStr))
        {
            QMessageBox::warning(this, tr("Cannot add Train"),
                                 errStr);
            return;
        }
    }

    QDialog::done(result);
}
