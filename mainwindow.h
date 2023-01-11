#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "qthread.h"
#include "qtimer.h"
#include <QMainWindow>
#include <QVector>
#include <QStringListModel>
#include <QProcess>
#include <QSettings>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

typedef struct {
    QString name;
    QString path;
    QString map;
    QString dll;
    QString url;
    bool    fresh;
} Gameinfo;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_edNotes_textChanged();
    void notesSaveTimeout();
    void on_listView_clicked(const QModelIndex &index);
    void on_btnLaunch_clicked();
    void on_edRating_sliderMoved(int position);
    void on_btnGoUrl_clicked();

    void on_edRating_sliderReleased();

    void on_listView_activated(const QModelIndex &index);

private:
    Ui::MainWindow *ui;

    QString hlpath;

    QVector<Gameinfo> games;
    QStringListModel gamemodel;
    QStringList gamenames;

    QProcess *run;
    QThread *thread;

    int timerId; //seconds timer for playtime

    bool hllaunched=false;
    bool hlwindow_before=false;
    int playseconds=0;

    QSettings *playinfos;

    QTimer notesSaveTimer;
    void timerEvent(QTimerEvent *event);

    const QString extractQuotedString(QStringList* params);
    qint64 getScreenshotCount(const QString name);
    qint64 getSavegameCount(const QString name);
};


#endif // MAINWINDOW_H
