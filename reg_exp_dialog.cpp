#include "reg_exp_dialog.hpp"
#include "ui_regexpdialog.h"

#include "macros.hpp"

RegExpDialog::RegExpDialog(QRegExp const & initExp, QWidget * parent)
  : TBase(parent)
  , m_ui(new Ui::RegExpDialog)
  , m_regExp(initExp)
{
  m_ui->setupUi(this);
  setModal(true);

  VERIFY(QObject::connect(m_ui->m_regExpEditor, &QLineEdit::textChanged,
                          this, &RegExpDialog::regExpChanged));
  VERIFY(QObject::connect(m_ui->m_tryText, &QLineEdit::textChanged,
                          this, &RegExpDialog::tryTextChanged));

  m_regExp.setPatternSyntax(QRegExp::FixedString);
  m_regExp.setCaseSensitivity(Qt::CaseInsensitive);

  m_ui->m_regExpEditor->setText(m_regExp.pattern());
}

RegExpDialog::~RegExpDialog()
{
  delete m_ui;
}

QRegExp const & RegExpDialog::GetRegExp() const
{
  return m_regExp;
}

void RegExpDialog::regExpChanged()
{
  m_regExp.setPattern(m_ui->m_regExpEditor->text());
  updateResult();
}

void RegExpDialog::tryTextChanged()
{
  updateResult();
}

void RegExpDialog::updateResult()
{
  if (!m_regExp.isValid())
    m_ui->m_regExpEditor->setStyleSheet(QStringLiteral("color:red"));
  else
  {
    m_ui->m_regExpEditor->setStyleSheet("");
    if (m_ui->m_tryText->text().contains(m_regExp))
      m_ui->m_tryText->setStyleSheet(QStringLiteral("color:green"));
    else
      m_ui->m_tryText->setStyleSheet("");
  }
}

