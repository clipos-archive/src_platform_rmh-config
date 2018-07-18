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

#include "frmMain.h"
#include "frmWait.h"
#include "ldap-action.h"
#include "frmAdminInfo.h"

#include <QLayout>
#include <QMessageBox>
#include <QPushButton>
#include <QButtonGroup>
#include <QRadioButton>
#include <QApplication>
#include <QLabel>
#include <QLineEdit>
#include <QTabWidget>
#include <QDir>
#include <QFile>
#include <QCheckBox>
#include <QProcess>
#include <QRegExp>
#include <QListWidget>
#include <QFileDialog>

#include <stdlib.h>
#include <unistd.h>


static const QString configFilename = QDir::homePath() + "/.rmh_config";
static const QString ldaprcFilename = QDir::homePath() + "/.ldaprc";
static const QString defaultCacertURI = "https://clip.ssi.gouv.fr/CACert.pem";
static const QString defaultCacertFilename = QDir::homePath() + "/CACert.pem";
static const QString defaultServerURI = "ldaps://clip.ssi.gouv.fr:20636";
static const QString defaultBaseSearch = "dc=clip,dc=sgdn,dc=gouv,dc=fr";




static QString getStdErr (QProcess& p, int maxline) {
  QString output;
  p.setReadChannel(QProcess::StandardError);
  QString line = p.readLine();
  while (maxline > 0 && (!line.isNull())) {
    output += line;
    output += '\n';
    maxline--;
    line = p.readLine ();
  }
  if (!line.isNull())
    output += "[...]\n";
  return output;
}

static QString getStdOut (QProcess& p, int maxline) {
  QString output;
  p.setReadChannel(QProcess::StandardOutput);
  QString line = p.readLine();
  while (maxline > 0 && (!line.isNull())) {
    output += line;
    output += '\n';
    maxline--;
    line = p.readLine ();
  }
  if (!line.isNull())
    output += "[...]\n";
  return output;
}






// frmMain constructor
//--------------------

