#include "proxy_item_delegate.hpp"

#include <QApplication>
#include <QEvent>
#include <QMouseEvent>
#include <QTableView>

ProxyItemDelegate::ProxyItemDelegate(QAbstractItemDelegate * delegate, QObject * parent)
  : TBase(parent)
  , m_delegate(delegate)
{
  Q_ASSERT(m_delegate != nullptr);
}

void ProxyItemDelegate::paint(QPainter * painter, QStyleOptionViewItem const & option, QModelIndex const & index) const
{
  m_delegate->paint(painter, option, index);
}

QSize ProxyItemDelegate::sizeHint(QStyleOptionViewItem const & option, QModelIndex const & index) const
{
  return m_delegate->sizeHint(option, index);
}

QWidget * ProxyItemDelegate::createEditor(QWidget * parent, QStyleOptionViewItem const & option, QModelIndex const & index) const
{
  return m_delegate->createEditor(parent, option, index);
}

void ProxyItemDelegate::destroyEditor(QWidget * editor, QModelIndex const & index) const
{
  m_delegate->destroyEditor(editor, index);
}

void ProxyItemDelegate::setEditorData(QWidget * editor, QModelIndex const & index) const
{
  m_delegate->setEditorData(editor, index);
}

void ProxyItemDelegate::setModelData(QWidget * editor, QAbstractItemModel * model, QModelIndex const & index) const
{
  m_delegate->setModelData(editor, model, index);
}

void ProxyItemDelegate::updateEditorGeometry(QWidget * editor, QStyleOptionViewItem const & option, QModelIndex const & index) const
{
  m_delegate->updateEditorGeometry(editor, option, index);
}

bool ProxyItemDelegate::editorEvent(QEvent * event, QAbstractItemModel * model, QStyleOptionViewItem const & option, QModelIndex const & index)
{
  return m_delegate->editorEvent(event, model, option, index);
}

bool ProxyItemDelegate::helpEvent(QHelpEvent * event, QAbstractItemView * view, QStyleOptionViewItem const & option, QModelIndex const & index)
{
  return m_delegate->helpEvent(event, view, option, index);
}

QVector<int> ProxyItemDelegate::paintingRoles() const
{
  return m_delegate->paintingRoles();
}

