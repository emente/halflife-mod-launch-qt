#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "Windows.h"

#include <QDir>
#include <QDesktopServices>
#include <QUrl>
#include <QProcess>
#include <QTextStream>
#include <QSettings>
#include <QThread>
#include <QObject>
#include <QFileDialog>

const QString MainWindow::extractQuotedString(QStringList* params)
{
    params->removeFirst();
    QString combined = params->join(' ').trimmed();
    if (combined.at(0) == '"') {
        combined.chop(1);
        combined.remove(0,1);
    }
    return combined;
}

qint64 MainWindow::getScreenshotCount(const QString name)
{
    qint64 shots=0;
    QDir dir2(hlpath+"\\"+name+"\\");
    foreach(QFileInfo item, dir2.entryInfoList() )
    {
        if (item.fileName().endsWith(".tga",Qt::CaseInsensitive) &&
            item.fileName().startsWith("halflife",Qt::CaseInsensitive)) {
            shots++;
        }
    }
    return shots;
}

qint64 MainWindow::getSavegameCount(const QString name)
{
    qint64 saves=0;
    QDir dir(hlpath+"\\"+name+"\\save\\");
    foreach(QFileInfo item, dir.entryInfoList() )
    {
        if (item.fileName().endsWith(".sav",Qt::CaseInsensitive)) {
            saves++;
        }
    }
    return saves;
}


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QSettings hlpathini("Org","qt-hl-modrun");
    hlpath = hlpathini.value("hlpath",QDir::currentPath()).toString();
    while (!QFile::exists(hlpath+"\\hl.exe")) {
        QString filename = QFileDialog::getOpenFileName(this, "Select hl.exe inside Half-Life folder", hlpath, tr("Half-life binary (hl.exe)"));
        QFileInfo info(filename);
        hlpath = info.absolutePath();
        hlpathini.setValue("hlpath",hlpath);
    }

    playinfos = new QSettings(hlpath+"\\playstats.ini",QSettings::IniFormat);
    QString lastmod = playinfos->value("settings/last_mod","").toString();

    QDir dir(hlpath);
    dir.setSorting(QDir::Name | QDir::IgnoreCase);

    uint index=0;
    uint lastindex=0;
    foreach(QFileInfo item, dir.entryInfoList() )
       {
           if(item.isDir())
           {
                QString inipath = item.absoluteFilePath()+"\\liblist.gam";
                QString gamepath=item.baseName().toLower();

                QFile inifile(inipath);
                if (inifile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                    games.append(Gameinfo());
                    games.last().path = gamepath;
                    games.last().name = gamepath;
                    games.last().map = "?";
                    games.last().dll = "?";

                    if (gamepath == lastmod){
                        lastindex=index;
                    }

                    QTextStream in(&inifile);
                    while (!in.atEnd()) {
                        QString line = in.readLine();
                        QStringList params = line.trimmed().split(" ");

                        if (params.at(0) == "game") {
                            games.last().name = extractQuotedString(&params);
                        } else if (params.at(0) == "startmap") {
                            games.last().map = extractQuotedString(&params);
                        } else if (params.at(0) == "gamedll") {
                            games.last().dll = extractQuotedString(&params);
                        } else if (params.at(0) == "url_info") {
                            games.last().url = extractQuotedString(&params);
                        }
                    }

                    games.last().fresh = 0 == playinfos->value("launches/"+gamepath,0).toInt() +
                            playinfos->value("playtime/"+gamepath,0).toInt() +
                            getSavegameCount(gamepath)+
                            getScreenshotCount(gamepath);

                    gamenames.append(games.last().name);
                    index++;
                }

           }
    }

    gamemodel.setStringList(gamenames);
    ui->listView->setModel(&gamemodel);

    QModelIndex m = ui->listView->model()->index(lastindex,0);
    ui->listView->setCurrentIndex(m);
    ui->listView->scrollTo(m);
    on_listView_clicked(m);

    notesSaveTimer.setSingleShot(true);
    connect(&notesSaveTimer, SIGNAL(timeout()), this, SLOT(notesSaveTimeout()));

    timerId = startTimer(1000);
}

MainWindow::~MainWindow()
{
     killTimer(timerId);
    delete ui;
}

void MainWindow::on_listView_clicked(const QModelIndex &index)
{
    on_listView_activated(index);
}