frmMain::frmMain (bool adminMode) : adminMode (adminMode) {
  setWindowTitle ("Configuration des services RM_H");

  // Création de l'objet tabs
  //-------------------------
  tabs = new QTabWidget (this);

  // Onglet changement mot de passe
  //-------------------------------
  tabChangePassword = new QFrame ();

  login = new QLineEdit ("", tabChangePassword);
  oldPassword = new QLineEdit ("", tabChangePassword);
  oldPassword->setEchoMode (QLineEdit::Password);
  connect (login, SIGNAL (textChanged (const QString&)), this, SLOT (loginOrPassChanged ()));
  connect (oldPassword, SIGNAL (textChanged (const QString&)), this, SLOT (loginOrPassChanged ()));
  connect (login, SIGNAL (returnPressed ()), this, SLOT (updatePassword ()));
  connect (oldPassword, SIGNAL (returnPressed ()), this, SLOT (updatePassword ()));
  for (int i=0; i<2; i++) {
    newPassword[i] = new QLineEdit ("", tabChangePassword);
    newPassword[i]->setEchoMode (QLineEdit::Password);
    connect (newPassword[i], SIGNAL (textChanged (const QString&)), this, SLOT (loginOrPassChanged ()));
    connect (newPassword[i], SIGNAL (returnPressed ()), this, SLOT (updatePassword ()));
  }

  btnUpdatePass = new QPushButton ("Changer le mot de passe", tabChangePassword);
  connect (btnUpdatePass, SIGNAL (clicked ()), this, SLOT (updatePassword ()));
  btnUpdatePass->setEnabled (false);


  // Mise en page
  QGridLayout* lv_changePass2 = new QGridLayout (tabChangePassword);
  lv_changePass2->addWidget (new QLabel ("Nom d'utilisateur : ", tabChangePassword), 1, 0);
  lv_changePass2->addWidget (login, 1, 1);
  lv_changePass2->addWidget (new QLabel ("Ancien mot de passe : ", tabChangePassword), 2, 0);
  lv_changePass2->addWidget (oldPassword, 2, 1);
  lv_changePass2->addWidget (new QLabel ("Nouveau mot de passe : ", tabChangePassword), 3, 0);
  lv_changePass2->addWidget (newPassword[0], 3, 1);
  lv_changePass2->addWidget (new QLabel ("Nouveau mot de passe (vérification) : ", tabChangePassword), 4, 0);
  lv_changePass2->addWidget (newPassword[1], 4, 1);
  lv_changePass2->addWidget (btnUpdatePass, 5, 1);
    
  tabs->addTab (tabChangePassword, "Changer de mot de passe");


  // Onglet certificat
  //------------------
  tabCertificate = new QFrame ();
  cacertURI = new QLineEdit ("", tabCertificate);
  cacertFile = new QLineEdit ("", tabCertificate);

  btnResetCacertParams = new QPushButton ("Rétablir les chemins par défaut", tabCertificate);
  connect (btnResetCacertParams, SIGNAL (clicked ()), this, SLOT (setDefaultCacertParams ()));
  btnDownloadCacert = new QPushButton ("Télécharger le certificat", tabCertificate);
  connect (btnDownloadCacert, SIGNAL (clicked ()), this, SLOT (downloadCacert ()));

  // Mise en page
  QGridLayout* lv_cert = new QGridLayout (tabCertificate);
  lv_cert->addWidget (new QLabel ("Adresse du certificat : ", tabCertificate), 2, 0);
  lv_cert->addWidget (cacertURI, 2, 1);
  lv_cert->addWidget (new QLabel ("Chemin où enregistrer le fichier : ", tabCertificate), 3, 0);
  lv_cert->addWidget (cacertFile, 3, 1);
  lv_cert->addWidget (btnResetCacertParams, 4, 0);
  lv_cert->addWidget (btnDownloadCacert, 4, 1);



  // Onglet magasins
  //----------------
  tabCertDBs = new QFrame ();
  CDBAutoClean = new QCheckBox ("Nettoyer les certificats à chaque lancement d'une application Mozilla", tabCertDBs);
  
  CDBCleaningPolicy = new QButtonGroup ();
  QRadioButton* btnDontTouch = new QRadioButton ("Ne pas toucher au magasin existant", tabCertDBs);
  QRadioButton* btnCleanAndTrust = new QRadioButton ("Réinitialiser le magasin avec les certificats embarqués", tabCertDBs);
  QRadioButton* btnCleanAndDontTrust = new QRadioButton ("Réinitialiser le magasin sans faire confiance aux certificats embarqués", tabCertDBs);
  CDBCleaningPolicy->addButton (btnDontTouch, P_DONT_TOUCH);
  CDBCleaningPolicy->addButton (btnCleanAndTrust, P_CLEAN_AND_TRUST);
  CDBCleaningPolicy->addButton (btnCleanAndDontTrust, P_CLEAN_AND_DONT_TRUST);

  CDBTrustedCerts = new QListWidget (tabCertDBs);
  CDBTrustedCerts->setSelectionMode (QAbstractItemView::MultiSelection);
  CDBAddTrustedCert = new QPushButton ("+", tabCertDBs);
  connect (CDBAddTrustedCert, SIGNAL (clicked ()), this, SLOT (addTrustedCert ()));
  CDBDelTrustedCert = new QPushButton ("-", tabCertDBs);
  connect (CDBDelTrustedCert, SIGNAL (clicked ()), this, SLOT (delTrustedCert ()));

  CDBCleanAndSave = new QPushButton ("Nettoyer et sauver la configuration", tabCertDBs);  
  connect (CDBCleanAndSave, SIGNAL (clicked ()), this, SLOT (cleanAndSave ()));

  // Mise en page
  QVBoxLayout* lv_certdbs = new QVBoxLayout (tabCertDBs);
  lv_certdbs->addItem (new QSpacerItem (0, 10, QSizePolicy::Minimum, QSizePolicy::Fixed));
  lv_certdbs->addWidget (CDBAutoClean);
  lv_certdbs->addItem (new QSpacerItem (0, 10, QSizePolicy::Minimum, QSizePolicy::Fixed));

  lv_certdbs->addWidget (new QLabel ("Politique de nettoyage", tabCertDBs));
  lv_certdbs->addWidget (btnDontTouch);
  lv_certdbs->addWidget (btnCleanAndTrust);
  lv_certdbs->addWidget (btnCleanAndDontTrust);
  lv_certdbs->addItem (new QSpacerItem (0, 10, QSizePolicy::Minimum, QSizePolicy::Fixed));

  lv_certdbs->addWidget (new QLabel ("Certificats racines à ajouter : ", tabCertDBs));
  lv_certdbs->addWidget (CDBTrustedCerts);
  CDBTrustedCerts->setMinimumWidth (500);
  CDBTrustedCerts->setMinimumHeight (100);

  QHBoxLayout* layCDBButtons = new QHBoxLayout ();
  layCDBButtons->addItem (new QSpacerItem (2, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));
  layCDBButtons->addWidget (CDBAddTrustedCert);
  layCDBButtons->addItem (new QSpacerItem (10, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));
  layCDBButtons->addWidget (CDBDelTrustedCert);
  layCDBButtons->addItem (new QSpacerItem (2, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));

  lv_certdbs->addLayout (layCDBButtons);
  lv_certdbs->addItem (new QSpacerItem (0, 10, QSizePolicy::Minimum, QSizePolicy::Fixed));

  lv_certdbs->addWidget (CDBCleanAndSave);

  lv_certdbs->addItem (new QSpacerItem (0, 10, QSizePolicy::Minimum, QSizePolicy::Fixed));


  // Onglet paramètres avancés
  //--------------------------
  tabLDAPAdvancedParams = new QFrame ();
  serverURI = new QLineEdit ("", tabLDAPAdvancedParams);
  searchBase = new QLineEdit ("", tabLDAPAdvancedParams);

  btnDefaultPassParams = new QPushButton ("Paramètres par défaut", tabLDAPAdvancedParams);
  connect (btnDefaultPassParams, SIGNAL (clicked ()), this, SLOT (setDefaultPassParams ()));

  btnSavePassParams = new QPushButton ("Sauver les paramètres", tabLDAPAdvancedParams);
  connect (btnSavePassParams, SIGNAL (clicked ()), this, SLOT (savePassParams ()));

  // Mise en page
  QGridLayout* lv_advanced = new QGridLayout (tabLDAPAdvancedParams);
  lv_advanced->addWidget (new QLabel ("Adresse du serveur : ", tabLDAPAdvancedParams), 2, 0);
  lv_advanced->addWidget (serverURI, 2, 1);
  lv_advanced->addWidget (new QLabel ("Base de recherche : ", tabLDAPAdvancedParams), 3, 0);
  lv_advanced->addWidget (searchBase, 3, 1);
  lv_advanced->addWidget (btnDefaultPassParams, 4, 0);
  lv_advanced->addWidget (btnSavePassParams, 4, 1);



  // Boutons Mode Avancé / Quitter
  //------------------------------

  if (adminMode) {
    /* Administrator mode only */
    chkAdminMode = new QCheckBox ("Accès administrateur", this);
    chkAdminMode->setChecked(false);
    connect (chkAdminMode, SIGNAL(toggled(bool)), this, SLOT (switchAdminMode(bool)));
    switchAdminMode (false);
    /***************************/
  }

  chkAdvancedMode = new QCheckBox ("Paramètres avancés", this);
  connect (chkAdvancedMode, SIGNAL(toggled(bool)), this, SLOT (switchAdvancedMode (bool)));
  chkAdvancedMode->setChecked(false);

  btnQuit = new QPushButton ("Quitter", this);
  btnQuit->setDefault  (false);
  btnQuit->setAutoDefault  (false);
  connect (btnQuit, SIGNAL(clicked()), this, SLOT (quit()));

  QHBoxLayout* layButtons = new QHBoxLayout ();
  layButtons->addItem (new QSpacerItem (2, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));
  layButtons->addWidget (chkAdvancedMode);

  if (adminMode) {
    /* Administrator mode only */
    layButtons->addItem (new QSpacerItem (2, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));
    layButtons->addWidget (chkAdminMode);
    /***************************/
  }

  layButtons->addItem (new QSpacerItem (10, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));
  layButtons->addWidget (btnQuit);
  layButtons->addItem (new QSpacerItem (2, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));

  // Gestion du layout de la fenetre
  //--------------------------------
  QVBoxLayout* formLayout = new QVBoxLayout (this);
  formLayout->addWidget (tabs);
  formLayout->addLayout (layButtons);

  configFile.readFile (configFilename);
  if (! checkTLSCert ('\t') && ! checkTLSCert (' ')) {
    QMessageBox::information (NULL, "Information",
			      "<p>Bienvenue dans l'application de configuration des services RM_H.</p>"
			      "<p>Il semblerait que vous n'ayez pas encore configuré cette application. "
			      "Afin que les services fonctionnent correctement, il est très fortement "
			      "conseillé de commencer par télécharger le certificat des services RM_H. "
			      "Pour cela, rendez-vous dans l'onglet Certificat.</p>"
			      "<p>Ensuite, pensez à configurer la gestion des magasins de certificats Mozilla, "
			      "dans l'onglet Magasins Mozilla</p>");
    chkAdvancedMode->setChecked(true);
  }

  loadParams ();
}



