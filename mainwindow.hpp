#pragma once

#include "file_system_model.hpp"

#include <QMainWindow>
#include <QItemSelectionModel>

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
  Q_SLOT void onRootDialogCall();
  Q_SLOT void onRootSpecified();

  Q_SLOT void onSelectionChanged(QItemSelection const & selected, QItemSelection const & deselected);

private:
  Ui::MainWindow * ui;

  FileSystemModel * m_model;
  QItemSelectionModel * m_selectionModel;
};
