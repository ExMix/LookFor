#pragma once

#include <QDialog>
#include <QRegExp>

namespace Ui
{

class RegExpDialog;

} //namespace Ui

class RegExpDialog : public QDialog
{
  using TBase = QDialog;
public:
  RegExpDialog(QRegExp const & initExp, QWidget * parent);
  ~RegExpDialog();

  QRegExp const & GetRegExp() const;

private:
  Q_SLOT void regExpChanged();
  Q_SLOT void tryTextChanged();

  void updateResult();

private:
  Ui::RegExpDialog * m_ui;
  QRegExp m_regExp;
};