// Gestion générique des fichiers de configuration

bool frmMain::checkTLSCert(char delim) {
  ldaprc.setDelimiter (delim);
  ldaprc.readFile (ldaprcFilename);
  QString path = ldaprc.getVal ("TLS_CACERT");
  return QFile::exists (path);
}


void frmMain::loadParams () {
  QString p;

  login->setText (configFile.getVal ("DEFAULT_LOGIN"));
  login->selectAll();

  p = configFile.getVal ("CACERT_URI");
  if (p.isEmpty())
    cacertURI->setText (defaultCacertURI);
  else
    cacertURI->setText (p);

  p = ldaprc.getVal ("TLS_CACERT");
  if (p.isEmpty())
    cacertFile->setText (defaultCacertFilename);
  else
    cacertFile->setText (p.trimmed ());


  p = configFile.getVal ("CERTDB_AUTOMATIC", QRegExp ("(yes|true)", Qt::CaseInsensitive));
  CDBAutoClean->setChecked (! p.isEmpty());

  if (! (configFile.getVal ("CERTDB_INIT", QRegExp ("(yes|true)", Qt::CaseInsensitive)).isEmpty())) {
    if (! (configFile.getVal ("CERTDB_TRUST_BUILTIN", QRegExp ("(yes|true)", Qt::CaseInsensitive)).isEmpty())) {
      CDBCleaningPolicy->button(P_CLEAN_AND_TRUST)->setChecked (true);
    } else {
      CDBCleaningPolicy->button(P_CLEAN_AND_DONT_TRUST)->setChecked (true);
    }
  } else
    CDBCleaningPolicy->button(P_DONT_TOUCH)->setChecked (true);

  QStringList certs = configFile.getVal ("CERTDB_TRUSTED_CERTS").split (" ");
  CDBTrustedCerts->addItems (certs);


  p = configFile.getVal ("SERVER_URI");
  if (p.isEmpty())
    serverURI->setText (defaultServerURI);
  else
    serverURI->setText (p);

  p = configFile.getVal ("SEARCH_BASE");
  if (p.isEmpty())
    searchBase->setText (defaultBaseSearch);
  else
    searchBase->setText (p);

  if (adminMode) {
    /* Administrator mode only */
    p = configFile.getVal ("SAVE_MANAGER_PARAMS", QRegExp ("(yes|true)", Qt::CaseInsensitive));
    saveInfos = ! p.isEmpty();

    managerDN = configFile.getVal ("MANAGER_DN");
    /***************************/
  }
}




