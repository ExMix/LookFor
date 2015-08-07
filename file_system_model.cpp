#include "file_system_model.hpp"
#include "macros.hpp"

#include <QFileInfo>
#include <QDirIterator>
#include <QIcon>

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
    m_children.emplace_back(new Node(info, this, m_children.size()));
    if (m_checkState != Qt::Unchecked)
      m_children.back()->SetCheckState(Qt::Checked);
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

  int GetChildIndex() const
  {
    return m_childIndex;
  }

private:
  Node(QFileInfo const & info, Node * parent, int childIndex)
    : m_info(info)
    , m_parent(parent)
    , m_childIndex(childIndex)
  {
  }

  QFileInfo m_info;
  Qt::CheckState m_checkState = Qt::Unchecked;

  Node * m_parent = nullptr;
  int m_childIndex = 0;

  std::vector<std::unique_ptr<Node> > m_children;
};

void setRootImpl(Node * root, int depth)
{
  if (depth > 2)
    return;

  QDirIterator iter(root->GetInfo().absoluteFilePath());
  while (iter.hasNext())
  {
    iter.next();
    QFileInfo info = iter.fileInfo();
    QString fileName = info.fileName();
    if (fileName != "." && fileName != "..")
    {
      root->AddChild(info);
      if (info.isDir())
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

QVariant getIcon(Node const * node)
{
  static QIcon rootIcon(QStringLiteral(":/assets/root.png"));
  static QIcon folderIcon(QStringLiteral(":/assets/folder.png"));
  static QIcon fileIcon(QStringLiteral(":/assets/file.png"));

  QFileInfo const & info = node->GetInfo();
  QString path;
  if (info.isRoot())
    return rootIcon;
  else if (info.isDir())
    return folderIcon;

  return fileIcon;
}

class FieldHelper
{
  using TFieldGetter = function<QVariant (Node const *)>;
  using TStateGetters = QHash<int, TFieldGetter>;
public:
  FieldHelper()
  {
    addField(bind(&getName, _1), "Name");
    addField(bind(&getSize, _1), "Size");

    m_stateGetters[Qt::CheckStateRole] = bind(&getCheckState, _1);
    m_stateGetters[Qt::DecorationRole] = bind(&getIcon, _1);
  }

  int getFieldCount() const
  {
    return m_fieldGetters.size();
  }

  QVariant getFieldValue(Node const * node, int column, int role) const
  {
    if (role == Qt::DisplayRole)
      return m_fieldGetters[column](node);
    else if (column == 0)
    {
      TStateGetters::const_iterator fn = m_stateGetters.find(role);
      if (fn != m_stateGetters.end())
        return fn.value()(node);
    }

    return QVariant();
  }

  QVariant getFieldName(int section, int role)
  {
    if (role != Qt::DisplayRole)
      return QVariant();

    Q_ASSERT(section < static_cast<int>(m_fieldNames.size()));
    return m_fieldNames[section];
  }

  using TRange = std::pair<int, int>;
  using TDataChanged = function<void (Node * parent, TRange const & rowRange,
                                      TRange const & columnRange, int role)>;

  bool setFieldValue(Node * node, QVariant const & v, int column, int role, TDataChanged const & fn) const
  {
    if (role == Qt::CheckStateRole && column == 0)
    {
      Q_ASSERT(v.canConvert<int>());
      Qt::CheckState state = (Qt::CheckState)v.value<int>();
      node->SetCheckState(state);

      setCheckStateForChildren(node, state, fn);
      setCheckStateForParent(node, state, fn);

      return true;
    }

    return false;
  }

  bool isDir(Node * node)
  {
    if (node == nullptr)
      return false;

    return node->GetInfo().isDir();
  }

private:
  void setCheckStateForChildren(Node * parent, Qt::CheckState state, TDataChanged const & fn) const
  {
    int childCount = static_cast<int>(parent->GetChildCount());
    for (int i = 0; i < childCount; ++i)
    {
      Node * child = parent->GetChild(i);
      child->SetCheckState(state);
      setCheckStateForChildren(child, state, fn);
    }

    fn(parent, std::make_pair(0, childCount - 1), std::make_pair(0, 0), Qt::CheckStateRole);
  }

  void setCheckStateForParent(Node * node, Qt::CheckState state, TDataChanged const & fn) const
  {
    Node * parent = node->GetParent();

    int childIndex = node->GetChildIndex();
    fn(node, std::make_pair(childIndex, childIndex), std::make_pair(0, 0), Qt::CheckStateRole);

    if (parent == nullptr)
      return;

    if (state != Qt::PartiallyChecked)
    {
      for (size_t i = 0; i < parent->GetChildCount(); ++i)
      {
        if (parent->GetChild(i)->GetCheckState() != state)
        {
          state = Qt::PartiallyChecked;
          break;
        }
      }
    }

    parent->SetCheckState(state);
    setCheckStateForParent(parent, state, fn);
  }

private:
  void addField(TFieldGetter const & getter, QString const & name)
  {
    Q_ASSERT(m_fieldGetters.size() == m_fieldNames.size());
    m_fieldGetters.push_back(getter);
    m_fieldNames.push_back(name);
  }

private:
  std::vector<TFieldGetter> m_fieldGetters;
  std::vector<QString> m_fieldNames;
  TStateGetters m_stateGetters;
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

bool FileSystemModel::isDir(QModelIndex const & index)
{
  return s_helper.isDir(static_cast<Node *>(index.internalPointer()));
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

  return createIndex(row, column, node->GetChild(row));
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

  auto dataChangedSlot = [this](Node * parent, FieldHelper::TRange const & rowRange,
                                FieldHelper::TRange const & columnRange, int role)
  {
    if (rowRange.second - rowRange.first < 0 ||
        columnRange.second - columnRange.first)
      return;

    QModelIndex from = createIndex(rowRange.first, columnRange.first, parent);
    QModelIndex to = createIndex(rowRange.second, columnRange.second, parent);

    emitDataChanged(from, to, role);
  };

  return s_helper.setFieldValue(node, value, index.column(), role, dataChangedSlot);
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

  return flags;
}

void FileSystemModel::emitDataChanged(QModelIndex const & from, QModelIndex const & to, int role)
{
  Q_ASSERT(from.parent() == to.parent());
  emit dataChanged(from, to, QVector<int>{ role });
}

