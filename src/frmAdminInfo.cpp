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

#include "frmAdminInfo.h"

#include <QLayout>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QTabWidget>
#include <QDir>
#include <QFile>
#include <QCheckBox>
#include <QProcess>

#include <stdlib.h>
#include <unistd.h>



// frmAdminInfo constructor
//-------------------------

frmAdminInfo::frmAdminInfo (QString& managerDN,
			    QString& managerPass,
			    bool& saveInfos)
  : managerDN (managerDN), managerPass (managerPass), saveInfos (saveInfos)
{
  setWindowTitle ("Informations concernant le compte administrateur");

  edtManagerDN = new QLineEdit (managerDN, this);
  edtManagerPasswd = new QLineEdit ("", this);
  edtManagerPasswd->setEchoMode (QLineEdit::Password);
  chkSaveManagerDN = new QCheckBox ("Retenir le nom distinctif", this);
  chkSaveManagerDN->setChecked(saveInfos);
  btnOK = new QPushButton ("OK", this);
  connect (btnOK, SIGNAL (clicked ()), this, SLOT (btnOKClicked ()));
  btnCancel = new QPushButton ("Annuler", this);
  connect (btnCancel, SIGNAL (clicked ()), this, SLOT (reject ()));

  // Mise en page
  QGridLayout* lv_admin = new QGridLayout (this);
  lv_admin->addWidget (new QLabel ("Nom distinctif : ", this), 0, 0);
  lv_admin->addWidget (edtManagerDN, 0, 1);
  lv_admin->addWidget (new QLabel ("Mot de passe : ", this), 1, 0);
  lv_admin->addWidget (edtManagerPasswd, 1, 1);
  lv_admin->addWidget (chkSaveManagerDN, 2, 0);
  lv_admin->addWidget (btnOK, 3, 0);
  lv_admin->addWidget (btnCancel, 3, 1);
}


void frmAdminInfo::btnOKClicked () {
  managerDN = edtManagerDN->text();
  managerPass = edtManagerPasswd->text();
  saveInfos = chkSaveManagerDN->isChecked();

  accept();
}
