#ifndef HLRUNNER_H
#define HLRUNNER_H

#include <QObject>
#include <QProcess>

class HLRunner : public QObject
{
    Q_OBJECT
public:
    explicit HLRunner(QObject *parent = nullptr);

     void run();
signals:
     void finished();
private:
     QProcess proc;
private slots:
      void on_process_finished(int exitCode, QProcess::ExitStatus exitStatus);

signals:

};

#endif // HLRUNNER_H
