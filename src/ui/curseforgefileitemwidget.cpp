#include "curseforgefileitemwidget.h"
#include "ui_curseforgefileitemwidget.h"

#include <QDebug>

#include "local/localmod.h"
#include "curseforge/curseforgemod.h"
#include "util/funcutil.h"
#include "download/downloadmanager.h"

CurseforgeFileItemWidget::CurseforgeFileItemWidget(QWidget *parent, CurseforgeMod *mod, const CurseforgeFileInfo &info, const QString &path, LocalMod *localMod) :
    QWidget(parent),
    ui(new Ui::CurseforgeFileItemWidget),
    mod_(mod),
    localMod_(localMod),
    fileInfo_(info),
    downloadPath_(path)
{
    ui->setupUi(this);
    ui->displayNameText->setText(fileInfo_.displayName());
    ui->downloadProgress->setVisible(false);

    //file name and link
    QString linkText = fileInfo_.fileName();
    linkText = "<a href=%1>" + linkText + "</a>";
    ui->fileNameText->setText(linkText.arg(fileInfo_.url().toString()));

    //game version
    QString gameversionText;
    for(const auto &ver : fileInfo_.gameVersions())
        gameversionText.append(ver).append(" ");
    ui->gameVersionText->setText(gameversionText);

    //loader type
    QString loaderTypeText;
    for(const auto &loader : fileInfo_.loaderTypes())
        loaderTypeText.append(ModLoaderType::toString(loader)).append(" ");
    ui->loaderTypeText->setText(loaderTypeText);

    //size
    ui->downloadSpeedText->setText(numberConvert(fileInfo_.size(), "B"));
}

CurseforgeFileItemWidget::~CurseforgeFileItemWidget()
{
    delete ui;
}

void CurseforgeFileItemWidget::on_downloadButton_clicked()
{
    ui->downloadButton->setText(tr("Downloading"));
    ui->downloadButton->setEnabled(false);
    ui->downloadProgress->setVisible(true);

    ui->downloadProgress->setMaximum(fileInfo_.size());

    DownloadFileInfo info(fileInfo_);
    if(localMod_)
        info.setIconBytes(localMod_->modInfo().iconBytes());
    else
        info.setIconBytes(mod_->modInfo().iconBytes());
    info.setPath(downloadPath_);

    auto downloader = DownloadManager::addModDownload(info);
    connect(downloader, &Downloader::downloadProgress, this, [=](qint64 bytesReceived, qint64 /*bytesTotal*/){
        ui->downloadProgress->setValue(bytesReceived);
    });
    connect(downloader, &ModDownloader::downloadSpeed, this, [=](qint64 bytesPerSec){
        ui->downloadSpeedText->setText(numberConvert(bytesPerSec, "B/s"));
    });
    connect(downloader, &Downloader::finished, this, [=]{
        ui->downloadProgress->setVisible(false);
        ui->downloadSpeedText->setText(numberConvert(fileInfo_.size(), "B"));
        ui->downloadButton->setText(tr("Downloaded"));
        if(localMod_){
            LocalModInfo info(downloader->filePath());
            QFile file(downloader->filePath());
            info.addOld();
            if(!file.rename(file.fileName() + ".old")){
                file.remove();
                return;
            }
            localMod_->addOldInfo(info);
        }
    });
}

void CurseforgeFileItemWidget::setDownloadPath(const QString &newDownloadPath)
{
    downloadPath_ = newDownloadPath;
}
