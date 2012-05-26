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

#ifndef _VISIBILITYCHECKER_H
#define _VISIBILITYCHECKER_H

#include <QScriptEngineDebugger>
#include <QApplication>
#include <QMainWindow>

class VisibilityChecker : public QObject
{
  Q_OBJECT
public:
  VisibilityChecker(QScriptEngineDebugger* dbg) : _pDbg(dbg) {}

public slots:
  void check() { if (!_pDbg->standardWindow()->isVisible()) QApplication::exit(0); }

private:
  QScriptEngineDebugger* _pDbg;
};

#endif //_VISIBILITYCHECKER_H
