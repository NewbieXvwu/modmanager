#include "curseforgemodinfo.h"

#include "util/tutil.hpp"

CurseforgeModInfo::CurseforgeModInfo()
{

}

CurseforgeModInfo CurseforgeModInfo::fromVariant(const QVariant &variant)
{
    CurseforgeModInfo curseforgeModInfo;

    curseforgeModInfo.id = value(variant, "id").toInt();
    curseforgeModInfo.name = value(variant, "name").toString();
    curseforgeModInfo.summary = value(variant, "summary").toString();
    curseforgeModInfo.websiteUrl = value(variant, "websiteUrl").toUrl();
    curseforgeModInfo.downloadCount = value(variant, "downloadCount").toInt();
    curseforgeModInfo.modLoaders = value(variant, "modLoaders").toStringList();

    //authors
    auto authorsList = value(variant, "authors").toList();
    for(const auto &author : qAsConst(authorsList))
        curseforgeModInfo.authors << value(author, "name").toString();

    //thumbnail image
    auto attachmentsList = value(variant, "attachments").toList();
    if(!attachmentsList.isEmpty())
        curseforgeModInfo.thumbnailUrl = value(attachmentsList.at(0), "thumbnailUrl").toUrl();

    //latest file url
    auto latestFileList = value(variant, "latestFiles").toList();
    for(const auto &variant : latestFileList)
        curseforgeModInfo.fileInfoList.append(CurseforgeFileInfo::fromVariant(variant));

    return curseforgeModInfo;
}

int CurseforgeModInfo::getId() const
{
    return id;
}

const QString &CurseforgeModInfo::getName() const
{
    return name;
}

const QString &CurseforgeModInfo::getSummary() const
{
    return summary;
}

const QUrl &CurseforgeModInfo::getWebsiteUrl() const
{
    return websiteUrl;
}

const QStringList &CurseforgeModInfo::getAuthors() const
{
    return authors;
}

const QUrl &CurseforgeModInfo::getThumbnailUrl() const
{
    return thumbnailUrl;
}

const QByteArray &CurseforgeModInfo::getThumbnailBytes() const
{
    return thumbnailBytes;
}

const QString &CurseforgeModInfo::getDescription() const
{
    return description;
}

int CurseforgeModInfo::getDownloadCount() const
{
    return downloadCount;
}

const QStringList &CurseforgeModInfo::getModLoaders() const
{
    return modLoaders;
}

bool CurseforgeModInfo::isFabricMod() const
{
    return modLoaders.contains("Fabric");
}

bool CurseforgeModInfo::isForgeMod() const
{
    return modLoaders.contains("Forge");
}

bool CurseforgeModInfo::isRiftMod() const
{
    return modLoaders.contains("Rift");
}

const QList<CurseforgeFileInfo> &CurseforgeModInfo::getFileInfoList() const
{
    return fileInfoList;
}

std::optional<CurseforgeFileInfo> CurseforgeModInfo::getFileInfo(const GameVersion &version, const QString &modLoader) const
{
    for(auto info : fileInfoList){
        if((version == GameVersion::ANY || info.getGameVersions().contains(version) || info.getGameVersions().isEmpty()) &&
                (modLoader == "Any" || info.getModLoaders().contains(modLoader) || info.getModLoaders().empty()))
            return {info};
    }
    return std::nullopt;
}