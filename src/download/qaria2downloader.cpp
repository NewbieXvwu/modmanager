#include "qaria2downloader.h"

#include <QDebug>

#include "qaria2.h"

QAria2Downloader::QAria2Downloader(aria2::A2Gid gid) :
    AbstractDownloader(QAria2::qaria2()),
    gid_(gid)
{}

QAria2Downloader::QAria2Downloader(const QUrl &url, const QString &path) :
    AbstractDownloader(QAria2::qaria2(), url, path)
{}

void QAria2Downloader::deleteDownloadHandle()
{
    aria2::deleteDownloadHandle(handle_);
    handle_ = nullptr;
}

void QAria2Downloader::update()
{
    if(!handle_)
        handle_ = aria2::getDownloadHandle(QAria2::qaria2()->session(), gid_);
    if(!handle_) return;
    status();
    qDebug() << "progress:" << handle_->getCompletedLength() << handle_->getTotalLength();
    qDebug() << "speed:" << handle_->getDownloadSpeed() << handle_->getUploadSpeed();
    emit downloadProgress(handle_->getCompletedLength(), handle_->getTotalLength());
    emit downloadSpeed(handle_->getDownloadSpeed(), handle_->getUploadSpeed());
}

aria2::DownloadStatus QAria2Downloader::status()
{
    if(handle_ && status_ != handle_->getStatus()){
        status_ = handle_->getStatus();
        emit statusChanged(status_);
    }
    return status_;
}

void QAria2Downloader::setEvent(const aria2::DownloadEvent &event)
{
    qDebug() << gid_ << "event:" << event;
    switch (event) {
    case aria2::EVENT_ON_DOWNLOAD_START:
        status_ = aria2::DOWNLOAD_ACTIVE;
        break;
    case aria2::EVENT_ON_DOWNLOAD_PAUSE:
        status_ = aria2::DOWNLOAD_PAUSED;
        break;
    case aria2::EVENT_ON_DOWNLOAD_STOP:
        status_ = aria2::DOWNLOAD_REMOVED;
        break;
    case aria2::EVENT_ON_DOWNLOAD_COMPLETE:
        status_ = aria2::DOWNLOAD_COMPLETE;
        emit finished();
        break;
    case aria2::EVENT_ON_DOWNLOAD_ERROR:
        status_ = aria2::DOWNLOAD_ERROR;
        break;
    case aria2::EVENT_ON_BT_DOWNLOAD_COMPLETE:
        status_ = aria2::DOWNLOAD_COMPLETE;
        break;
    }
    emit statusChanged(status_);
}

void QAria2Downloader::setGid(aria2::A2Gid newGid)
{
    gid_ = newGid;
}
