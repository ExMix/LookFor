#pragma once

#include "file_system_model.hpp"

#include <QMainWindow>
#include <QSortFilterProxyModel>

namespace Ui
{

class MainWindow;

} // namespace Ui

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = 0);
  ~MainWindow();

private:
  void LoadState();
  void SaveState();

private:
  Q_SLOT void onRootDialogCall();
  Q_SLOT void onRootSpecified();

  Q_SLOT void onTreeSelectionChanged(QItemSelection const & selected, QItemSelection const & deselected);
  Q_SLOT void onTableSelectionChanged(QItemSelection const & selected, QItemSelection const & deselected);

  Q_SLOT void onResizeColumns();
  Q_SLOT void onSetRegExp();

private:
  Ui::MainWindow * m_ui;

  FileSystemModel * m_fileModel;
  QSortFilterProxyModel * m_model;

  bool m_ignoreTableSelection;
};
