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

#ifndef FRMADMININFO_H
#define FRMADMININFO_H

#include <QDialog>

class QLineEdit;
class QPushButton;
class QCheckBox;


class frmAdminInfo : public QDialog
{
  Q_OBJECT

public:
  frmAdminInfo (QString& managerDN,
	       QString& managerPass,
	       bool& saveInfos);

 private:
  QString& managerDN;
  QString& managerPass;
  bool& saveInfos;

  QLineEdit* edtManagerDN;
  QLineEdit* edtManagerPasswd;
  QCheckBox* chkSaveManagerDN;
  QPushButton* btnOK;
  QPushButton* btnCancel;

 private slots:
  void btnOKClicked();
};


#endif // FRMADMININFO_H
