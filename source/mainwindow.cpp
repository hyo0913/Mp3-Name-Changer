#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QTextCodec>
#include <QThread>
#include <QProgressBar>

#include "FileNameTreeWidgetItem.h"

#include "taglib/fileref.h"
#include "taglib/tag.h"
#include "taglib/tbytevector.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->lineEditSrcFolder->setReadOnly(true);
    ui->lineEditDstFolder->setReadOnly(true);

    connect(ui->toolButtonSelectSrcFolder, SIGNAL(clicked()), this, SLOT(onToolButtonSelectSrcFolderClicked()));
    connect(ui->toolButtonSelectDstFolder, SIGNAL(clicked()), this, SLOT(onToolButtonSelectDstFolderClicked()));
    connect(ui->pushButtonLoad, SIGNAL(clicked()), this, SLOT(onPushButtonLoadClicked()));
    connect(ui->pushButtonRename, SIGNAL(clicked()), this, SLOT(onPushButtonRenameClicked()));
    connect(ui->pushButtonCopy, SIGNAL(clicked()), this, SLOT(onPushButtonCopyClicked()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::updateTreeWidgetColumnWidth()
{
    int width = ui->treeWidgetFileName->width();
    ui->treeWidgetFileName->setColumnWidth(0, width/2);
}

void MainWindow::onToolButtonSelectSrcFolderClicked()
{
    QString folder = QFileDialog::getExistingDirectory();
    if( folder.isEmpty() ) { return; }

    ui->lineEditSrcFolder->setText(folder);

    QString dstFolder = ui->lineEditDstFolder->text();
    if( dstFolder.isEmpty() )
    {
        dstFolder = folder+"_Copy";
        ui->lineEditDstFolder->setText(dstFolder);
    }
}

void MainWindow::onToolButtonSelectDstFolderClicked()
{
    QString folder = QFileDialog::getExistingDirectory();
    if( folder.isEmpty() ) { return; }

    ui->lineEditDstFolder->setText(folder);
}

void MainWindow::onPushButtonLoadClicked()
{
    ui->treeWidgetFileName->clear();

    QString srcFolder = ui->lineEditSrcFolder->text();

    QDir dir(srcFolder);
    if( !dir.exists() ) {
        QMessageBox::critical(this, "", tr("Source folder is non-exist."), QMessageBox::Close);
        return;
    }

    QStringList fileFormats;
    fileFormats << "*.mp3";
    QFileInfoList fileInfos = dir.entryInfoList(fileFormats, QDir::Files|QDir::NoDotAndDotDot);

    for( int i = 0; i < fileInfos.count(); i++ )
    {
        QFileInfo info = fileInfos.at(i);

        FileNameTreeWidgetItem* item = new FileNameTreeWidgetItem();
        ui->treeWidgetFileName->addTopLevelItem(item);

        item->setFileInfo(info);
    }

    updateTreeWidgetColumnWidth();
}

void MainWindow::onPushButtonRenameClicked()
{
    QList<QTreeWidgetItem*> selectedItems = ui->treeWidgetFileName->selectedItems();
    int count = selectedItems.count();
    if( count <= 0 ) {
        // do all?

        for( int i = 0; i < ui->treeWidgetFileName->topLevelItemCount(); i++ ) {
            selectedItems << ui->treeWidgetFileName->topLevelItem(i);
        }
    }

    count = selectedItems.count();
    for( int j = 0; j < count; j++ ) {
        FileNameTreeWidgetItem* item = static_cast<FileNameTreeWidgetItem*>(selectedItems.at(j));
        if( item == NULL ) { continue; }

        const QFileInfo& info = item->fileInfo();

        TagLib::FileName fileName(info.absoluteFilePath().toStdWString().data());
        TagLib::FileRef file(fileName);
        if( file.isNull() ) { continue; }

        TagLib::String artist_string = file.tag()->artist();
        TagLib::String title_string = file.tag()->title();
        QString artist;
        QString title;

        if( ui->comboBoxCodec->currentText() == "Default" )
        {
            artist = QString::fromStdWString(artist_string.toWString());
            title = QString::fromStdWString(title_string.toWString());
        }
        else if( ui->comboBoxCodec->currentText() == "EUC-KR" )
        {
            QTextCodec* codecEucKr = QTextCodec::codecForName("EUC-KR");

            TagLib::ByteVector artistByteVectorEucKr = artist_string.data(TagLib::String::Latin1);
            artist = codecEucKr->toUnicode(artistByteVectorEucKr.data(), artistByteVectorEucKr.size());

            TagLib::ByteVector titleByteVectorEucKr = title_string.data(TagLib::String::Latin1);
            title = codecEucKr->toUnicode(titleByteVectorEucKr.data(), titleByteVectorEucKr.size());
        }

        item->setArtist(artist);
        item->setTitle(title);
    }

    ui->treeWidgetFileName->update();
}

void MainWindow::onPushButtonCopyClicked()
{
    QString dstFolder = ui->lineEditDstFolder->text();

    QDir dir(dstFolder);
    if( !dir.exists() ) {
        dir.mkdir(dstFolder);
    }

    QThread* thread = new QThread();
    QWidget* widget = new QWidget();
    QHBoxLayout* layout = new QHBoxLayout();
    QProgressBar* progressBar = new QProgressBar();

    layout->addWidget(progressBar);
    widget->setLayout(layout);

    progressBar->setMinimum(0);
    progressBar->setMaximum(ui->treeWidgetFileName->topLevelItemCount());

    thread->start();
    widget->moveToThread(thread);

    widget->setAttribute(Qt::WA_DeleteOnClose);
    widget->show();

    connect(widget, SIGNAL(destroyed()), thread, SLOT(deleteLater()));

    for( int i = 0; i < ui->treeWidgetFileName->topLevelItemCount(); i++ )
    {
        const FileNameTreeWidgetItem* item = static_cast<FileNameTreeWidgetItem*>(ui->treeWidgetFileName->topLevelItem(i));
        if( item == NULL ) { continue; }

        const QFileInfo &fileInfo = item->fileInfo();

        QFile::copy(fileInfo.absoluteFilePath(), QString("%1/%2.%3").arg(dstFolder).arg(item->renamedFileName()).arg(fileInfo.suffix()));

        progressBar->setValue(i+1);
    }
}

