#include "FileCopier.h"

#include <QFile>
#include <QTimerEvent>

FileCopier::FileCopier() :
    m_timerId(0),
    m_copyInfoIdx(0)
{

}

FileCopier::~FileCopier()
{

}

void FileCopier::setCopyInfos(const QList<CopyInfo> &infos)
{
    m_copyInfos = infos;
}

void FileCopier::startCopy()
{
    emit changedProgressRange(0, m_copyInfos.count());
    emit changedProgressValue(0);

    if( m_copyInfos.count() > 0 ) {
        m_timerId = this->startTimer(1);
        m_copyInfoIdx = 0;
    } else {
        emit finished();
    }
}

void FileCopier::timerEvent(QTimerEvent *event)
{
    if( m_timerId != event->timerId() ) { return; }

    int count = m_copyInfos.count();
    if( m_copyInfoIdx < count )
    {
        const CopyInfo& info = m_copyInfos.at(m_copyInfoIdx);
        QFile::copy(info.Src, info.Dst);
        emit changedProgressValue(++m_copyInfoIdx);
    }
    else if( m_copyInfoIdx == count )
    {
        this->killTimer(m_timerId);
        m_timerId = 0;
        m_copyInfoIdx = 0;
        emit finished();
    }
}
