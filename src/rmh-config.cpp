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

#include <QApplication>
#include <QtSingleApplication>
#include <QTranslator>
#include <QTextCodec>
#include <stdlib.h>
#include <unistd.h>

#include "frmMain.h"


int main (int argc, char** argv) {
  bool admin = false;
  int c;
  while ((c = getopt (argc, argv, "a")) != -1) {
    switch (c) {
    case 'a':
      admin = true;
      break;
    }
  }

  QtSingleApplication app ("Configuration des services RM_H", argc, argv);
  if (app.sendMessage("Hi there") || app.isRunning())
    return 0;

  QTranslator qt(0);
  qt.load ("qt_fr.qm", PREFIX"/share/qt4/translations");
  app.installTranslator(&qt);

  QTextCodec *codec = QTextCodec::codecForName("utf8");
  QTextCodec::setCodecForCStrings(codec);

  frmMain* window = new frmMain (admin);
  app.setActivationWindow (window);
  QObject::connect(&app, SIGNAL(messageReceived(const QString &)), 
                                  &app, SLOT(activateWindow()));
  window->show();
  exit (app.exec());
}
