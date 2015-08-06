#include "file_system_model.hpp"
#include "macros.hpp"

#include <QFileInfo>
#include <QDirIterator>

namespace
{

template <typename T, typename ...Args>
std::unique_ptr<T> MakeUnique(Args &&... args)
{
  return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

class Node
{
public:
  Node(QFileInfo const & info)
    : m_info(info)
  {
  }

  void AddChild(QFileInfo const & info)
  {
    m_children.emplace_back(new Node(info, this));
  }

  Node * GetParent() const
  {
    return m_parent;
  }

  QFileInfo const & GetInfo() const
  {
    return m_info;
  }

  size_t GetChildCount() const
  {
    return m_children.size();
  }

  Node * GetChild(size_t index) const
  {
    Q_ASSERT(index < m_children.size());
    return m_children[index].get();
  }

  Node * GetLastChild() const
  {
    Q_ASSERT(!m_children.empty());
    return m_children.back().get();
  }

  Qt::CheckState GetCheckState() const
  {
    return m_checkState;
  }

  void SetCheckState(Qt::CheckState state)
  {
    m_checkState = state;
  }

private:
  Node(QFileInfo const & info, Node * parent)
    : m_info(info)
    , m_parent(parent)
  {
  }

  QFileInfo m_info;
  Qt::CheckState m_checkState = Qt::Unchecked;
  Node * m_parent = nullptr;
  std::vector<std::unique_ptr<Node> > m_children;
};

void setRootImpl(Node * root, int depth)
{
  if (depth > 4)
    return;

  QFileInfo const & info = root->GetInfo();
  QDirIterator iter(info.absoluteFilePath());
  while (iter.hasNext())
  {
    iter.next();
    QFileInfo info = iter.fileInfo();
    QString fileName = info.fileName();
    if (fileName != "." && fileName != "..")
    {
      root->AddChild(iter.fileInfo());
      setRootImpl(root->GetLastChild(), depth + 1);
    }
  }
}

} // namespace

struct FileSystemModel::Impl
{
  std::unique_ptr<Node> m_root;
};

#define ROOT m_impl->m_root

QVariant getName(Node const * node)
{
  QFileInfo info = node->GetInfo();
  if (info.isRoot())
    return info.absolutePath();
  return info.fileName();
}

QVariant getSize(Node const * node)
{
  QFileInfo const & info = node->GetInfo();
  if (info.isDir())
    return QVariant();
  return info.size();
}

QVariant getCheckState(Node const * node)
{
  return node->GetCheckState();
}

class FieldHelper
{
public:
  FieldHelper()
  {
    addField(bind(&getName, _1), "Name");
    addField(bind(&getSize, _1), "Size");

    m_stateGetters.push_back(bind(&getCheckState, _1));
  }

  int getFieldCount() const
  {
    return m_fieldGetters.size();
  }

  QVariant getFieldValue(Node const * node, int column, int role) const
  {
    if (role == Qt::DisplayRole)
      return m_fieldGetters[column](node);
    else if (role == Qt::CheckStateRole && column == 0)
      return m_stateGetters[0](node);

    return QVariant();
  }

  QVariant getFieldName(int section, int role)
  {
    if (role != Qt::DisplayRole)
      return QVariant();

    Q_ASSERT(section < static_cast<int>(m_fieldNames.size()));
    return m_fieldNames[section];
  }

  bool setFieldValue(Node * node, QVariant const & v, int column, int role) const
  {
    if (role == Qt::CheckStateRole && column == 0)
    {
      Q_ASSERT(v.canConvert<int>());
      node->SetCheckState((Qt::CheckState)v.value<int>());
    }

    return false;
  }

private:
  using TFieldGetter = function<QVariant (Node const *)>;
  void addField(TFieldGetter const & getter, QString const & name)
  {
    Q_ASSERT(m_fieldGetters.size() == m_fieldNames.size());
    m_fieldGetters.push_back(getter);
    m_fieldNames.push_back(name);
  }

private:
  std::vector<TFieldGetter> m_fieldGetters;
  std::vector<QString> m_fieldNames;
  std::vector<TFieldGetter> m_stateGetters;
};

static FieldHelper s_helper;

////////////////////////////////////////

FileSystemModel::FileSystemModel(QObject * parent)
  : TBase(parent)
  , m_impl(new Impl())
{
}

FileSystemModel::~FileSystemModel()
{
  m_impl.reset();
}

void FileSystemModel::setRoot(QString const & rootPath)
{
  beginResetModel();
  ROOT.reset();
  if (QDir(rootPath).exists())
  {
    ROOT.reset(new Node(QFileInfo(rootPath)));
    setRootImpl(ROOT.get(), 0);
  }
  endResetModel();
}

int FileSystemModel::rowCount(QModelIndex const & parent) const
{
  if (!parent.isValid())
    return ROOT ? 1 : 0;

  Q_ASSERT(parent.internalPointer() != nullptr);
  Node * node = static_cast<Node *>(parent.internalPointer());
  return node->GetChildCount();
}

int FileSystemModel::columnCount(QModelIndex const & /*parent*/) const
{
  return s_helper.getFieldCount();
}

QModelIndex FileSystemModel::index(int row, int column, QModelIndex const & parent) const
{
  Node * node = static_cast<Node *>(parent.internalPointer());
  if (node == nullptr)
    return createIndex(row, column, ROOT.get());

  return createIndex(row, column, ROOT->GetChild(row));
}

QModelIndex FileSystemModel::parent(QModelIndex const & child) const
{
  Node * childNode = static_cast<Node *>(child.internalPointer());
  if (childNode == nullptr)
    return QModelIndex();


  Node * parent = childNode->GetParent();
  if (parent == nullptr)
    return QModelIndex();

  return createIndex(0, 0, parent);
}

QVariant FileSystemModel::data(QModelIndex const & index, int role) const
{
  Q_ASSERT(index.internalPointer() != nullptr);
  Node * node = static_cast<Node *>(index.internalPointer());
  return s_helper.getFieldValue(node, index.column(), role);
}

bool FileSystemModel::setData(QModelIndex const & index, QVariant const & value, int role)
{
  Q_ASSERT(index.internalPointer() != nullptr);
  Node * node = static_cast<Node *>(index.internalPointer());
  return s_helper.setFieldValue(node, value, index.column(), role);
}

QVariant FileSystemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (orientation == Qt::Vertical)
    return QVariant();

  return s_helper.getFieldName(section, role);
}

Qt::ItemFlags FileSystemModel::flags(QModelIndex const & index) const
{
  Qt::ItemFlags flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
  if (index.column() == 0)
    flags |= Qt::ItemIsUserCheckable;
//  if (rowCount(index) > 0)
//    flags |= Qt::ItemIsTristate;

  return flags;
}

