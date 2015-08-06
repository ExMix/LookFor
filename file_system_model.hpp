#pragma once

#include <QAbstractItemModel>

class FileSystemModel : public QAbstractItemModel
{
  using TBase = QAbstractItemModel;
public:
  FileSystemModel(QObject * parent = 0);
  ~FileSystemModel();

  void setRoot(QString const & rootPath);

  int rowCount(QModelIndex const & parent) const override;
  int columnCount(QModelIndex const & parent) const override;

  QModelIndex index(int row, int column, QModelIndex const & parent) const override;
  QModelIndex parent(QModelIndex const & child) const override;
  QVariant data(QModelIndex const & index, int role) const override;
  bool setData(QModelIndex const & index, QVariant const & value, int role) override;
  QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
  Qt::ItemFlags flags(QModelIndex const & index) const override;

  Q_SIGNAL void subtreeSelected(QModelIndex const & index);

private:
  struct Impl;
  std::unique_ptr<Impl> m_impl;
};
