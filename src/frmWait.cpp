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

#include "frmWait.h"

#include <QLabel>
#include <QTimer>
#include <QLayout>
#include <QProcess>
#include <QKeyEvent>


void frmWait::init (const QString& title, const  QString& msg) {
  setWindowTitle (title);
  QLabel* lbl = new QLabel (msg, this);
  lbl->setSizePolicy (QSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed));

  QVBoxLayout* formLayout = new QVBoxLayout (this);

  QHBoxLayout* horzLayout = new QHBoxLayout ();
  horzLayout->addItem (new QSpacerItem (30, 0, QSizePolicy::Expanding, QSizePolicy::Expanding));
  horzLayout->addWidget (lbl);
  horzLayout->addItem (new QSpacerItem (30, 0, QSizePolicy::Expanding, QSizePolicy::Expanding));
  
  formLayout->addItem (new QSpacerItem (0, 30, QSizePolicy::Expanding, QSizePolicy::Expanding));
  formLayout->addLayout (horzLayout);
  formLayout->addItem (new QSpacerItem (0, 15, QSizePolicy::Expanding, QSizePolicy::Expanding));
  
  QTimer* tmrTest = new QTimer (this);
  connect (tmrTest, SIGNAL(timeout()), this, SLOT (close()));
  tmrTest->start (100);
}


frmWait::frmWait(QWidget* parent, const QString& title, const  QString& msg, const bool& ptrFinished)
  : QDialog (parent), finished (&ptrFinished), runningProcess (NULL)
{
  init (title, msg);
}

frmWait::frmWait(QWidget* parent, const QString& title, const  QString& msg, QProcess& p)
  : QDialog (parent), finished (NULL), runningProcess (&p)
{
  init (title, msg);
}


void frmWait::keyPressEvent (QKeyEvent* e) {
  if (e->key() == Qt::Key_Escape)
    e->accept();
}


void frmWait::closeEvent (QCloseEvent* e) {
  if (((finished != NULL)  && (*finished)) ||
      ((runningProcess != NULL) 
        && (runningProcess->state() != QProcess::Running)))
    e->accept();
  else
    e->ignore();
}
