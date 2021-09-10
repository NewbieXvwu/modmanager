#include "modrinthmodbrowser.h"
#include "ui_modrinthmodbrowser.h"

#include <QScrollBar>
#include <QDir>
#include <QDesktopServices>

#include "modrinthmoditemwidget.h"
#include "modrinthmodinfodialog.h"
#include "local/localmodpathmanager.h"
#include "local/localmodpath.h"
#include "modrinth/modrinthmod.h"
#include "modrinth/modrinthapi.h"
#include "gameversion.h"
#include "modloadertype.h"
#include "config.h"

ModrinthModBrowser::ModrinthModBrowser(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ModrinthModBrowser),
    api_(new ModrinthAPI(this))
{
    ui->setupUi(this);

    for(const auto &type : ModLoaderType::modrinth)
        ui->loaderSelect->addItem(ModLoaderType::toString(type));

    connect(ui->modListWidget->verticalScrollBar(), &QAbstractSlider::valueChanged,  this , &ModrinthModBrowser::onSliderChanged);

    updateVersionList();
    connect(VersionManager::manager(), &VersionManager::modrinthVersionListUpdated, this, &ModrinthModBrowser::updateVersionList);

    updateLocalPathList();
    connect(LocalModPathManager::manager(), &LocalModPathManager::pathListUpdated, this, &ModrinthModBrowser::updateLocalPathList);

    getModList(currentName_);
    isUiSet_ = true;
}

ModrinthModBrowser::~ModrinthModBrowser()
{
    delete ui;
}

void ModrinthModBrowser::searchModByPathInfo(const LocalModPathInfo &info)
{
    isUiSet_ = false;
    ui->versionSelect->setCurrentText(info.gameVersion());
    ui->loaderSelect->setCurrentIndex(ModLoaderType::modrinth.indexOf(info.loaderType()));
    ui->downloadPathSelect->setCurrentText(info.showText());
    isUiSet_ = true;
    getModList(currentName_);
}

void ModrinthModBrowser::updateVersionList()
{
    isUiSet_ = false;
    ui->versionSelect->clear();
    ui->versionSelect->addItem(tr("Any"));
    for(const auto &version : GameVersion::modrinthVersionList())
        ui->versionSelect->addItem(version);
    isUiSet_ = true;
}

void ModrinthModBrowser::updateLocalPathList()
{
    ui->downloadPathSelect->clear();
    downloadPathList_.clear();
    ui->downloadPathSelect->addItem(tr("Custom"));
    downloadPathList_ << Config().getDownloadPath();
    for(const auto &path : LocalModPathManager::pathList()){
        ui->downloadPathSelect->addItem(path->info().showText());
        downloadPathList_ << path->info().path();
    }
}

void ModrinthModBrowser::on_searchButton_clicked()
{
    currentName_ = ui->searchText->text();
    getModList(currentName_);
}

void ModrinthModBrowser::onSliderChanged(int i)
{
    if(hasMore_ && i == ui->modListWidget->verticalScrollBar()->maximum()){
        currentIndex_ += 20;
        getModList(currentName_, currentIndex_);
    }
}

void ModrinthModBrowser::getModList(QString name, int index)
{
    if(!index) currentIndex_ = 0;
    ui->searchButton->setText(tr("Searching..."));
    ui->searchButton->setEnabled(false);
    setCursor(Qt::BusyCursor);

    GameVersion gameVersion = ui->versionSelect->currentIndex()? GameVersion(ui->versionSelect->currentText()) : GameVersion::Any;
    auto sort = ui->sortSelect->currentIndex();
    GameVersion version = ui->versionSelect->currentIndex()? GameVersion(ui->versionSelect->currentText()) : GameVersion::Any;
    auto type = ModLoaderType::modrinth.at(ui->loaderSelect->currentIndex());

    api_->searchMods(name, currentIndex_, version, type, sort, [=](const QList<ModrinthModInfo> &infoList){
        ui->searchButton->setText(tr("&Search"));
        ui->searchButton->setEnabled(true);
        setCursor(Qt::ArrowCursor);

        //new search
        if(currentIndex_ == 0){
            modList_.clear();
            for(int i = 0; i < ui->modListWidget->count(); i++)
                ui->modListWidget->itemWidget(ui->modListWidget->item(i))->deleteLater();
            ui->modListWidget->clear();
            hasMore_ = true;
        }

        if(infoList.isEmpty()){
            hasMore_ = false;
            return ;
        }

        //show them
        for(const auto &info : qAsConst(infoList)){
            auto modrinthMod = new ModrinthMod(this, info);
            modList_.append(modrinthMod);

            auto *listItem = new QListWidgetItem();
            listItem->setSizeHint(QSize(500, 100));
            auto version = ui->versionSelect->currentIndex()? GameVersion(ui->versionSelect->currentText()): GameVersion::Any;
            auto downloadPath = downloadPathList_.at(ui->downloadPathSelect->currentIndex());
            auto modItemWidget = new ModrinthModItemWidget(ui->modListWidget, modrinthMod, downloadPath);
            connect(this, &ModrinthModBrowser::downloadPathChanged, modItemWidget, &ModrinthModItemWidget::setDownloadPath);
            ui->modListWidget->addItem(listItem);
            ui->modListWidget->setItemWidget(listItem, modItemWidget);
            modrinthMod->acquireIcon();
        }
    });
}


void ModrinthModBrowser::on_modListWidget_doubleClicked(const QModelIndex &index)
{
    auto mod = modList_.at(index.row());
    auto dialog = new ModrinthModInfoDialog(this, mod);
    dialog->show();
}


void ModrinthModBrowser::on_sortSelect_currentIndexChanged(int)
{
    if(isUiSet_) getModList(currentName_);
}


void ModrinthModBrowser::on_versionSelect_currentIndexChanged(int)
{
    if(isUiSet_) getModList(currentName_);
}


void ModrinthModBrowser::on_loaderSelect_currentIndexChanged(int)
{
    if(isUiSet_) getModList(currentName_);
}


void ModrinthModBrowser::on_openFolderButton_clicked()
{
    QDir dir(downloadPathList_.at(ui->downloadPathSelect->currentIndex()));
    QUrl url(dir.absolutePath());
    url.setScheme("file");
    QDesktopServices::openUrl(url);
}


void ModrinthModBrowser::on_downloadPathSelect_currentIndexChanged(int index)
{
    if(!isUiSet_ || index < 0 || index >= downloadPathList_.size()) return;
    emit downloadPathChanged(downloadPathList_.at(index));
}
