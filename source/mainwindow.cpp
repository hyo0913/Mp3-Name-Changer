#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QTextCodec>
#include <QThread>
#include <QProgressBar>

#include "FileNameTreeWidgetItem.h"
#include "FileCopier.h"

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

        item->setRenamedFileName(QString("%1 - %2.%3").arg(artist).arg(title).arg(info.suffix()));
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

    QList<CopyInfo> copyInfos;
    CopyInfo copyInfo;
    for( int i = 0; i < ui->treeWidgetFileName->topLevelItemCount(); i++ )
    {
        const FileNameTreeWidgetItem* item = static_cast<FileNameTreeWidgetItem*>(ui->treeWidgetFileName->topLevelItem(i));
        if( item == NULL ) { continue; }

        const QFileInfo &fileInfo = item->fileInfo();

        copyInfo.Src = fileInfo.absoluteFilePath();
        copyInfo.Dst = QString("%1/%2").arg(dstFolder).arg(item->renamedFileName());

        copyInfos << copyInfo;
    }

    QDialog* dialogTemp = new QDialog();

    QProgressBar* progressBar = new QProgressBar();
    progressBar->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);

    dialogTemp->setLayout(new QVBoxLayout());
    dialogTemp->layout()->addWidget(progressBar);
    dialogTemp->adjustSize();
    dialogTemp->resize(dialogTemp->width()*2, dialogTemp->height());

    dialogTemp->setWindowTitle(tr("Copying..."));
    dialogTemp->move(this->pos()+this->rect().center()-dialogTemp->rect().center());
    dialogTemp->setMaximumSize(dialogTemp->size());

    QThread* thread = new QThread();
    FileCopier* copier = new FileCopier();
    copier->moveToThread(thread);

    copier->setCopyInfos(copyInfos);

    connect(thread, SIGNAL(started()), copier, SLOT(startCopy()));
    connect(copier, SIGNAL(changedProgressRange(int,int)), progressBar, SLOT(setRange(int,int)));
    connect(copier, SIGNAL(changedProgressValue(int)), progressBar, SLOT(setValue(int)));
    connect(copier, SIGNAL(finished()), dialogTemp, SLOT(accept()));

    thread->start();

    dialogTemp->exec();

    thread->quit();
    thread->wait(10000);

    delete copier;
    delete thread;
    delete dialogTemp;
}

