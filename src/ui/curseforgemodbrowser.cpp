#include "curseforgemodbrowser.h"
#include "ui_curseforgemodbrowser.h"

#include <QScrollBar>
#include <QUrlQuery>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QDebug>

#include "util/tutil.hpp"
#include "curseforge/curseforgemod.h"
#include "curseforge/curseforgeapi.h"
#include "curseforgemoditemwidget.h"
#include "curseforgemodinfodialog.h"
#include "gameversion.h"

CurseforgeModBrowser::CurseforgeModBrowser(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CurseforgeModBrowser),
    accessManager(new QNetworkAccessManager(this))
{
    ui->setupUi(this);
    connect(ui->modListWidget->verticalScrollBar(), &QAbstractSlider::valueChanged,  this , &CurseforgeModBrowser::onSliderChanged);
}

CurseforgeModBrowser::~CurseforgeModBrowser()
{
    delete ui;
}

void CurseforgeModBrowser::updateVersions()
{
    ui->versionSelect->clear();
    ui->versionSelect->addItem(tr("Any"));
    for(const auto &version : qAsConst(GameVersion::versionList))
        ui->versionSelect->addItem(version);
}

void CurseforgeModBrowser::on_searchButton_clicked()
{
    currentName = ui->searchText->text();
    getModList(currentName);
}

void CurseforgeModBrowser::onSliderChanged(int i)
{
    if(i == ui->modListWidget->verticalScrollBar()->maximum()){
        currentIndex += 20;
        getModList(currentName, currentIndex);
    }
}

void CurseforgeModBrowser::getModList(QString name, int index)
{
    if(!index) currentIndex = 0;
    ui->searchButton->setText(tr("Searching..."));
    ui->searchButton->setEnabled(false);

    GameVersion gameVersion = ui->versionSelect->currentIndex()? GameVersion(ui->versionSelect->currentText()) : GameVersion::ANY;
    auto sort = ui->sortSelect->currentIndex();

    CurseforgeAPI::searchMods(gameVersion, index, name, sort, [=](const QList<CurseforgeModInfo> &infoList){
        ui->searchButton->setText(tr("&Search"));
        ui->searchButton->setEnabled(true);

        //new search
        if(currentIndex == 0){
            for(auto mod : qAsConst(modList))
                mod->deleteLater();
            modList.clear();
            for(int i = 0; i < ui->modListWidget->count(); i++)
                ui->modListWidget->itemWidget(ui->modListWidget->item(i))->deleteLater();
            ui->modListWidget->clear();
        }

        //show them
        for(const auto &info : qAsConst(infoList)){
            auto curseforgeMod = new CurseforgeMod(this, accessManager, info);
            modList.append(curseforgeMod);

            auto *listItem = new QListWidgetItem();
            listItem->setSizeHint(QSize(500, 100));
            auto version = ui->versionSelect->currentIndex()? GameVersion(ui->versionSelect->currentText()): GameVersion::ANY;
            auto loaderType = ui->loaderSelect->currentIndex()? ui->loaderSelect->currentText() : "";
            auto fileInfo = curseforgeMod->getModInfo().getFileInfo(version, loaderType);
            auto modItemWidget = new CurseforgeModItemWidget(ui->modListWidget, curseforgeMod, fileInfo);
            ui->modListWidget->addItem(listItem);
            ui->modListWidget->setItemWidget(listItem, modItemWidget);
            setItemHidden(listItem, curseforgeMod->getModInfo());
            if(!listItem->isHidden())
                curseforgeMod->downloadThumbnail();

        }

    });
}

void CurseforgeModBrowser::setItemHidden(QListWidgetItem *item, const CurseforgeModInfo &modInfo)
{
    int index = ui->loaderSelect->currentIndex();
    switch (index) {
    //any
    case 0:
        item->setHidden(false);
        break;
    //fabric
    case 1:
        item->setHidden(!modInfo.isFabricMod());
        break;
    //forge
    case 2:
        item->setHidden(!modInfo.isForgeMod());
        break;
    //rift
    case 3:
        item->setHidden(!modInfo.isRiftMod());
        break;
    }
}

void CurseforgeModBrowser::on_modListWidget_doubleClicked(const QModelIndex &index)
{
    auto mod = modList.at(index.row());
    auto dialog = new CurseforgeModInfoDialog(this, mod);
    dialog->show();
}


void CurseforgeModBrowser::on_versionSelect_currentIndexChanged(int)
{
    getModList(currentName);
}


void CurseforgeModBrowser::on_sortSelect_currentIndexChanged(int)
{
    getModList(currentName);
}


void CurseforgeModBrowser::on_loaderSelect_currentIndexChanged(int)
{
    for(int i = 0; i < ui->modListWidget->count(); i++){
        auto item = ui->modListWidget->item(i);
        auto b = item->isHidden();
        auto mod = modList.at(i);
        setItemHidden(item, mod->getModInfo());
        if(b && !item->isHidden() && mod->getModInfo().getThumbnailBytes().isEmpty())
            mod->downloadThumbnail();
    }
}
