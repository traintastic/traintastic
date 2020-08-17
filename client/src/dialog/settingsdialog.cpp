#include "settingsdialog.hpp"
#include <QFormLayout>
#include <QComboBox>
#include <QDirIterator>
#include <traintastic/locale/locale.hpp>
#include "../utils/getlocalepath.hpp"

static QString getLanguageName(const QString& filename)
{
  QFile file(filename);
  if(file.open(QIODevice::ReadOnly | QIODevice::Text))
  {
    const QString header = QStringLiteral("## Traintastic language file: ");
    QString line = QString::fromUtf8(file.readLine());
    if(line.startsWith(header))
      return line.remove(0, header.length()).trimmed();
  }
  return "";
}

SettingsDialog::SettingsDialog(QWidget* parent) :
  QDialog(parent, Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint)
{
  setWindowTitle(Locale::tr("qtapp.mainmenu:settings"));

  QFormLayout* form = new QFormLayout();

  // language:
  QComboBox* cb = new QComboBox(this);
  QDirIterator it(getLocalePath(), {"*.txt"}, QDir::Files | QDir::Readable);
  while(it.hasNext())
  {
    const QString filename = it.next();
    const QString label = getLanguageName(filename);
    if(!label.isEmpty())
    {
      cb->addItem(label, filename);
      if(filename.toStdString() == Locale::instance->filename)
        cb->setCurrentIndex(cb->count() - 1);
    }
  }
  form->addRow(Locale::tr("qtapp.settings:language"), cb);

  setLayout(form);
}
