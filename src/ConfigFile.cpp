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

#include "ConfigFile.h"
#include <QFile>
#include <QTextStream>


void ConfigFile::readFile (const QString& fn) {
  filename = fn;
  QFile f (filename);
  resetContent ();

  if (f.open (QIODevice::ReadOnly)) {
    QTextStream s (&f);
    QString line;

    while (! (line = s.readLine ()).isNull())
      readLine (line);
  }
}



QString ConfigFile::getVal (const QString& vname, const QRegExp& filter) const {
  return QString::null;
}

QString ConfigFile::getVal (const QString& vname) const {
  return QString::null;
}

void ConfigFile::setVal (const QString& vname, const QString& val) {
  return;
}

QString ConfigFile::getNS (int ni) const {
  return QString::null;
}

void ConfigFile::setNS (int n, const QString& val) {
  return;
}



void ConfigFile::writeFile () {
  if (filename.isEmpty ())
    return;
  QFile f (filename);
  const QStringList content = contentToWrite ();

  if (f.open (QIODevice::WriteOnly | QIODevice::Truncate)) {
    QTextStream s (&f);
    QStringList::const_iterator i;

    for (i=content.constBegin(); i!=content.constEnd(); ++i)
      s << *i << '\n';
  } else {
    return;
  }
}


void ConfigFile::writeFileAs (const QString& fn) {
  filename = fn;
  writeFile ();
}
