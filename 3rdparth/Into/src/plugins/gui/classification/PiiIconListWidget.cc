/* This file is part of Into.
 * Copyright (C) 2003-2011 Intopii.
 * All rights reserved.
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

#include "PiiIconListWidget.h"
#include <QPushButton>
#include <QVBoxLayout>
#include <QSplitter>

PiiIconListWidget::PiiIconListWidget(QWidget *parent, QWidget *pEditorWidget) : QWidget(parent), _pEditorWidget(0)
{
  QVBoxLayout *pMainLayout = new QVBoxLayout;

  QPushButton *pCloseButton = new QPushButton(tr("Close"));
  connect(pCloseButton, SIGNAL(clicked()), this, SLOT(close()));

  QVBoxLayout *pCloseLayout = new QVBoxLayout;
  pCloseLayout->addStretch(1);
  pCloseLayout->addWidget(pCloseButton);

  QWidget *pBottomWidget = new QWidget;
  pBottomWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

  _pBottomLayout = new QHBoxLayout;
  _pBottomLayout->addStretch(1);
  _pBottomLayout->addLayout(pCloseLayout);
  _pBottomLayout->setContentsMargins(0,9,0,0);
  pBottomWidget->setLayout(_pBottomLayout);
  setEditorWidget(pEditorWidget);
  
  _pView = new QListWidget;
  _pView->setViewMode(QListView::IconMode);
  _pView->setSpacing(5);
  _pView->setIconSize(QSize(64,64));
  _pView->setWrapping (true);
  _pView->setFlow(QListView::LeftToRight);
  _pView->setResizeMode (QListView::Adjust);
  _pView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  _pView->setSelectionMode(QAbstractItemView::NoSelection);
  
  _pView->resize(400,400);

  QObject::connect(_pView, SIGNAL(itemActivated(QListWidgetItem *)),
                   this, SIGNAL(itemActivated(QListWidgetItem *)));


  QSplitter *pSplitter = new QSplitter(Qt::Vertical);
  pSplitter->addWidget(_pView);
  pSplitter->addWidget(pBottomWidget);
  pMainLayout->addWidget(pSplitter);

  pSplitter->setStretchFactor(0,1);
  pSplitter->setStretchFactor(1,0);

  setLayout(pMainLayout);
  resize(QSize(500,500));
}

void PiiIconListWidget::setImages(const QList<QImage*>& lstImages)
{
  _pView->clear();
  
  for (int j = 0; j < lstImages.size(); ++j)
    {
      QImage *im = lstImages.at(j);
      
      QListWidgetItem* item = new QListWidgetItem;
      item->setIcon(QIcon(QPixmap::fromImage(im->scaled(QSize(64,64),Qt::KeepAspectRatio))));
      item->setData(Qt::UserRole, j);

      QString strImageSize = im->textKeys().contains("pii_imagesize") ?
        im->text("pii_imagesize") :
        QString("%1x%2").arg(im->width()).arg(im->height());
      
      QString tooltip = QString("IMAGE %1/%2\n location: %3,%4 \n Size: %5")
        .arg(j+1)
        .arg(lstImages.size())
        .arg(im->offset().x())
        .arg(im->offset().y())
        .arg(strImageSize);
      item->setToolTip(tooltip);
      
      _pView->addItem(item);
    }
}

void PiiIconListWidget::setEditorWidget(QWidget *pEditorWidget)
{
  // If the new widget is same than the old, we do nothing.
  if (pEditorWidget == _pEditorWidget || pEditorWidget == 0)
    return;

  _pBottomLayout->insertWidget(0,pEditorWidget);
 
  if (_pEditorWidget != 0)
    {
      _pBottomLayout->removeWidget(_pEditorWidget);
      _pEditorWidget->hide();
      _pEditorWidget->deleteLater();
    }

  _pEditorWidget = pEditorWidget;
  _pEditorWidget->show();
}

void PiiIconListWidget::keyPressEvent(QKeyEvent *e)
{
  if ( e->key() == Qt::Key_Escape )
    close();

  QWidget::keyPressEvent(e);
}

    
  
