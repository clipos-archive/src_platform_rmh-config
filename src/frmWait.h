// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright © 2009-2018 ANSSI. All Rights Reserved.
/**
 * @author Olivier Levillain <clipos@ssi.gouv.fr>
 *
 * Copyright (C) 2009 SGDN/DCSSI
 * Copyright (C) 2011 SGDSN/ANSSI
 * @n
 * All rights reserved.
 */

#ifndef FRMWAIT_H
#define FRMWAIT_H

#include <qdialog.h>

class QProcess;

class frmWait : public QDialog
{
    Q_OBJECT

 public:
  frmWait(QWidget* parent, const QString& title, const  QString& msg, const bool& ptrFinished);
  frmWait(QWidget* parent, const QString& title, const  QString& msg, QProcess& p);

 protected:
  virtual void keyPressEvent (QKeyEvent* e);
  virtual void closeEvent (QCloseEvent* e);

 private:
  const bool* finished;
  QProcess* runningProcess;
  void init (const QString& title, const  QString& msg);
};

#endif // FRMWAIT_H
