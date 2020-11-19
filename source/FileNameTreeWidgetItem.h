#ifndef FILENAMETREEWIDGETITEM_H
#define FILENAMETREEWIDGETITEM_H

#include <QTreeWidgetItem>

#include <QFileInfo>

class FileNameTreeWidgetItem : public QTreeWidgetItem
{
public:
    FileNameTreeWidgetItem();
    ~FileNameTreeWidgetItem();

    QVariant data(int column, int role) const override;

    const QFileInfo &fileInfo() const { return m_fileInfo; }
    inline void setFileInfo(const QFileInfo &info) { m_fileInfo = info; }
    inline void setArtist(const QString &artist) { m_artist = artist; }
    inline void setTitle(const QString &title) { m_title = title; }
    inline bool isValid() const { return (m_fileInfo.isFile() && hasArtist() && hasTitle()); }
    inline bool hasArtist() const { return !m_artist.isEmpty(); }
    inline bool hasTitle() const { return !m_title.isEmpty(); }

    QString renamedFileName() const;

private:
    QFileInfo m_fileInfo;
    QString m_artist;
    QString m_title;
};

#endif // FILENAMETREEWIDGETITEM_H
