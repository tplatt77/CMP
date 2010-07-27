///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010, Michael A. Jackson. BlueQuartz Software
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////
#include "QImageProcessingInputFrame.h"

//-- Qt Includes
#include <QtCore/QFileInfo>
#include <QtCore/QFile>
#include <QtCore/QDir>

#include <QtGui/QListWidget>
#include <QtGui/QStringListModel>

// Our Project wide includes
//#include "QtSupport/QFileCompleter.h"
//#include "QtSupport/ImageGraphicsDelegate.h"
#include "QtSupport/ProcessQueueController.h"
#include "QtSupport/ProcessQueueDialog.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QImageProcessingInputFrame::QImageProcessingInputFrame(QWidget *parent) :
QFrame(parent),
m_ProxyModel(NULL),
m_OutputExistsCheck(false),
m_QueueController(NULL),
m_QueueDialog(NULL),
#if defined(Q_WS_WIN)
m_OpenDialogLastDirectory("C:\\")
#else
m_OpenDialogLastDirectory("~/")
#endif
{
  m_QueueDialog = new ProcessQueueDialog(this);
  m_QueueDialog->setVisible(false);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QImageProcessingInputFrame::~QImageProcessingInputFrame()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void QImageProcessingInputFrame::setWidgetListEnabled(bool b)
{
  foreach (QWidget* w, m_WidgetList)
    {
      w->setEnabled(b);
    }
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool QImageProcessingInputFrame::verifyOutputPathParentExists(QString outFilePath, QLineEdit* lineEdit)
{
  QFileInfo fileinfo(outFilePath);
  QDir parent(fileinfo.dir());
  return parent.exists();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool QImageProcessingInputFrame::verifyPathExists(QString outFilePath, QLineEdit* lineEdit)
{
  QFileInfo fileinfo(outFilePath);
  if (false == fileinfo.exists())
  {
    lineEdit->setStyleSheet("border: 1px solid red;");
  }
  else
  {
    lineEdit->setStyleSheet("");
  }
  return fileinfo.exists();
}



// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void QImageProcessingInputFrame::populateFileTable(QLineEdit* sourceDirectoryLE, QListView *fileListView)
{
  if (NULL == m_ProxyModel)
  {
    m_ProxyModel = new QSortFilterProxyModel(this);
  }

  QDir sourceDir(sourceDirectoryLE->text());
  sourceDir.setFilter(QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);
  QStringList strList = sourceDir.entryList();
  QAbstractItemModel* strModel = new QStringListModel(strList, this->m_ProxyModel);
  m_ProxyModel->setSourceModel(strModel);
  m_ProxyModel->setDynamicSortFilter(true);
  m_ProxyModel->setFilterKeyColumn(0);
  fileListView->setModel(m_ProxyModel);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AIMImage::Pointer QImageProcessingInputFrame::loadImage(QString filePath)
{
//  std::cout << " loadImage(): filePath: " << filePath.toStdString() << std::endl;
  QImage image;
  AIMImage::Pointer aimImage = AIMImage::NullPointer();
  if (filePath.isEmpty() == false)
  {
    image = QImage(filePath);
    if (image.isNull() == true)
    {
      QString m("Error loading image from ");
      m.append(filePath);
      qDebug(m.toAscii());
      return aimImage;
    }
    QVector<QRgb> colorTable(256);
    for (quint32 i = 0; i < 256; ++i)
    {
      colorTable[i] = qRgb(i, i, i);
    }
    image.setColorTable(colorTable);
    aimImage = convertQImageToGrayScaleAIMImage(image);
    if (NULL == aimImage.data())
    {
      return aimImage;
    }
  }
  return aimImage;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AIMImage::Pointer QImageProcessingInputFrame::convertQImageToGrayScaleAIMImage(QImage image)
{
  AIMImage::Pointer aimImage = AIMImage::New();
  quint8* oImage = aimImage->allocateImageBuffer(image.width(), image.height(), true);
  if (NULL == oImage)
  {
    return AIMImage::NullPointer();
  }

  // Copy the QImage into the AIMImage object, converting to gray scale as we go.
  qint32 height = image.height();
  qint32 width = image.width();
  QRgb rgbPixel;
  int gray;
  qint32 index;
  for (qint32 y=0; y<height; y++) {
    for (qint32 x =0; x<width; x++) {
      index = (y *  width) + x;
      rgbPixel = image.pixel(x, y);
      gray = qGray(rgbPixel);
      oImage[index] = static_cast<unsigned char>(gray);
    }
  }
  return aimImage;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QStringList QImageProcessingInputFrame::generateInputFileList()
{
  QStringList list;
  int count = this->m_ProxyModel->rowCount();
  // this->fileListView->selectAll();
  QAbstractItemModel* sourceModel = this->m_ProxyModel->sourceModel();
  for (int i = 0; i < count; ++i)
  {
    QModelIndex proxyIndex = this->m_ProxyModel->index(i,0);
    QModelIndex sourceIndex = this->m_ProxyModel->mapToSource(proxyIndex);
    list.append( sourceModel->data(sourceIndex, 0).toString() );
  }
  return list;
}