// Changement de mot de passe

bool frmMain::updatePossible () {
  return (! serverURI->text().isEmpty()) &&
    (! searchBase->text().isEmpty()) &&
    (! login->text().isEmpty()) &&
    ((! oldPassword->isEnabled()) || (! oldPassword->text().isEmpty())) &&
    (! newPassword[0]->text().isEmpty()) &&
    (newPassword[0]->text() == newPassword[1]->text());
}

void frmMain::loginOrPassChanged () {
  btnUpdatePass->setEnabled (updatePossible());
}


// If bindingDN is null, the DN associated to login will be used
// If login is null, only a fake change is made, to check wether the admin binding works
ChangePasswordThread::ChangePasswordThread (const QString& serverURI, const QString& searchBase,
					    const QString& login, const QString& oldPassword, const QString& newPassword,
					    bool& ptrRes, QString& message, bool& ptrFinished, const QString& bindingDN)
  : serverURI (serverURI), searchBase (searchBase),
    login (login), oldPassword (oldPassword), newPassword (newPassword),
    ptrResult (ptrRes), ptrMessage (message), ptrFinished (ptrFinished),
    bindingDN (bindingDN) {}


void ChangePasswordThread::run () {
  LDAP* ld;
  int ldap_errno;
  char* dn = NULL;
  const char* bind_dn = NULL;
  int rc;
  ptrResult = false;
  bool justCheckBind = login.isEmpty();


  if (! justCheckBind) {
    // Recherche du nom distinctif du login dont on veut modifier le mot de passe
    ld = get_ldap_connection (serverURI.toAscii().constData());
    if (!ld) {
      ptrMessage = QString("Erreur de connexion au serveur %1\n" 
	"Vérifiez les paramètres avancés et réessayez.\n\n"
	"Détails de l'erreur : %2").arg(serverURI).arg(errString);
      goto main_exit;
    }
    
    rc = get_dn (&ld, searchBase.toAscii().constData(), login.toAscii().constData(), &dn);
    if (rc != 0) {
      ptrMessage = QString("Impossible de trouver l'utilisateur %1\n"
	"Vérifiez le nom entré, et éventuellement les paramètres avancés, puis réessayez.\n\n"
	"Détails de l'erreur : %2").arg(login).arg(errString);
      goto main_exit;
    }
  }
    
  if (bindingDN.isEmpty())
    bind_dn = dn;
  else
    bind_dn = bindingDN.toAscii().constData();

  // Connexion en tant que $bind_dn
  ld = get_ldap_connection (serverURI.toAscii().constData());
  if (!ld) {
    ptrMessage = QString("Erreur de connexion au serveur %1\n"
      "Vérifiez les paramètres avancés et réessayez.\n\n"
      "Détails de l'erreur : %2").arg(serverURI).arg(errString);
    goto main_free;
  }

  rc = bind_as (&ld, bind_dn, oldPassword.toAscii().constData());
  if (rc != 0) {
    ptrMessage = QString ("Mot de passe incorrect.\n\n" 
      "Détails de l'erreur : %1").arg(errString);
    goto main_free;
  }

  // We were just here to check wether the admin DN/Password worked, so we can leave.
  if (justCheckBind) {
    ptrResult = true;
    ptrMessage = "Vous êtes maintenant en mode administrateur.";
    goto main_unbind;
  }

  rc = change_password (&ld, dn, newPassword.toAscii().constData());
  if (rc != 0) {
    ptrMessage = QString ("Erreur du serveur LDAP lors du changement de mot de passe.\n\n"
      "Détails de l'erreur : %1").arg(errString);
    goto main_unbind;
  }
  
  ptrResult = true;
  ptrMessage = "Le mot de passe de l'utilisateur " + login +
    " a été correctement mis à jour";

 main_unbind:
  if ((ldap_errno = ldap_unbind_s (ld)) != 0) {
    ptrResult = false;
    ptrMessage = QString ("Erreur lors de la désassociation LDAP (unbind) : %1").arg(ldap_err2string (ldap_errno));
  }

 main_free:
  if (! justCheckBind)
    free (dn);

 main_exit:
  ptrFinished = true;
}



