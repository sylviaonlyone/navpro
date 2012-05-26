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

#ifndef _PIIFILESYSTEMWATCHER_H
#define _PIIFILESYSTEMWATCHER_H

#include <PiiDefaultOperation.h>

#include <QFileSystemWatcher>
#include <QDateTime>

/**
 * An operation for monitoring directories for new files. If the file
 * is added or modified in directory that are being watched, its
 * absolute file name will be emitted.
 *
 * @outputs
 *
 * @out filename - the absolute path of a file that was modified or
 * added to a watched directory.
 *
 * @ingroup PiiBasePlugin
 */
class PiiFileSystemWatcher : public PiiDefaultOperation
{
  Q_OBJECT

  /**
   * The list of watched directories.
   */
  Q_PROPERTY(QStringList directories READ directories WRITE setDirectories);

  /**
   * Sets the name filters used in watching new files. Each name
   * filter is a wildcard (globbing) filter that understands * and ? 
   * wildcards. The operation will only report files whose names match
   * the one of the filters. If the filter list is empty, all changed
   * files will be reported. An example:
   *
   * @code
   * QStringList lstNameFilters = QStringList() << "*.jpg" << "*.png";
   * pFileSystemWatcher->setProperty("nameFilters", lstNameFilters);
   * @endcode
   */
  Q_PROPERTY(QStringList nameFilters READ nameFilters WRITE setNameFilters);

  /**
   * Delay the emission of a file name for this many seconds. If @p
   * watchDelay is non-zero, added and modified files will not be
   * reported until they remain unchanged at least @p watchDelay
   * seconds. This can be used delay the handling of a large file
   * until it is completely stored into the file system. The default
   * value is zero.
   */
  Q_PROPERTY(int watchDelay READ watchDelay WRITE setWatchDelay);
  
  PII_OPERATION_SERIALIZATION_FUNCTION
public:
  PiiFileSystemWatcher();

  void setDirectories(const QStringList& directories);
  QStringList directories() const;
  void setNameFilters(const QStringList& nameFilters);
  QStringList nameFilters() const;
  void setWatchDelay(int watchDelay);
  int watchDelay() const;

  void check(bool reset);

protected:
  void process();

private slots:
  /**
   * This slot is called when the directory at a specified path is
   * modified.
   */
  void directoryChanged(const QString& path);
  void emitNotModifiedFileNames();
  
private:
  void emitAllFileNames();
  int indexOf(const QStringList& paths, const QString& path);

  /// @internal
  class Data : public PiiDefaultOperation::Data
  {
  public:
    Data();
    // List of watched dirs and their last check times
    QStringList lstDirectories;
    QList<QDateTime> lstPreviousCheckTimes;
    
    QFileSystemWatcher fileSystemWatcher;
    QStringList lstNameFilters;
    int iWatchDelay;
    
    // List of modified files pending emission
    QStringList lstModifiedFiles;
    // Lists of modified files found during the last check, per folder.
    QList<QStringList> lstLastModifiedFiles;
    QList<QDateTime> lstModifiedTimes;
  };
  PII_D_FUNC;
};


#endif //_PIIFILESYSTEMWATCHER_H
