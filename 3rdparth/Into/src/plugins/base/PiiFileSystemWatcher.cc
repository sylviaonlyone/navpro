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

#include "PiiFileSystemWatcher.h"
#include <PiiYdinTypes.h>
#include <QtDebug>
#include <QTimer>
#include <QDirIterator>


PiiFileSystemWatcher::Data::Data() :
  iWatchDelay(0)
{
}

PiiFileSystemWatcher::PiiFileSystemWatcher() :
  PiiDefaultOperation(new Data, NonThreaded)
{
  PII_D;
  addSocket(new PiiOutputSocket("filename"));

  //connect signal for notifying changes
  connect(&d->fileSystemWatcher, SIGNAL(directoryChanged(const QString&)), this, SLOT(directoryChanged(const QString&)));
}

void PiiFileSystemWatcher::check(bool reset)
{
  PII_D;
  PiiDefaultOperation::check(reset);

  if (d->lstDirectories.size() == 0)
    PII_THROW(PiiExecutionException, tr("Cannot start without watched directories."));

  // This unnecessary check suppresses a bogus warning from Qt.
  if (!d->fileSystemWatcher.directories().isEmpty())
    // remove old paths
    d->fileSystemWatcher.removePaths(d->fileSystemWatcher.directories());

  //set new paths for watching
  d->fileSystemWatcher.addPaths(d->lstDirectories);

  // Set previous check time for all directories
  QDateTime currentTime = QDateTime::currentDateTime();
  d->lstPreviousCheckTimes.clear();
  d->lstLastModifiedFiles.clear();
  for (int i=0; i<d->lstDirectories.size(); ++i)
    {
      d->lstPreviousCheckTimes << currentTime;
      d->lstLastModifiedFiles << QStringList();
    }
  
  d->lstModifiedFiles.clear();
}

int PiiFileSystemWatcher::indexOf(const QStringList& paths, const QString& path)
{
  // Must be case-insensitive on Windows
#ifdef Q_OS_WIN
  QString strPath = path.toLower();
  for (int i=0; i<paths.size(); ++i)
    if (paths[i].toLower() == strPath)
      return i;
  return -1;
#else
  return paths.indexOf(path);
#endif
}

void PiiFileSystemWatcher::directoryChanged(const QString& path)
{
  PII_D;
  //qDebug("directoryChanged()");
  // Find the index of the path in our path list.
  int iPathIndex = indexOf(d->lstDirectories, path);

  // Not on our watch list.
  if (iPathIndex == -1)
    return;
  
  // Take the last check time of the directory.
  QDateTime previousCheckTime = d->lstPreviousCheckTimes[iPathIndex];

  //qDebug() << "path index" << iPathIndex << "previous check time:" << previousCheckTime;
  
  // Iterate over all files in the directory
  QDirIterator iterator(path, d->lstNameFilters, QDir::Files);
  // Collects the names of newest files.
  QStringList lstNewestFiles;
  while (iterator.hasNext())
    {
      iterator.next();
      QFileInfo info = iterator.fileInfo();
      QDateTime lastModified = info.lastModified();
      //qDebug() << info.absoluteFilePath() << lastModified;
      /* NOTE
       *
       * It may happen that many files are added within the resolution
       * of the file system timestamp. Therefore, we must risk
       * capturing the same change twice first. An additional check is
       * needed to see if the file was already sent. Another solution
       * would be to delay the emission of changed file names more
       * than the file system timestamp resolution, but this would
       * impose an unnecessary delay.
       */
      // See if file has been modified after (or during) the previous
      // check.
      if (lastModified >= previousCheckTime)
        {
          QString strFullFileName = info.absoluteFilePath();
          QString strFileName = iterator.fileName();
          int iFileIndex = d->lstModifiedFiles.indexOf(strFullFileName);
          //qDebug() << "accepted, iFileIndex = " << iFileIndex;
          // File is not currently in the modified file list -> add
          if (iFileIndex == -1)
            {
              // If the file is newer than the last check time, it
              // needs to be sent.
              if (lastModified > previousCheckTime)
                d->lstModifiedFiles << strFullFileName;
              // If it was changed exactly at the time of the last
              // check, it may have been missed...
              else if (lastModified == previousCheckTime &&
                       !d->lstLastModifiedFiles[iPathIndex].contains(strFileName))
                d->lstModifiedFiles << strFullFileName;
            }
          // If this file is the newest so far, update previous check
          // time and clear the list of newest files.
          if (lastModified > d->lstPreviousCheckTimes[iPathIndex])
            {
              d->lstPreviousCheckTimes[iPathIndex] = lastModified;
              lstNewestFiles.clear();
              lstNewestFiles << strFileName;
            }
          // Add to the list of newest files.
          else if (lastModified == d->lstPreviousCheckTimes[iPathIndex])
            lstNewestFiles << strFileName;
        }
    }

  //qDebug() << "newest files:" << lstNewestFiles;
  
  // Store the names of files that were modified at the same time
  // instant.
  d->lstLastModifiedFiles[iPathIndex] = lstNewestFiles;

  // If watch delay is zero, we need to send modified files immediately.
  if (d->iWatchDelay == 0)
    emitAllFileNames();
  // Otherwise, set up a timer that checks file modification times
  // after a while.
  else
    QTimer::singleShot(d->iWatchDelay*1000, this, SLOT(emitNotModifiedFileNames()));
}

void PiiFileSystemWatcher::emitAllFileNames()
{
  PII_D;
  try
    {
      for (int i=0; i<d->lstModifiedFiles.size(); ++i)
        {
          //qDebug("Sending %s", qPrintable(d->lstModifiedFiles[i]));
          emitObject(d->lstModifiedFiles[i]);
        }
    }
  catch (PiiExecutionException&)
    {
      // nop
    }
  d->lstModifiedFiles.clear();
}

void PiiFileSystemWatcher::emitNotModifiedFileNames()
{
  PII_D;
  // It may happen that the timer signal gets delivered too late.
  if (state() != Running)
    return;
  
  QDateTime lastAcceptedTime = QDateTime::currentDateTime().addSecs(-d->iWatchDelay);
  try
    {
      for (int i=d->lstModifiedFiles.size(); i--; )
        {
          // If the file was last modified more than d->iWatchDelay secs
          // ago, send and remove from list.
          if (QFileInfo(d->lstModifiedFiles[i]).lastModified() <= lastAcceptedTime)
            {
              //qDebug("Sending %s", qPrintable(d->lstModifiedFiles[i]));
              emitObject(d->lstModifiedFiles[i]);
              d->lstModifiedFiles.removeAt(i);
              d->lstModifiedTimes.removeAt(i);
            }
        }
    }
  catch (PiiExecutionException&)
    {
      // nop
    }
}

void PiiFileSystemWatcher::process() {}

void PiiFileSystemWatcher::setDirectories(const QStringList& directories) { _d()->lstDirectories = directories; }
QStringList PiiFileSystemWatcher::directories() const { return _d()->lstDirectories; }
void PiiFileSystemWatcher::setNameFilters(const QStringList& nameFilters) { _d()->lstNameFilters = nameFilters; }
QStringList PiiFileSystemWatcher::nameFilters() const { return _d()->lstNameFilters; }
void PiiFileSystemWatcher::setWatchDelay(int watchDelay) { _d()->iWatchDelay = watchDelay; }
int PiiFileSystemWatcher::watchDelay() const { return _d()->iWatchDelay; }
