#ifndef PAGESWITCHER_H
#define PAGESWITCHER_H

#include <QStackedWidget>
#include <QStandardItemModel>

class Browser;
class ExploreBrowser;
class DownloadBrowser;
class CurseforgeModBrowser;
class ModrinthModBrowser;
class OptifineModBrowser;
class ReplayModBrowser;
class LocalModBrowser;
class LocalModPath;

class PageSwitcher : public QStackedWidget
{
    Q_OBJECT
public:
    enum BrowserCategory{ Download, Explore, Local };
    explicit PageSwitcher(QWidget *parent = nullptr);

    void nextPage();
    void previesPage();

    void addDownloadPage();
    void addCurseforgePage();
    void addModrinthPage();
    void addOptiFinePage();
    void addReplayModPage();
    void addLocalPage(LocalModBrowser *browser);
    void addLocalPage(LocalModPath *path);

    void removeExplorePage(int index);
    void removeCurseforgePage();
    void removeModrinthPage();
    void removeOptiFinePage();
    void removeReplayModPage();

    LocalModBrowser *takeLocalModBrowser(int index);
    void removeLocalModBrowser(int index);

    DownloadBrowser *downloadBrowser() const;
    CurseforgeModBrowser *curseforgeModBrowser() const;
    ModrinthModBrowser *modrinthModBrowser() const;
    OptifineModBrowser *optifineModBrowser() const;
    ReplayModBrowser *replayModBrowser() const;
    const QList<ExploreBrowser *> &exploreBrowsers() const;
    const QList<LocalModBrowser *> &localModBrowsers() const;
    ExploreBrowser *exploreBrowser(int index) const;
    LocalModBrowser *localModBrowser(int index) const;
    QPair<int, int> currentCategoryPage() const;
    int currentCategory() const;
    int currentPage() const;
    Browser *currentBrowser() const;
    QStandardItemModel *model();
signals:
    void pageChanged(QModelIndex modelIndex);
public slots:
    void setCurrentIndex(int index);
    void setPage(int category, int page);
    void updateUi();
private:
    void removeExplorePage(ExploreBrowser *exploreBrowser);
    QStandardItemModel model_;
    QVector<int> pageCount_;

    DownloadBrowser *downloadBrowser_ = nullptr;
    CurseforgeModBrowser *curseforgeModBrowser_ = nullptr;
    ModrinthModBrowser *modrinthModBrowser_ = nullptr;
    OptifineModBrowser *optifineModBrowser_ = nullptr;
    ReplayModBrowser *replayModBrowser_ = nullptr;
    QList<ExploreBrowser *> exploreBrowsers_;
    QList<LocalModBrowser *> localModBrowsers_;
};

#endif // PAGESWITCHER_H
