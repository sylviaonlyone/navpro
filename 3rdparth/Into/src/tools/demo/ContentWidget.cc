/* This file is part of Into demo.
 * Copyright (C) 2003-2011 Intopii.
 * All rights reserved.
 *
 * Created: Mon Mar 15 2010 by Lasse Raiha
 *
 * $Revision:$
 *
 * IMPORTANT LICENSING INFORMATION
 *
 * Into is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License version 3
 * as published by the Free Software Foundation.
 *
 * Into is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public
 * License along with this program. If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include <ContentWidget.h>
#include <QRect>
#include <QDesktopWidget>
#include <QDesktopServices>
#include <QTextCursor>
#include <QCoreApplication>
#include <QApplication>
#include <QProcess>
#include <QLinearGradient>
#include <QPainter>
#include <QtDebug>
#include <QSettings>

static QPixmap createShadow(const QPixmap& pixmap, const QColor& backgroundColor)
{
  int iWidth = pixmap.width();
  int iHeight = pixmap.height()/2;
  
  QImage shadowImage(pixmap.toImage().mirrored(false, true).copy(0,0,iWidth, iHeight));
  
  QColor color1 = backgroundColor;
  QColor color2 = backgroundColor;
  color1.setAlpha(230);
  color2.setAlpha(245);
  
  QLinearGradient gradient(QPointF(0,0), QPointF(0,iHeight));
  gradient.setColorAt(0, color1);
  gradient.setColorAt(0.3, color2);
  gradient.setColorAt(1, backgroundColor);

  QPainter p(&shadowImage);
  p.fillRect(0, 0, iWidth, iHeight, gradient);
  p.end();

  return QPixmap::fromImage(shadowImage);
}


ContentWidget::ContentWidget(QWidget *parent, Qt::WindowFlags flags) :
  QWidget(parent, flags),
  _mode(Debug),
  _bMacOS(false)
{
  setupUi(this);
  
  connect(_pQuitButton, SIGNAL(clicked()), this, SLOT(quitButtonClicked()));
  connect(_pLaunchButton, SIGNAL(clicked()), this, SLOT(executeCurrentDemo()));
  connect(_pDocumentationButton, SIGNAL(clicked()), this, SLOT(openCurrentDocumentation()));

  QDir applicationDirPath = QDir(QCoreApplication::applicationDirPath());
  QString path = applicationDirPath.path();
  if (path.contains("release"))
    _mode = Release;
  else if (path.contains("debug"))
    _mode = Debug;
  else
    _mode = Production;
    
  _bMacOS = applicationDirPath.dirName() == "MacOS";
  if (_mode == Production)
    {
      if (QFile::exists(QString("%1/demos/README").arg(path)))
        _demoRootDir = QDir(QString("%1/demos").arg(path));
      else if (_bMacOS)
        {
          for (int i=0; i<3; i++)
            applicationDirPath.cdUp();
          _demoRootDir = QDir(applicationDirPath.path());
        }
      else
        _demoRootDir = QDir(QString("%1").arg(path));
    }
  else
    {
      int count = _bMacOS ? 6 : 3;
      for (int i=0; i<count; i++)
        applicationDirPath.cdUp();

      _demoRootDir = QDir(QString("%1/demos").arg(applicationDirPath.path()));
    }

  QSettings settings("Intopii", "Demos");
  settings.setValue("imageDirectory", QString("%1/images").arg(_demoRootDir.path()));
  settings.setValue("defaultImageDirectory", QString("%1/images").arg(_demoRootDir.path()));
  settings.setValue("defaultVideoDirectory", QString("%1/videos").arg(_demoRootDir.path()));
  
  _currentDir = _demoRootDir;
  _demoDir = _demoRootDir;

  _pixmapFolder = QPixmap(":/icons/folder.png");
  _pixmapQuit = QPixmap(":/icons/application-exit.png");
  _pixmapBack = QPixmap(":/icons/go-previous.png");
  _pixmapLaunch = QPixmap(":/icons/system-run.png");

  initDirectory();
}

ContentWidget::~ContentWidget()
{
  //delete all documents
  while (_lstDocuments.size() > 0)
    delete _lstDocuments.takeFirst();
}

void ContentWidget::setActionButtonsVisibility(bool visible)
{
  _pLaunchButton->setVisible(visible);
  _pDocumentationButton->setVisible(visible);
  updateImageSize();
}

void ContentWidget::initDirectory()
{
  bool bSubDemos = isThereSubDemoDirectories(_demoDir);
  setActionButtonsVisibility(!bSubDemos);

  if (bSubDemos)
    {
      _currentDir = _demoDir;
      
      //get subdirs
      QStringList lstDirectories = _currentDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);

      //Remove dirs where is not README-file
      lstDirectories = removeExtraDirectories(_demoDir, lstDirectories);
      
      //remove/delete old demo buttons
      while (_lstDemoButtons.size() > 0)
        {
          PushButton *pButton = _lstDemoButtons.takeFirst();
          _pDirectoryLayout->removeWidget(pButton);
          delete pButton;
        }

      QString path = _currentDir.path();
      
      //create new demo buttons
      for (int i=0; i<lstDirectories.size(); i++)
        {
          QDir tempDir(QString("%1/%2").arg(path, lstDirectories[i]));
          
          PushButton *pDemoButton = new PushButton;
          pDemoButton->setName(getTitle(path, lstDirectories[i]));
          pDemoButton->setDirname(lstDirectories[i]);
          pDemoButton->setPixmap(isThereSubDemoDirectories(tempDir) ? _pixmapFolder : _pixmapLaunch);

          _lstDemoButtons << pDemoButton;
          _pDirectoryLayout->insertWidget(_pDirectoryLayout->count()-1, pDemoButton);
          
          connect(pDemoButton, SIGNAL(clicked()), this, SLOT(demoButtonClicked()));
        }
      
      //update text of the quit/back button
      if (_currentDir == _demoRootDir)
        {
          _pQuitButton->setName(tr("Quit"));
          _pQuitButton->setPixmap(_pixmapQuit);
        }
      else
        {
          _pQuitButton->setPixmap(_pixmapBack);
          _pQuitButton->setName(tr("Back"));
        }
    }

  updateInfo(getCurrentDocument());
}

void ContentWidget::openCurrentDocumentation()
{
  QDesktopServices::openUrl(QUrl(QString("http://doc.intopii.com/%1").arg(getTitle(_demoDir.path(), _demoDir.dirName()))));
}

void ContentWidget::executeCurrentDemo()
{
  QString strFileName = QString("%1/").arg(_demoDir.path());
  QString strDemoName = _demoDir.dirName();
  
  if (_mode != Production)
    strFileName.append(QString("%1").arg(_mode == Release ? "release/" : "debug/"));
  
  if (_bMacOS)
    strFileName.append(QString("%1.app/Contents/MacOS/").arg(strDemoName));

  strFileName.append(strDemoName);
  
#ifdef Q_OS_WIN
  strFileName += ".exe";
#endif
  QFileInfo file(strFileName);
  if (file.exists() && file.isExecutable())
    QProcess::startDetached(file.absoluteFilePath());
}

void ContentWidget::updateInfo(Document *document)
{
  _pTitleLabel->setText(document->strTitle);
  _pTextBrowser->setPlainText(document->strDescription);
  _pImageLabel->setPixmap(document->pixmap);
  _pMirrorLabel->setPixmap(createShadow(document->pixmap, _pMainFrame->palette().color(_pMainFrame->backgroundRole())));
  _imageRefSize = document->pixmap.size();
  updateImageSize();
}

void ContentWidget::resizeEvent(QResizeEvent *e)
{
  QWidget::resizeEvent(e);

  updateImageSize();
}

void ContentWidget::updateImageSize()
{
  QSize scaledSize(_pMainFrame->width() / 3, _pTextBrowser->height() / 2);
  
  double dScaleRatio = qMin(scaledSize.width() / (double)_imageRefSize.width(), scaledSize.height() / (double)_imageRefSize.height());
  int w = (int)(dScaleRatio * _imageRefSize.width() + 0.5);
  int h = (int)(dScaleRatio * _imageRefSize.height() + 0.5);

  _pImageLabel->setFixedSize(w,h);
  _pMirrorLabel->setFixedSize(w,h);
}

void ContentWidget::showEvent(QShowEvent *e)
{
  QWidget::showEvent(e);
  updateInfo(getCurrentDocument());
}

void ContentWidget::demoButtonClicked()
{
  PushButton *pSenderButton = qobject_cast<PushButton*>(sender());
  if (pSenderButton)
    {
      _demoDir = _currentDir;
      _demoDir.cd(pSenderButton->dirname());
      initDirectory();
    }
}

void ContentWidget::quitButtonClicked()
{
  if (_currentDir == _demoRootDir)
    emit quit();
  else
    {
      _currentDir.cdUp();
      _demoDir = _currentDir;
      initDirectory();
    }
}

QStringList ContentWidget::removeExtraDirectories(const QDir& dir, const QStringList& directories)
{
  QStringList retDirectories;
  
  for (int i=0; i<directories.size(); i++)
    {
      QString fileName = QString("%1/%2/README").arg(dir.path()).arg(directories[i]);
      if (QFile::exists(fileName))
        retDirectories << directories[i];
    }

  return retDirectories;
}

bool ContentWidget::isThereSubDemoDirectories(const QDir& dir)
{
  //get subdirs
  QStringList lstDirectories = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);

  for (int i=0; i<lstDirectories.size(); i++)
    {
      QString fileName = QString("%1/%2/README").arg(dir.path()).arg(lstDirectories[i]);
      if (QFile::exists(fileName))
        return true;
    }
  

  return false;
}

QString ContentWidget::getTitle(const QString& path, const QString& dirname)
{
  QFile file(QString("%1/%2/README").arg(path).arg(dirname));
  if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
      QTextStream stream(&file);
      return stream.readLine();
    }

  return dirname;
}

ContentWidget::Document* ContentWidget::getCurrentDocument()
{
  QString key = _demoDir.path();
  
  //check if the document has already created
  for (int i=0; i<_lstDocuments.size(); i++)
    {
      if (_lstDocuments[i]->strKey == key)
        return _lstDocuments[i];
    }

  //cannot find existing document, create new
  Document *pDocument = createNewDocument(_demoDir);
  _lstDocuments << pDocument;

  return pDocument;
}

ContentWidget::Document* ContentWidget::createNewDocument(const QDir& directory)
{
  //create document
  Document *pDocument = new Document(directory.path());

  //store image
  QPixmap pixmap;
  QFileInfo info(QString("%1/image.png").arg(directory.path()));
  if (info.exists())
    pixmap = QPixmap(info.absoluteFilePath());

  pDocument->pixmap = pixmap.isNull() ? QPixmap(":/images/default.png") : pixmap;
  
  //store descriptions
  QFile file(QString("%1/README").arg(directory.path()));
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
      pDocument->strTitle = tr("No title");
      pDocument->strDescription = tr("Could not load description.");
    }
  else
    {
      QTextStream in(&file);

      //add title
      pDocument->strTitle = in.readLine();


      //add description
      QString strDescription;
      bool bLastEmpty = true;
      while (!in.atEnd())
        {
          QString line = in.readLine();

          if (!line.isEmpty())
            {
              if (!bLastEmpty)
                strDescription.append(' ');
              strDescription.append(line);
              bLastEmpty = false;
            }
          else if (!bLastEmpty)
            {
              strDescription.append("\n\n");
              bLastEmpty = true;
            }
        }

      pDocument->strDescription = strDescription;
    }

  return pDocument;
}


void ContentWidget::enterEvent(QEvent *e)
{
  setCursor(Qt::ArrowCursor);
  QWidget::enterEvent(e);
}
