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

QString FileNameTreeWidgetItem::renamedFileName() const
{
    return QString("%1 - %2").arg(m_artist).arg(m_title);
}
