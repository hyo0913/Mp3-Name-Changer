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
    bool isValid() const;

    QString renamedFileName() const;
    void setRenamedFileName(const QString &fileName);

private:
    QFileInfo m_fileInfo;
    QString m_renamedFileName;
};

#endif // FILENAMETREEWIDGETITEM_H