void frmMain::updatePassword () {
  if (!updatePossible())
    return;

  bool finished = false;
  bool ok;
  QString message;

  QString bindingLogin, oldPasswd;
  if (adminMode && chkAdminMode->isChecked()) {
    bindingLogin = managerDN;
    oldPasswd = managerPassword;
  } else {
    bindingLogin = QString::null;
    oldPasswd = oldPassword->text();
  }
    

  ChangePasswordThread t (serverURI->text(), searchBase->text(),login->text(),
			  oldPasswd, newPassword[0]->text(), ok, message,
			  finished, bindingLogin);

  frmWait dialog (this, "Opération en cours", "Veuillez patienter pendant la modification du mot de passe...", finished);

  t.start ();
  dialog.exec ();
   
  if (ok) {
    savePassParams ();
    oldPassword->setText ("");
    newPassword[0]->setText ("");
    newPassword[1]->setText ("");
    login->selectAll();
    login->setFocus();
    QMessageBox::information (NULL, "Opération réussie", message);
  } else {
    QMessageBox::warning (NULL, "Erreur", message);
  }
}






// Gestion du certificat racine

void frmMain::setDefaultCacertParams () {
  cacertURI->setText (defaultCacertURI);
  cacertFile->setText (defaultCacertFilename);
}


