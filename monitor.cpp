#include "monitor.h"
#include <QtCore/QtCore>
#include <QProcess>

Monitor::Monitor(QObject *parent) :
    QObject(parent)
{
}

void Monitor::error(QProcess::ProcessError error)
{
  qDebug() << "Error: " << error;
}

void 	Monitor::finished(int exitCode, QProcess::ExitStatus exitStatus)
{
  qDebug() << "Finished: " << exitCode;
  qApp->exit();
}

void 	Monitor::readyReadStandardError()
{
  qDebug() << "ReadyError";
}

void 	Monitor::readyReadStandardOutput()
{
  qDebug() << "readyOut";
  QProcess *p = (QProcess *)sender();
  QByteArray buf = p->readAllStandardOutput();

  qDebug() << buf;
}

void 	Monitor::started()
{
  qDebug() << "Proc Started";
}

