#include "mainwindow.hpp"
#include "ui_mainwindow.h"

#include "macros.hpp"
#include "proxy_item_delegate.hpp"
#include "reg_exp_dialog.hpp"

#include <QFileDialog>
#include <QSettings>

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent)
  , m_ui(new Ui::MainWindow)
  , m_ignoreTableSelection(false)
{
  m_fileModel = new FileSystemModel(this);
  m_model = new QSortFilterProxyModel(m_fileModel);
  m_model->setSourceModel(m_fileModel);

  m_ui->setupUi(this);

  m_ui->m_fileTree->setModel(m_model);

  for (int i = 1; i < m_model->columnCount(QModelIndex()); ++i)
    m_ui->m_fileTree->hideColumn(i);

  m_ui->m_fileTable->setModel(m_model);

  QAbstractItemDelegate * proxyDelegate = new ProxyItemDelegate(m_ui->m_fileTable->itemDelegate(), m_ui->m_fileTable);
  m_ui->m_fileTable->setItemDelegate(proxyDelegate);

  m_ui->m_fileTable->setRootIndex(QModelIndex());

  m_ui->m_fileTable->verticalHeader()->setDefaultSectionSize(18);

  QHeaderView * tableHeader = m_ui->m_fileTable->horizontalHeader();
  tableHeader->resizeSections(QHeaderView::Interactive);

  QAction * resizeToContent = new QAction(QStringLiteral("Resize to content"), tableHeader);
  VERIFY(QObject::connect(resizeToContent, &QAction::triggered,
                          this, &MainWindow::onResizeColumns));

  tableHeader->setContextMenuPolicy(Qt::ActionsContextMenu);
  tableHeader->addAction(resizeToContent);

  VERIFY(QObject::connect(m_ui->m_rootEditor, SIGNAL(returnPressed()), this, SLOT(onRootSpecified())));
  VERIFY(QObject::connect(m_ui->m_rootDlgButton, SIGNAL(clicked()), this, SLOT(onRootDialogCall())));
  VERIFY(QObject::connect(m_ui->m_fileTree->selectionModel(), &QItemSelectionModel::selectionChanged,
                          this, &MainWindow::onTreeSelectionChanged));

  VERIFY(QObject::connect(m_ui->m_fileTable->selectionModel(), &QItemSelectionModel::selectionChanged,
                          this, &MainWindow::onTableSelectionChanged));

  QAction * regExpAction = new QAction(QStringLiteral("Set filter regexp"), this);
  m_ui->m_fileTable->addAction(regExpAction);
  m_ui->m_fileTree->addAction(regExpAction);
  VERIFY(QObject::connect(regExpAction, &QAction::triggered,
                          this, &MainWindow::onSetRegExp));

  LoadState();
}

MainWindow::~MainWindow()
{
  SaveState();
  delete m_ui;
}

void MainWindow::LoadState()
{
  QSettings settings("settings.ini", QSettings::IniFormat);
  m_ui->m_rootEditor->setText(settings.value("RootPath", "").toString());

  settings.beginGroup("MainWindow");
  QByteArray windowGeometry = settings.value("geometry", QByteArray()).toByteArray();
  restoreGeometry(windowGeometry);
  settings.endGroup();

  settings.beginGroup("Splitter");
  QByteArray splitterSizes = settings.value("sizes", QByteArray()).toByteArray();
  m_ui->m_splitter->restoreState(splitterSizes);
  settings.endGroup();

  settings.beginGroup("FileTable");
  QByteArray header = settings.value("tableHeader", QByteArray()).toByteArray();
  m_ui->m_fileTable->horizontalHeader()->restoreState(header);
  settings.endGroup();

  onRootSpecified();
}

void MainWindow::SaveState()
{
  QSettings settings("settings.ini", QSettings::IniFormat);
  settings.setValue("RootPath", m_ui->m_rootEditor->text());

  settings.beginGroup("MainWindow");
  settings.setValue("geometry", saveGeometry());
  settings.endGroup();

  settings.beginGroup("Splitter");
  settings.setValue("sizes", m_ui->m_splitter->saveState());
  settings.endGroup();

  settings.beginGroup("FileTable");
  settings.setValue("tableHeader", m_ui->m_fileTable->horizontalHeader()->saveState());
  settings.endGroup();
}

void MainWindow::onRootDialogCall()
{
  QString dir = QFileDialog::getExistingDirectory(this, "Select root", "");
  if (dir.isEmpty())
    return;

  m_ui->m_rootEditor->setText(dir);
  onRootSpecified();
}

void MainWindow::onRootSpecified()
{
  QString rootDir = m_ui->m_rootEditor->text();
  m_fileModel->setRoot(rootDir);
}

namespace
{

class BoolGuard
{
public:
  BoolGuard(bool & v)
    : m_v(v)
  {
    m_v = true;
  }

  ~BoolGuard()
  {
    m_v = false;
  }

private:
  bool & m_v;
};

} // namespace

void MainWindow::onTableSelectionChanged(QItemSelection const & selected, QItemSelection const & /*deselected*/)
{
  BoolGuard guard(m_ignoreTableSelection);

  QModelIndexList lst = selected.indexes();
  if (lst.empty())
    return;

  m_ui->m_fileTree->selectionModel()->select(selected, QItemSelectionModel::ClearAndSelect);
  QModelIndex item = lst.first().parent();
  while (item != QModelIndex())
  {
    if (!m_ui->m_fileTree->isExpanded(item))
      m_ui->m_fileTree->expand(item);
    item = item.parent();
  }

  QModelIndex sourceIndex = m_model->mapToSource(lst.first());
  if (m_fileModel->canFetchMore(sourceIndex))
    m_fileModel->fetchMore(sourceIndex);
}

void MainWindow::onTreeSelectionChanged(QItemSelection const & selected, QItemSelection const & /*deselected*/)
{
  if (m_ignoreTableSelection)
    return;

  QModelIndexList lst = selected.indexes();
  if (lst.empty())
    return;

  QModelIndex index = lst.first();
  QModelIndex currentRoot = m_ui->m_fileTable->rootIndex();
  if (m_fileModel->isDir(m_model->mapToSource(index)) && index != currentRoot)
  {
    m_ui->m_fileTable->setRootIndex(index);
    m_ui->m_fileTable->selectionModel()->clear();
  }
  else
  {
    QModelIndex idxParent = index.parent();
    if (idxParent != currentRoot)
      m_ui->m_fileTable->setRootIndex(idxParent);

    m_ui->m_fileTable->selectionModel()->select(selected, QItemSelectionModel::ClearAndSelect);
  }
}

void MainWindow::onResizeColumns()
{
  m_ui->m_fileTable->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
  m_ui->m_fileTable->horizontalHeader()->setStretchLastSection(true);
}

void MainWindow::onSetRegExp()
{
  RegExpDialog dlg(m_model->filterRegExp(), this);

  if (dlg.exec() == QDialog::Accepted)
  {
    m_model->setFilterRole(Qt::DisplayRole);
    m_model->setFilterKeyColumn(0);
    m_model->setFilterRegExp(dlg.GetRegExp());
  }
}
