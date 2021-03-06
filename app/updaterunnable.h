#ifndef UPDATERUNNABLE_H
#define UPDATERUNNABLE_H

#include <QObject>
#include <QRunnable>

/***********************************************************************************/
/// Used to update Navigator code from git, and rebuild frontend files.
/** This is always invoked by QThreadPool */
class UpdateRunnable : public QObject, public QRunnable {
  Q_OBJECT
public:
  explicit UpdateRunnable(const QString &ONInstallDir);

protected:
  void run() override;

signals:
  /// Emitted when update check complete.
  void finished();

private:
  const QString m_ONInstallDir;
};

#endif // UPDATERUNNABLE_H
