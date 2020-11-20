#ifndef FILECOPIER_H
#define FILECOPIER_H

#include <QObject>

typedef struct {
    QString Src;
    QString Dst;
} CopyInfo;

class FileCopier : public QObject
{
    Q_OBJECT

public:
    FileCopier();
    ~FileCopier();

    void setCopyInfos(const QList<CopyInfo> &infos);

public slots:
    void startCopy();

private:
    int m_timerId;
    int m_copyInfoIdx;
    QList<CopyInfo> m_copyInfos;

    void timerEvent(QTimerEvent *event);

signals:
    void changedProgressRange(int, int);
    void changedProgressValue(int);
    void finished();
};

#endif // FILECOPIER_H
