#ifndef MONITOR_H
#define MONITOR_H

#include <QObject>
#include <QtCore/QtCore>

class Monitor : public QObject
{
Q_OBJECT
public:
explicit Monitor(QObject *parent = 0);

signals:


public slots:
void error(QProcess::ProcessError error);
void 	finished(int exitCode, QProcess::ExitStatus exitStatus);
void 	readyReadStandardError();
void 	readyReadStandardOutput();
void 	started();
};

#endif // MONITOR_H