void frmMain::downloadCacert() {
  char tempFile[] = "/var/tmp/rmh-config-cacert-XXXXXX";
  QProcess p;
  QString cmdLine;

  if (mkstemp (tempFile) < 0) {
    QMessageBox::warning (NULL, "Erreur", "Impossible de créer un fichier temporaire.");
    return;
  }

  QString tempFilename = QString (tempFile);

  QStringList args;
  args << "-k" << "-o" << tempFilename << cacertURI->text();
  p.start("curl", args);
  if (!p.waitForStarted()) {
    QMessageBox::warning (NULL, "Erreur", "Impossible de télécharger " + cacertURI->text() + ".");
    return;
  }
  frmWait dialog (this, "Opération en cours", "Veuillez patienter pendant le téléchargement du certificat...", p);
  dialog.exec ();

  if (p.exitStatus() != QProcess::NormalExit || p.exitCode() != 0) {
    QMessageBox::warning (NULL, "Erreur",
			  "Impossible de télécharger " + cacertURI->text() + ".\n\n" +
			  "Ligne de commande :\n" + cmdLine +
			  "\nDétails de la sortie du programme :\n" + getStdErr (p, 5));
    return;
  }
  
  args.clear();
  args << "x509" << "-fingerprint" << "-noout" << "-in" << tempFilename;
  p.start("openssl", args);
  if (!p.waitForStarted()) {
    QMessageBox::warning (NULL, "Erreur", "Impossible de calculer l'empreinte du certificat téléchargé.");
    QFile::remove (tempFilename);
    return;
  }
  p.waitForFinished();
  if (p.exitStatus() != QProcess::NormalExit || p.exitCode() != 0) {
    QMessageBox::warning (NULL, "Erreur",
			  "Erreur lors du calcul de l'empreinte du certificat téléchargé.\n\n"
			  "Ligne de commande :\n" + cmdLine +
			  "\nDétails de la sortie du programme :\n" + getStdErr (p, 5));
    QFile::remove (tempFilename);
    return;
  }
  
  p.setReadChannel(QProcess::StandardOutput);
  QString fingerprint = p.readLine();
  int pos = fingerprint.indexOf ('=');
  if (pos > 0)
    fingerprint = fingerprint.mid (pos+1);
  
  if (QMessageBox::question (0, "Confirmation", 
			     "Le certificat racine téléchargé a pour empreinte SHA-1\n\n" +
			     fingerprint + "\n\n"
			     "Est-ce l'empreinte qui vous a été communiquée "
			     "par les administrateurs des services RMH ?",
			     QMessageBox::Yes | QMessageBox::Default,
			     QMessageBox::No | QMessageBox::Escape) == QMessageBox::No) {
    QFile::remove (tempFilename);
    QMessageBox::warning (NULL, "Erreur", "Le certificat téléchargé n'a pas été validé.");
    return;
  }

  args.clear();
  args << "-f" << tempFile << cacertFile->text();
  p.start("mv", args);
  if (!p.waitForStarted()) {
    QMessageBox::warning (NULL, "Erreur", "Erreur lors de l'enregistrement de " + cacertFile->text() + ".");
    return;
  }
  p.waitForFinished();
  if (p.exitStatus() != QProcess::NormalExit || p.exitCode() != 0) {
    QMessageBox::warning (NULL, "Erreur",
			  "Erreur lors de l'enregistrement de " + cacertFile->text() + ".\n\n"
			  "Ligne de commande :\n" + cmdLine +
			  "\nDétails de la sortie du programme :\n" + getStdErr (p, 5));
    return;
  }
  
  configFile.setVal ("CACERT_URI", cacertURI->text());
  configFile.writeFile (); 
  ldaprc.setVal ("TLS_CACERT", cacertFile->text());
  ldaprc.writeFile (); 
  addToTrustedCertListBox (cacertFile->text());

  if (! checkTLSCert ('\t') && ! checkTLSCert (' ')) {
    QMessageBox::warning (NULL, "Erreur",
			  "Le certificat ne semble pas avoir été correctement enregistré\n"
			  "pour une raison inconnue. Veuillez contacter votre administrateur.");
    return;
  }
  
  cacertURI->selectAll();
  cacertURI->setFocus();
  QMessageBox::information (NULL, "Opération réussie",
			    "Le nouveau certificat a bien été téléchargé à l'emplacement souhaité.\n"
			    "La configuration a été mise à jour en conséquence.");
}





