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
  m_selectionModel = new QItemSelectionModel(m_model);

  ui->m_fileTree->setModel(m_model);
  ui->m_fileTree->setHeaderHidden(true);
  ui->m_fileTree->setSelectionMode(QAbstractItemView::SingleSelection);
  ui->m_fileTree->setSelectionModel(m_selectionModel);

  for (int i = 1; i < m_model->columnCount(QModelIndex()); ++i)
    ui->m_fileTree->hideColumn(i);

  ui->m_fileTable->setModel(m_model);
  ui->m_fileTable->setSelectionMode(QAbstractItemView::SingleSelection);
  ui->m_fileTable->setSelectionModel(m_selectionModel);
  ui->m_fileTable->setRootIndex(QModelIndex());
  ui->m_fileTable->verticalHeader()->hide();
  ui->m_fileTable->horizontalHeader()->resizeSections(QHeaderView::Interactive);
  ui->m_fileTable->horizontalHeader()->setStretchLastSection(true);
  ui->m_fileTable->horizontalHeader()->setCascadingSectionResizes(true);

  VERIFY(QObject::connect(ui->m_rootEditor, SIGNAL(returnPressed()), this, SLOT(onRootSpecified())));
  VERIFY(QObject::connect(ui->m_rootDlgButton, SIGNAL(clicked()), this, SLOT(onRootDialogCall())));
  VERIFY(QObject::connect(m_selectionModel, &QItemSelectionModel::selectionChanged,
                          this, &MainWindow::onSelectionChanged));
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

void MainWindow::onSelectionChanged(QItemSelection const & selected, QItemSelection const & /*deselected*/)
{
  QModelIndexList lst = selected.indexes();
  if (lst.empty())
    return;

  ui->m_fileTable->setRootIndex(lst.first().parent());
}
