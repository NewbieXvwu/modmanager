#include "localmodfile.h"

#include <QDir>
#include <QCryptographicHash>
#include <MurmurHash2.h>

const QStringList LocalModFile::availableSuffix{ "jar", "old", "disabled"};

LocalModFile::LocalModFile(QObject *parent, const QString &path) :
    QObject(parent),
    path_(path),
    fileInfo_(path)
{}

bool LocalModFile::loadInfo()
{
    if(type() == NotMod) return false;
    QFile modFile(path_);

    //file open error
    if(!modFile.open(QIODevice::ReadOnly))
        return false;
    QByteArray fileContent = modFile.readAll();
    modFile.close();

    //sha1
    sha1_ = QCryptographicHash::hash(fileContent, QCryptographicHash::Sha1).toHex();

    //exclude some bytes for murmurhash
    QByteArray filteredFileContent;
    for (const char& b : qAsConst(fileContent)){
        if (b == 0x9 || b == 0xa || b == 0xd || b == 0x20) continue;
        filteredFileContent.append(b);
    }
    murmurhash_ = QByteArray::number(MurmurHash2(filteredFileContent.constData(), filteredFileContent.length(), 1));

    //load fabric mod
    if(fabricModInfoList_ = FabricModInfo::fromZip(path_); !fabricModInfoList_.isEmpty()){
        loaderType_ = ModLoaderType::Fabric;
        return true;
    }

    //load forge mod
    if(forgeModInfoList_ = ForgeModInfo::fromZip(path_); !forgeModInfoList_.isEmpty()){
        loaderType_ = ModLoaderType::Forge;
        return true;
    }

    //no available mod info
    return false;
}

bool LocalModFile::remove()
{
    QFile file(path_);
    bool bl = file.remove();
    if(bl){
        path_.clear();
        fileInfo_.setFile("");
    }
    emit fileChanged();
    return bl;
}

bool LocalModFile::rename(const QString newBaseName)
{
    auto [ baseName, suffix ] = baseNameFullSuffix();
    QFile file(path_);
    auto newPath = QDir(fileInfo_.absolutePath()).absoluteFilePath(newBaseName + suffix);
    if(file.rename(newPath)){
        path_ = newPath;
        fileInfo_.setFile(path_);
            emit fileChanged();
        return true;
    } else
    return false;
}

bool LocalModFile::addOld()
{
    QFile file(path_);
    path_.append(".old");
    fileInfo_.setFile(path_);

    bool bl = file.rename(path_);
    if(bl)
        emit fileChanged();
    return bl;
}

bool LocalModFile::removeOld()
{
    QFile file(path_);
    path_.remove(".old");
    fileInfo_.setFile(path_);

    bool bl = file.rename(path_);
    if(bl)
        emit fileChanged();
    return bl;
}

const QString &LocalModFile::sha1() const
{
    return sha1_;
}

const QString &LocalModFile::murmurhash() const
{
    return murmurhash_;
}

std::tuple<QString, QString> LocalModFile::baseNameFullSuffix() const
{
    auto fileName = fileInfo_.fileName();
    QFileInfo fileInfo(fileName);
    QString fullSuffix;
    while(true){
        fileInfo.setFile(fileName);
        auto suffix = fileInfo.suffix();
        if(!availableSuffix.contains(suffix))
            break;
        fileName = fileInfo.completeBaseName();
        fullSuffix.prepend("." + suffix);
    }
    return { fileName, fullSuffix };
}

LocalModFile::FileType LocalModFile::type() const
{
    if(fileInfo_.suffix() == "jar")
        return Normal;
    else if(fileInfo_.suffix() == "old")
        return Old;
    else if(fileInfo_.suffix() == "disabled")
        return Disabled;
    else if(fileInfo_.suffix() == "downloading")
        return Downloading;
    else
        return NotMod;
}

const QString &LocalModFile::path() const
{
    return path_;
}

const QFileInfo &LocalModFile::fileInfo() const
{
    return fileInfo_;
}

const CommonModInfo *LocalModFile::commonInfo() const
{
    if(loaderType_ == ModLoaderType::Fabric)
        return &fabricModInfoList_.first();
    else if(loaderType_ == ModLoaderType::Forge)
        return &forgeModInfoList_.first();
    else
        assert(false);//should't be here
}

FabricModInfo LocalModFile::fabric() const
{
    assert(loaderType_ == ModLoaderType::Fabric);
    return fabricModInfoList_.first();
}

ForgeModInfo LocalModFile::forge() const
{
    assert(loaderType_ == ModLoaderType::Forge);
    return forgeModInfoList_.first();
}

ModLoaderType::Type LocalModFile::loaderType() const
{
    return loaderType_;
}

const QList<FabricModInfo> &LocalModFile::fabricModInfoList() const
{
    return fabricModInfoList_;
}

const QList<ForgeModInfo> &LocalModFile::forgeModInfoList() const
{
    return forgeModInfoList_;
}