// Gestion des magasins de certificats mozilla

bool frmMain::cleanMozillaCerts() {
  QProcess p;
  QString cmdLine;
  QString s;
  bool init, trustLibNSS;
  QString certs;
  QStringList args;

  init = ! (configFile.getVal ("CERTDB_INIT", QRegExp ("(yes|true)", Qt::CaseInsensitive)).isEmpty());
  if (init)
    trustLibNSS = ! (configFile.getVal ("CERTDB_TRUST_BUILTIN", QRegExp ("(yes|true)", Qt::CaseInsensitive)).isEmpty());
  else
    trustLibNSS = true;

  certs = configFile.getVal ("CERTDB_TRUSTED_CERTS");

  QString cmd("init-mozilla-certs.sh");
  args.clear();
  args << "-H" << QDir::homePath();
  if (init)
    args << "-I";
  else
    args << "-i";
  if (trustLibNSS) 
    args << "-T";
  else
    args << "-t";
  args << "-A" << certs;

  p.start(cmd, args);
  if (!p.waitForStarted()) {
    QMessageBox::warning (NULL, "Erreur", "Impossible de nettoyer les magasins de certificats.");
    return false;
  }

  frmWait dialog (this, "Opération en cours", "Veuillez patienter pendant le nettoyage des magasins de certificats...", p);
  dialog.exec ();
  
  if (p.exitStatus() != QProcess::NormalExit || p.exitCode() != 0) {
    QMessageBox::warning (NULL, "Erreur",
			  QString ("Impossible de nettoyer les magasins de certificats.\n\n") +
			  "Ligne de commande :\n" + cmdLine +
			  "\nDétails de la sortie du programme :\n" + getStdOut (p, 5));
    return false;
  }
  
  return true;
}


void frmMain::addToTrustedCertListBox (const QString& filename) {
  QListWidgetItem* item;
  for (int i = 0; true; i++) {
    item = CDBTrustedCerts->item (i);
    if (item == NULL) {
      CDBTrustedCerts->addItem (filename);
      return;
    }
    if (item->text() == filename) return;
  }
}

void frmMain::addTrustedCert () {
  QStringList list = QFileDialog::getOpenFileNames (this, "Ajouter des certificats racine");

  for (QStringList::ConstIterator cert=list.constBegin(); cert != list.constEnd(); cert++)
    addToTrustedCertListBox (*cert);
}

