#include "FileNameTreeWidgetItem.h"

FileNameTreeWidgetItem::FileNameTreeWidgetItem()
{

}

FileNameTreeWidgetItem::~FileNameTreeWidgetItem()
{

}

QVariant FileNameTreeWidgetItem::data(int column, int role) const
{
    QVariant result;

    if( role == Qt::DisplayRole ) {
        if( column == 0 ) {
            result.setValue(m_fileInfo.fileName());
        } else if( column == 1 ) {
            if( isValid() ) {
                result.setValue(renamedFileName());
            } else {
                //result.setValue(QString("Failed to decode the file informations"));
            }
        }
    } else {
        result = QTreeWidgetItem::data(column, role);
    }

    return result;
}

bool FileNameTreeWidgetItem::isValid() const
{
    return !m_renamedFileName.isEmpty();
}

QString FileNameTreeWidgetItem::renamedFileName() const
{
    return m_renamedFileName;
}

void FileNameTreeWidgetItem::setRenamedFileName(const QString &fileName)
{
    m_renamedFileName = fileName;

    for( int i = 0; i < m_renamedFileName.length(); i++ ) {
        QChar temp = m_renamedFileName.at(i);
        if( temp == '<' || temp == '>' || temp == ':' || temp == '"' || temp == '/' || temp == '\\' ||
            temp == '|' || temp == '?' || temp == '*') {
            m_renamedFileName.replace(i, 1, '_');
        }
    }
}
