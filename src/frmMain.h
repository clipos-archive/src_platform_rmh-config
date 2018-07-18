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

#ifndef FRMMAIN_H
#define FRMMAIN_H

#include <QDialog>
#include <QThread>

#include "ClipConfigFile.h"

class QLineEdit;
class QTabWidget;
class QPushButton;
class QFrame;
class QCheckBox;
class QButtonGroup;
class QListWidget;


#define P_DONT_TOUCH 0
#define P_CLEAN_AND_TRUST 1
#define P_CLEAN_AND_DONT_TRUST 2


class frmMain : public QDialog
{
  Q_OBJECT

public:
  frmMain (bool adminMode);

 private:
  bool adminMode;

  QTabWidget* tabs;
  ClipConfigFile ldaprc;
  ClipConfigFile configFile;

  QFrame* tabChangePassword;
  QLineEdit* login;
  QLineEdit* oldPassword;
  QLineEdit* newPassword[2];
  QPushButton* btnUpdatePass;

  QFrame* tabCertificate;
  QLineEdit* cacertURI;
  QLineEdit* cacertFile;
  QPushButton* btnResetCacertParams;
  QPushButton* btnDownloadCacert;

  QFrame* tabCertDBs;
  QCheckBox* CDBAutoClean;
  QButtonGroup* CDBCleaningPolicy;
  QListWidget* CDBTrustedCerts;
  QPushButton* CDBAddTrustedCert;
  QPushButton* CDBDelTrustedCert;
  QPushButton* CDBCleanAndSave;

  QFrame* tabLDAPAdvancedParams;
  QLineEdit* serverURI;
  QLineEdit* searchBase;
  QPushButton* btnDefaultPassParams;
  QPushButton* btnSavePassParams;

  /* Administrator mode only */
  QCheckBox* chkAdminMode;
  QString managerDN;
  QString managerPassword;
  bool saveInfos;
  /***************************/

  QCheckBox* chkAdvancedMode;
  QPushButton* btnQuit;

 private:
  bool checkTLSCert(char delim);
  void loadParams ();
  bool updatePossible ();
  bool cleanMozillaCerts ();

 private slots:
  void loginOrPassChanged ();
  void updatePassword ();

  void addToTrustedCertListBox (const QString& filename);
  void addTrustedCert ();
  void delTrustedCert ();
  void cleanAndSave ();

  void setDefaultCacertParams ();
  void downloadCacert ();

  void setDefaultPassParams ();
  void savePassParams ();

  void switchAdvancedMode (bool);
  void switchAdminMode (bool);
  void quit (); 
};



class ChangePasswordThread : public QThread
{
 public:
  ChangePasswordThread (const QString& serverURI, const QString& searchBase,
			const QString& login, const QString& oldPassword, const QString& newPassword,
			bool& ptrRes, QString& message, bool& ptrFinished, const QString& bindingDN = QString::null);
  virtual void run ();
 private:
  const QString serverURI;
  const QString searchBase;
  const QString& login;
  const QString& oldPassword;
  const QString& newPassword;
  bool& ptrResult;
  QString& ptrMessage;
  bool& ptrFinished;
  const QString& bindingDN;
};


#endif // FRMMAIN_H
