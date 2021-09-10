#ifndef CONFIG_H
#define CONFIG_H

#include <QSettings>

#define getterAndSetter(name, type, key, defaultValue) \
    void set##name(const decltype(QVariant().to##type()) &key){\
        setValue(#key, key);\
    }\
    \
    decltype(QVariant().to##type()) get##name() const{\
        return value(#key, defaultValue).to##type();\
    }

#define getterAndSetter_prefix(name, type, key, prefix, defaultValue) \
    void set##name(const decltype(QVariant().to##type()) &key){\
        setValue(QString(prefix) + "/" + #key, key);\
    }\
    \
    decltype(QVariant().to##type()) get##name() const{\
        return value(QString(prefix) + "/" + #key, defaultValue).to##type();\
    }

class Config : private QSettings
{
public:
    explicit Config();

    //General
    getterAndSetter(DownloadPath, String, downloadPath, "");
    getterAndSetter(CommonPath, String, commonPath, "");
    enum VersionMatchType{ MinorVersion, MajorVersion };
    getterAndSetter(VersionMatch, Int, versionMatch, MinorVersion);
    getterAndSetter(AutoSearchOnWebsites, Bool, autoSearchWebOnSites, true);
    getterAndSetter(AutoCheckUpdate, Bool, autoCheckUpdate, true)
    getterAndSetter(UseCurseforgeUpdate, Bool, useCurseforgeUpdate, true)
    getterAndSetter(UseModrinthUpdate, Bool, useModrinthUpdate, true)
    enum PostUpdateType{ Delete, Keep, DoNothing };
    getterAndSetter(PostUpdate, Int, postUpdate, Keep);

    //Network
    getterAndSetter(ThreadCount, Int, threadCount, 8)
    getterAndSetter(DownloadCount, Int, downloadCount, 8)

    //Path List
    getterAndSetter(LocalPathList, List, localPathList, QVariant())

};

#endif // CONFIG_H