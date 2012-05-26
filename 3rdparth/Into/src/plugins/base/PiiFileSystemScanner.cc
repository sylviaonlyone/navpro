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

#include "PiiFileSystemScanner.h"

#include <PiiYdinTypes.h>

PiiFileSystemScanner::Data::Data() :
  iMaxDepth(1),
  iSortFlags(Unsorted),
  iFilters(Readable | Files),
  iRepeatCount(-1),
  iLoopIndex(0)
{
}

PiiFileSystemScanner::PiiFileSystemScanner() :
  PiiDefaultOperation(new Data, Threaded)
{
  PII_D;
  addSocket(d->pTriggerInput = new PiiInputSocket("trigger"));
  addSocket(d->pPathInput = new PiiInputSocket("path"));
  d->pTriggerInput->setOptional(true);
  d->pPathInput->setOptional(true);
  
  addSocket(new PiiOutputSocket("filename"));
}

void PiiFileSystemScanner::check(bool reset)
{
  PiiDefaultOperation::check(reset);

  if (reset)
    {
      resetPaths();
      _d()->iLoopIndex = 0;
    }
 }

void PiiFileSystemScanner::resetPaths()
{
  PII_D;
  if (d->lstPaths.isEmpty())
    d->lstPathsToScan = PathList() << qMakePair(QString("."), d->iMaxDepth);
  else
    {
      d->lstPathsToScan.clear();
      for (int i=0; i<d->lstPaths.size(); ++i)
        d->lstPathsToScan << qMakePair(d->lstPaths[i], d->iMaxDepth);
    }
  d->lstCollectedFiles.clear();
  if (!d->pPathInput->isConnected() && !findAtLeastOneFile())
    PII_THROW(PiiExecutionException, tr("Cannot find any matching files with the given set of filters."));
}

void PiiFileSystemScanner::process()
{
  PII_D;
  if (d->pPathInput->isConnected())
    {
      startMany();
      scanFolder(PiiYdin::convertToQString(d->pPathInput), d->iMaxDepth);
      for (int i=0; i<d->lstCollectedFiles.size(); ++i)
        emitObject(d->lstCollectedFiles[i]);
      endMany();
    }
  else
    {
      emitObject(d->lstCollectedFiles.takeFirst());

      // If no more files exist...
      if (d->lstCollectedFiles.isEmpty() &&
          !findAtLeastOneFile())
        {
          // If we are done iterating, stop spontaneously.
          if (!d->pTriggerInput->isConnected() &&
              d->iRepeatCount > 0 &&
              ++d->iLoopIndex >= d->iRepeatCount)
            operationStopped();
          else
            resetPaths();
        }
    }
}

bool PiiFileSystemScanner::findAtLeastOneFile()
{
  PII_D;
  // Scan folders until at least one matching file is found or the
  // folder list becomes empty.
  while (d->lstCollectedFiles.isEmpty() && !d->lstPathsToScan.isEmpty())
    {
      QPair<QString,int> pathPair(d->lstPathsToScan.takeFirst());
      scanFolder(pathPair.first, pathPair.second, d->lstPathsToScan, d->lstCollectedFiles);
    }
  return !d->lstCollectedFiles.isEmpty();
}

void PiiFileSystemScanner::scanFolder(const QString& path, int maxDepth)
{
  PII_D;
  d->lstPathsToScan.clear();
  d->lstCollectedFiles.clear();
  d->lstPathsToScan << qMakePair(path, maxDepth);

  // Breadth first search
  while (!d->lstPathsToScan.isEmpty())
    {
      QPair<QString,int> pathPair(d->lstPathsToScan.takeFirst());
      scanFolder(pathPair.first, pathPair.second, d->lstPathsToScan, d->lstCollectedFiles);
    }
}

void PiiFileSystemScanner::scanFolder(const QString& path, int maxDepth,
                                      PathList& paths, QStringList& files)
{
  PII_D;
  if (maxDepth > 1)
    {
      // If we have subfolders to scan, include all folders in search
      QFileInfoList lstInfos(QDir(path).entryInfoList(d->lstNameFilters,
                                                      QDir::Filter((d->iFilters |
                                                                    QDir::AllDirs |
                                                                    QDir::NoDotAndDotDot) &
                                                                   ~QDir::Dirs),
                                                      QDir::SortFlag(d->iSortFlags)));
      for (int i=0; i<lstInfos.size(); ++i)
        {
          QString strFullPath = path + '/' + lstInfos[i].fileName();
          if (lstInfos[i].isDir())
            {
              // Directories need to be scanned
              paths << qMakePair(strFullPath, maxDepth-1);
              // If all directories are to be listed, add this one to
              // matched files as well.
              if (d->iFilters & QDir::AllDirs)
                files << strFullPath;
              // If only matching directories are to be returned.
              else if (d->iFilters & QDir::Dirs)
                {
                  QRegExp wildcard;
                  wildcard.setPatternSyntax(QRegExp::Wildcard);
                  for (int j=0; j<d->lstNameFilters.size(); ++j)
                    if (wildcard.exactMatch(lstInfos[i].fileName()))
                      {
                        files << strFullPath;
                        break;
                      }
                }
            }
          else // normal file
            files << strFullPath;
        }
    }
  else
    {
      QStringList lstFiles(QDir(path).entryList(d->lstNameFilters, QDir::Filter(d->iFilters), QDir::SortFlag(d->iSortFlags)));
      for (int i=0; i<lstFiles.size(); ++i)
        files << path + '/' + lstFiles[i];
    }
}

void PiiFileSystemScanner::setPaths(const QStringList& paths) { _d()->lstPaths = paths; }
QStringList PiiFileSystemScanner::paths() const { return _d()->lstPaths; }
void PiiFileSystemScanner::setMaxDepth(int maxDepth) { _d()->iMaxDepth = maxDepth; }
int PiiFileSystemScanner::maxDepth() const { return _d()->iMaxDepth; }
void PiiFileSystemScanner::setSortFlags(SortFlags sortFlags) { _d()->iSortFlags = sortFlags; }
PiiFileSystemScanner::SortFlags PiiFileSystemScanner::sortFlags() const { return (SortFlag)_d()->iSortFlags; }
void PiiFileSystemScanner::setFilters(Filters filters) { _d()->iFilters = filters; }
PiiFileSystemScanner::Filters PiiFileSystemScanner::filters() const { return (Filter)_d()->iFilters; }
void PiiFileSystemScanner::setNameFilters(const QStringList& nameFilters) { _d()->lstNameFilters = nameFilters; }
QStringList PiiFileSystemScanner::nameFilters() const { return _d()->lstNameFilters; }
void PiiFileSystemScanner::setRepeatCount(int repeatCount) { _d()->iRepeatCount = repeatCount; }
int PiiFileSystemScanner::repeatCount() const { return _d()->iRepeatCount; }