void MainWindow::on_btnLaunch_clicked()
{
    playinfos->setValue("settings/last_mod",ui->edPath->text());
    playinfos->setValue("launches/"+ui->edPath->text(),playinfos->value("launches", 0).toInt() + 1);
    playinfos->setValue("last_launch/"+ui->edPath->text(),QDateTime::currentDateTime().toSecsSinceEpoch());

    QStringList params;
    params.append("-startwindowed");
    params.append("-dev");
    params.append("-console");
    params.append("-nointro");
    params.append("-noipx");
    params.append("-game");
    params.append(ui->edPath->text());
    if (ui->edDll->text().length()>0) {
        params.append("-dll "+ui->edDll->text());
    }
    if (ui->edMap->text().length()>0) {
        params.append("-map "+ui->edMap->text());
        //params.append("-startmap "+ui->edMap->text());
    }
    if (ui->cbDefaultSettings->isChecked()) {
        params.append("+exec mnt.cfg");
    }

    playinfos->sync();

    qDebug() << "hl starting";
    thread = new QThread();
    run = new QProcess();
    run->setWorkingDirectory(hlpath);
    run->start(hlpath+"\\hl.exe", params);
    run->waitForStarted();
    run->moveToThread(thread);
}

void MainWindow::on_edRating_sliderMoved(int position)
{
    switch (position) {
        case -1:
        ui->lblRatingText->setText("DELETE");
        break;
        case 0:
        ui->lblRatingText->setText("-");
        break;
        case 1:
        ui->lblRatingText->setText("Bad :(");
        break;
        case 2:
        ui->lblRatingText->setText("Okay :)");
        break;
        case 3:
        ui->lblRatingText->setText("Good :D");
        break;
    }
}

void MainWindow::timerEvent(QTimerEvent *event)
{
    HWND hplay = FindWindow(TEXT("Half-Life"),NULL);
    HWND hmenu = FindWindow(NULL,TEXT("Half-Life"));
    if (!IsWindowVisible(hplay)) {
        hplay=0;
    }

    if ((hplay!=0 || hmenu!=0) && !hllaunched) {
        hllaunched=true;
        playseconds=0;
        qDebug() << "hl started";
    }
    if (hplay!=0 && hllaunched) {
        playseconds++;
        qDebug() << "hl playing";
    }
    if ((hplay==0 && hmenu==0) && hllaunched) {
        hllaunched=false;
        qDebug() << "hl quitted";
        on_listView_clicked(ui->listView->currentIndex());
    }

    if (playseconds>60) {
        uint mins = playinfos->value("playtime/"+ui->edPath->text(),0).toUInt();
        while (playseconds>60) {
            playseconds=-60;
            mins++;
        }
        playinfos->setValue("playtime/"+ui->edPath->text(),mins);
    }
}

void MainWindow::on_edNotes_textChanged()
{
    if (notesSaveTimer.isActive())
    {
        notesSaveTimer.stop();
    }
    notesSaveTimer.start(1000);
}

void MainWindow::notesSaveTimeout()
{
    playinfos->setValue("note/"+ui->edPath->text(),ui->edNotes->toPlainText());
}

void MainWindow::on_btnGoUrl_clicked()
{
    QString url = ui->edUrl->text();
    if (!url.startsWith("http")) {
        url.prepend("http://");
    }
    QDesktopServices::openUrl(QUrl(url));
}

void MainWindow::on_edRating_sliderReleased()
{
    playinfos->setValue("rating/"+ui->edPath->text(),ui->edRating->value());
}


void MainWindow::on_listView_activated(const QModelIndex &index)
{
    Gameinfo info = games.at(index.row());
    ui->edName->setText(info.name);
    ui->edDll->setText(info.dll);
    ui->edMap->setText(info.map);
    ui->edPath->setText(info.path);
    ui->edUrl->setText(info.url);
    ui->edRating->setValue(playinfos->value("rating/"+ui->edPath->text(),0).toInt());
    ui->edNotes->setPlainText(playinfos->value("note/"+ui->edPath->text(),"").toString());
    ui->edPlaytime->setText(playinfos->value("playtime/"+ui->edPath->text(),"0").toString());

    QString times = playinfos->value("launches/"+ui->edPath->text(),0).toString();
    if (times!="0") {
        ui->edTimesPlayed->setText(times);
        qint64 secs = QDateTime::currentDateTime().toSecsSinceEpoch()- playinfos->value("last_launch/"+ui->edPath->text(),0).toInt();
        qint64 days = ceil(secs/(60*60*24));
        if (days==0) {
            ui->edLastPlayed->setText("today");
        } else {
            ui->edLastPlayed->setText(QString::number(days)+" days ago");
        }
    } else {
        ui->edTimesPlayed->setText("0");
        ui->edLastPlayed->setText("-");
    }

    ui->edNumSaves->setText(QString::number(getSavegameCount(ui->edPath->text())));
    ui->edNumShots->setText(QString::number(getScreenshotCount(ui->edPath->text())));
}