void frmMain::delTrustedCert () {
  int i = 0;
  QListWidgetItem* it;

  while ((it = CDBTrustedCerts->item (i)) != 0) {
    if (it->isSelected ())
      delete it;
    else
      i++;
  }
}


void frmMain::cleanAndSave () {
  bool init = ! (CDBCleaningPolicy->checkedId() == P_DONT_TOUCH);
  bool trust = ! (CDBCleaningPolicy->checkedId() == P_CLEAN_AND_DONT_TRUST);
  QStringList list;
  for (int i=0; i<CDBTrustedCerts->count(); i++)
    list.append (CDBTrustedCerts->item(i)->text());
  
  configFile.setVal ("CERTDB_AUTOMATIC", CDBAutoClean->isChecked() ? "yes" : "no");
  configFile.setVal ("CERTDB_INIT", init ? "yes" : "no");
  configFile.setVal ("CERTDB_TRUST_BUILTIN", trust ? "yes" : "no");
  configFile.setVal ("CERTDB_TRUSTED_CERTS", list.join (" "));
  configFile.writeFile (); 

  cleanMozillaCerts ();
}





// Paramètres avancés de l'annuaire

void frmMain::setDefaultPassParams () {
  serverURI->setText (defaultServerURI);
  searchBase->setText (defaultBaseSearch);
}

void frmMain::savePassParams () {
  configFile.setVal ("SERVER_URI", serverURI->text());
  configFile.setVal ("SEARCH_BASE", searchBase->text());
  configFile.setVal ("DEFAULT_LOGIN", login->text());
  configFile.writeFile (); 
}




// Opération d'administration de l'annuaire

void frmMain::switchAdvancedMode (bool activate) {
  if (activate) {
    tabs->addTab (tabCertificate, "Certificat");
    tabs->addTab (tabCertDBs, "Magasins de certificats Mozilla");
    tabs->addTab (tabLDAPAdvancedParams, "Paramètres avancés de l'annuaire");
  } else {
    tabs->removeTab (tabs->indexOf (tabCertificate));
    tabs->removeTab (tabs->indexOf (tabCertDBs));
    tabs->removeTab (tabs->indexOf (tabLDAPAdvancedParams));
  }
}

void frmMain::switchAdminMode (bool activate) {
  bool ok;

  // Tentons de passer en mode administrateur
  if (activate) {
    frmAdminInfo d (managerDN, managerPassword, saveInfos);

    ok = d.exec () == Accepted;

    if (ok) {
      // Connexion en tant que $managerDN pour vérifier
      bool finished = false;
      QString message;
      
      ChangePasswordThread t (serverURI->text(), searchBase->text(),
			      QString::null, managerPassword, QString::null,
			      ok, message, finished, managerDN);
      frmWait dialog (this, "Opération en cours", "Veuillez patienter pendant que les données entrées sont vérifiées", finished);

      t.start ();
      dialog.exec ();

      if (ok) {
	if (saveInfos) {
	  configFile.setVal ("SAVE_MANAGER_PARAMS", "true");
	  configFile.setVal ("MANAGER_DN", managerDN);
	} else {
	  configFile.setVal ("SAVE_MANAGER_PARAMS", "false");
	  configFile.setVal ("MANAGER_DN", "");
	}

	QMessageBox::information (NULL, "Opération réussie", message);
      } else {
	QMessageBox::warning (NULL, "Erreur", message);
      }
    }

    // Si l'utilisateur a pressé le bouton Annuler, ou que la vérification a échoué, on décoche la case
    if (!ok)
      chkAdminMode->setChecked (false);

  } else {
    // Sinon, on oublie le mot de passe
    // TODO ! Quelque chose pour effacer la chaîne de caractère ici ???
    managerPassword = QString::null;
  }

  // Si on est en mode administrateur, l'ancien mot de passe n'est pas nécessaire
  oldPassword->setEnabled (! chkAdminMode->isChecked());
}

void frmMain::quit() {
  QApplication::exit (0);
}


