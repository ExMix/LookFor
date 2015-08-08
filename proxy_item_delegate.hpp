#pragma once

#include <QAbstractItemDelegate>

class ProxyItemDelegate : public QAbstractItemDelegate
{
  using TBase = QAbstractItemDelegate;

public:
  ProxyItemDelegate(QAbstractItemDelegate * delegate, QObject * parent);

  void paint(QPainter * painter, QStyleOptionViewItem const & option, QModelIndex const & index) const;
  QSize sizeHint(QStyleOptionViewItem const & option, QModelIndex const & index) const;

  QWidget * createEditor(QWidget * parent, QStyleOptionViewItem const & option, QModelIndex const & index) const;
  void destroyEditor(QWidget * editor, QModelIndex const & index) const;
  void setEditorData(QWidget * editor, QModelIndex const & index) const;
  void setModelData(QWidget * editor, QAbstractItemModel * model, QModelIndex const & index) const;
  void updateEditorGeometry(QWidget * editor, QStyleOptionViewItem const & option, QModelIndex const & index) const;
  bool editorEvent(QEvent * event, QAbstractItemModel * model, QStyleOptionViewItem const & option, QModelIndex const & index);
  bool helpEvent(QHelpEvent * event, QAbstractItemView * view, QStyleOptionViewItem const & option, QModelIndex const & index);
  QVector<int> paintingRoles() const;

private:
  QAbstractItemDelegate * m_delegate;

  void initStyleOption(QStyleOptionViewItem * option, const QModelIndex & index) const;
};
