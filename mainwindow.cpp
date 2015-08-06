#include "mainwindow.hpp"
#include "ui_mainwindow.h"

#include "macros.hpp"

#include <QFileDialog>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent)
  , ui(new Ui::MainWindow)
{
  ui->setupUi(this);

  m_model = new FileSystemModel(this);

  ui->m_fileTree->setModel(m_model);
  ui->m_fileTree->setHeaderHidden(true);
  ui->m_fileTree->setSelectionMode(QAbstractItemView::SingleSelection);

  for (int i = 1; i < m_model->columnCount(QModelIndex()); ++i)
    ui->m_fileTree->hideColumn(i);

  ui->m_fileTable->setModel(m_model);
  ui->m_fileTable->setSelectionMode(QAbstractItemView::SingleSelection);
  ui->m_fileTable->setRootIndex(QModelIndex());
  ui->m_fileTable->verticalHeader()->hide();

  QHeaderView * tableHeader = ui->m_fileTable->horizontalHeader();
  tableHeader->resizeSections(QHeaderView::Interactive);
  tableHeader->setStretchLastSection(true);
  tableHeader->setCascadingSectionResizes(true);
  VERIFY(QObject::connect(tableHeader, &QHeaderView::sectionClicked,
                          this, &MainWindow::onTableHeaderClicked));

  VERIFY(QObject::connect(ui->m_rootEditor, SIGNAL(returnPressed()), this, SLOT(onRootSpecified())));
  VERIFY(QObject::connect(ui->m_rootDlgButton, SIGNAL(clicked()), this, SLOT(onRootDialogCall())));
  VERIFY(QObject::connect(ui->m_fileTree->selectionModel(), &QItemSelectionModel::selectionChanged,
                          this, &MainWindow::onTreeSelectionChanged));

  VERIFY(QObject::connect(ui->m_fileTable->selectionModel(), &QItemSelectionModel::selectionChanged,
                          this, &MainWindow::onTableSelectionChanged));
}

MainWindow::~MainWindow()
{
  delete ui;
}

void MainWindow::onRootDialogCall()
{
  QString dir = QFileDialog::getExistingDirectory(this, "Select root", "");
  if (dir.isEmpty())
    return;

  ui->m_rootEditor->setText(dir);
  onRootSpecified();
}

void MainWindow::onRootSpecified()
{
  QString rootDir = ui->m_rootEditor->text();
  m_model->setRoot(rootDir);
}

void MainWindow::onTableSelectionChanged(QItemSelection const & selected, QItemSelection const & /*deselected*/)
{
  QModelIndexList lst = selected.indexes();
  if (lst.empty())
    return;

  ui->m_fileTree->selectionModel()->select(selected, QItemSelectionModel::ClearAndSelect);
  QModelIndex item = lst.first().parent();
  while (item != QModelIndex())
  {
    ui->m_fileTree->setExpanded(item, true);
    item = item.parent();
  }
}

void MainWindow::onTreeSelectionChanged(QItemSelection const & selected, QItemSelection const & /*deselected*/)
{
  QModelIndexList lst = selected.indexes();
  if (lst.empty())
    return;

  QModelIndex index = lst.first();
  if (m_model->isDir(index))
    ui->m_fileTable->setRootIndex(m_model->index(index.row(), 0, index.parent()));
  else
  {
    ui->m_fileTable->setRootIndex(index.parent());
    ui->m_fileTable->selectionModel()->select(selected, QItemSelectionModel::ClearAndSelect);
    ui->m_fileTable->scrollTo(index);
  }
}

void MainWindow::onTableHeaderClicked(int index)
{
  ui->m_fileTable->resizeColumnToContents(index);
}
