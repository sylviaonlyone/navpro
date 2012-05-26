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

#ifndef _PIIFILESYSTEMSCANNER_H
#define _PIIFILESYSTEMSCANNER_H

#include <PiiDefaultOperation.h>
#include <QDir>

/**
 * Scans directory hierarchies finding files that match filters. This
 * operation is somewhat similar to the Unix "find" command or "dir
 * /s" in Windows.
 *
 * @inputs
 *
 * @in trigger - an optional input that can be used to trigger one
 * file name. The incoming object is ignored as it only serves as a
 * trigger pulse.
 *
 * @in path - an optional input that can be used to put new paths to
 * the scanner. If this input is connected, the #paths property will
 * be ignored. Whenever a new path is received, the given path is
 * scanned recursively, and every matching file will be emitted
 * through the @p filename output. The flow level of the output will
 * be one higher than that of the input.
 *
 * @outputs
 *
 * @out filename - a full path name of a matched file.
 *
 * @ingroup PiiBasePlugin
 */
class PiiFileSystemScanner : public PiiDefaultOperation
{
  Q_OBJECT

  /**
   * A list of paths to be scanned. If this list is empty, the current
   * working directory of the executable will be scanned.
   */
  Q_PROPERTY(QStringList paths READ paths WRITE setPaths);

  /**
   * The maximum depth of recursion when scanning directory
   * hierarchies. The default is 1. Note that in recursive scanning,
   * all intermediate directories will be scanned even if they don't
   * match the name filters.
   */
  Q_PROPERTY(int maxDepth READ maxDepth WRITE setMaxDepth);

  /**
   * A combination of SortFlag flags that control sorting. The default
   * is @p Unsorted.
   */
  Q_PROPERTY(SortFlags sortFlags READ sortFlags WRITE setSortFlags);

  /**
   * A combination of Filter flags that control the types of files to
   * be matched. The default is @p Readable | @p Files.
   */
  Q_PROPERTY(Filters filters READ filters WRITE setFilters);

  /**
   * A list of wildcard patterns used to match files by name. If this
   * list is empty, everything will be matched.
   *
   * @code
   * operation->setProperty("nameFilters", QStringList() << "*.avi" << "*.mpeg");
   * @endcode
   */
  Q_PROPERTY(QStringList nameFilters READ nameFilters WRITE setNameFilters);

  /**
   * The number of times the file names will be repeated. The default
   * value is 1. -1 means eternally. If this value is larger than one,
   * the scanner will start this many times from the beginning once it
   * has found all matching files.
   */
  Q_PROPERTY(int repeatCount READ repeatCount WRITE setRepeatCount);

  Q_FLAGS(SortFlags);
  Q_FLAGS(Filters);

  PII_OPERATION_SERIALIZATION_FUNCTION
public:
  /**
   * Sort options available to %PiiFileSystemScanner. See
   * QDir::SortFlag.
   */
  enum SortFlag
  {
    Name = QDir::Name,
    Time = QDir::Time,
    Size = QDir::Size,
    Type = QDir::Type,
    Unsorted = QDir::Unsorted,
    NoSort = QDir::NoSort,
    DirsFirst = QDir::DirsFirst,
    DirsLast = QDir::DirsLast,
    Reversed = QDir::Reversed,
    IgnoreCase = QDir::IgnoreCase,
    LocaleAware = QDir::LocaleAware
  };
  Q_DECLARE_FLAGS(SortFlags, SortFlag);

  /**
   * Filtering options available to %PiiFileSystemScanner. See
   * QDir::Filter.
   */
  enum Filter
    {
      Dirs = QDir::Dirs,
      AllDirs = QDir::AllDirs,
      Files = QDir::Files,
      Drives = QDir::Drives,
      NoSymLinks = QDir::NoSymLinks,
      NoDotAndDotDot = QDir::NoDotAndDotDot,
      NoDot = QDir::NoDot,
      NoDotDot = QDir::NoDotDot,
      AllEntries = QDir::AllEntries,
      Readable = QDir::Readable,
      Writable = QDir::Writable,
      Executable = QDir::Executable,
      Modified = QDir::Modified,
      Hidden = QDir::Hidden,
      System = QDir::System,
      CaseSensitive = QDir::CaseSensitive
    };
  Q_DECLARE_FLAGS(Filters, Filter);
  
  PiiFileSystemScanner();

  void check(bool reset);
  
protected:
  void process();

  void setPaths(const QStringList& paths);
  QStringList paths() const;
  void setMaxDepth(int maxDepth);
  int maxDepth() const;
  void setSortFlags(SortFlags sortFlags);
  SortFlags sortFlags() const;
  void setFilters(Filters filters);
  Filters filters() const;
  void setNameFilters(const QStringList& nameFilters);
  QStringList nameFilters() const;
  void setRepeatCount(int repeatCount);
  int repeatCount() const;

private:
  typedef QList<QPair<QString,int> > PathList;
  /// @internal
  class Data : public PiiDefaultOperation::Data
  {
  public:
    Data();

    PiiInputSocket* pTriggerInput, *pPathInput;
    QStringList lstPaths;
    int iMaxDepth;
    int iSortFlags;
    int iFilters;
    QStringList lstNameFilters;
    int iRepeatCount;
    PathList lstPathsToScan;
    QStringList lstCollectedFiles;
    int iLoopIndex;
  };
  PII_D_FUNC;

  void resetPaths();
  bool findAtLeastOneFile();
  void scanFolder(const QString& path, int maxDepth);
  void scanFolder(const QString& path, int maxDepth, PathList& paths, QStringList& files);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(PiiFileSystemScanner::SortFlags);
Q_DECLARE_OPERATORS_FOR_FLAGS(PiiFileSystemScanner::Filters);

#endif //_PIIFILESYSTEMSCANNER_H
