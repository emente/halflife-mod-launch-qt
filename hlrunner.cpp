#include "hlrunner.h"
#include <QThread>

HLRunner::HLRunner(QObject *parent)
    : QObject{parent}
{
    connect(this->proc,
    SIGNAL(finished(int,QProcess::ExitStatus)),
    this,
    SLOT(on_process_finished(int, QProcess::ExitStatus))
    );
}


void HLRunner::run()
{
    this->proc.start();
    this->proc.waitForFinished();
}
