#pragma once

#include <QObject>
#include <QRunnable>
#include <atomic>

class QFileInfo;

class DirScaner : public QObject, public QRunnable
{
  Q_OBJECT

public:
  DirScaner(QString const & path);

  Q_SIGNAL void fileFounded(QFileInfo const & info, DirScaner * scaner);
  Q_SIGNAL void scanFinished(DirScaner * scaner);

  void cancel();

protected:
  void run();

private:
  QString m_path;
  std::atomic<bool> m_canceled;
};
