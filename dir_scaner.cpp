#include "dir_scaner.hpp"

#include <QDirIterator>

DirScaner::DirScaner(QString const & path)
  : m_path(path)
  , m_canceled(false)
{
}

void DirScaner::run()
{
  QDirIterator iter(m_path);
  while (iter.hasNext())
  {
    if (m_canceled == true)
      break;

    iter.next();
    emit fileFounded(iter.fileInfo(), this);
  }

  emit scanFinished(this);
}

void DirScaner::cancel()
{
  m_canceled = true;
}

