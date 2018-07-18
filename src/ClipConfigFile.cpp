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

#include "ClipConfigFile.h"

#include <QRegExp>

static const QRegExp varname_rx ("\\w+");


ClipConfigFileLine::ClipConfigFileLine () {
  delimiter = '=';
  isModified = isNew = false;
  rawline = originalValue = QString();
}


ClipConfigFileLine::ClipConfigFileLine (const QString& vname, const QString& val, char sep) {
  varname = vname;
  value = val;
  delimiter = sep;
  rawline = varname + delimiter + value;
  isNew = isModified = true;
}


ClipConfigFileLine::ClipConfigFileLine (const QString& line, char sep) {
  delimiter = sep;
  int index = line.indexOf (delimiter);
  
  if (index != -1) {
    QString vn = line.left (index);
    if (varname_rx.exactMatch (vn)) {
      varname = vn;
      value = line.mid (index+1);      
    }
    if (value.isNull())
      value = QString ("");
    originalValue = QString (value);
 }

  rawline = line;
  isModified = isNew = false;
}


bool ClipConfigFileLine::getVal (const QString& vname, QString& res, const QRegExp& filter) const {
  if (vname != varname)
    return false;

  int index = filter.indexIn (value);
  if (index == -1)
    res = QString ();
  else
    res = value.mid (index, filter.matchedLength());
  return true;
}


bool ClipConfigFileLine::getVal (const QString& vname, QString& res) const {
  if (vname != varname)
    return false;

  res = value;
  return true;
}


bool ClipConfigFileLine::setVal (const QString& vname, const QString& val) {
  if (vname != varname) 
    return false;

  value = val;
  rawline = varname + delimiter + value;
  isModified = (value != originalValue);
  return true;
}


const QString& ClipConfigFileLine::getLineAndReset () {
  originalValue = value;
  isModified = false;
  isNew = false;
  return rawline;
}



bool ClipConfigFileLine::pendingChange() const {
  return isModified || isNew;
}





ClipConfigFile::ClipConfigFile () {
  delimiter = '=';
}

void ClipConfigFile::setDelimiter (char sep) {
  delimiter = sep;
}


void ClipConfigFile::resetContent () {
  content.clear();
}

void ClipConfigFile::readLine (const QString& line) {
  content.append (ClipConfigFileLine (line, delimiter));
}


QString ClipConfigFile::getVal (const QString& vname, const QRegExp& filter) const {
  QList<ClipConfigFileLine>::const_iterator i;
  QString res;

  for (i=content.constBegin(); i!=content.constEnd(); ++i) {
    if ((*i).getVal (vname, res, filter))
      return res;
  }

  return QString();
}


QString ClipConfigFile::getVal (const QString& vname) const {
  QList<ClipConfigFileLine>::const_iterator i;
  QString res;

  for (i=content.constBegin(); i!=content.constEnd(); ++i) {
    if ((*i).getVal (vname, res))
      return res;
  }

  return QString();
}


void ClipConfigFile::setVal (const QString& vname, const QString& val) {
  QList<ClipConfigFileLine>::iterator i;

  for (i=content.begin(); i!=content.end(); ++i) {
    if ((*i).setVal (vname, val))
      return;
  }

  content.append (ClipConfigFileLine (vname, val, delimiter));
}


bool ClipConfigFile::pendingChanges() const {
  QList<ClipConfigFileLine>::const_iterator i;

  for (i=content.constBegin(); i!=content.constEnd(); ++i) {
    if ((*i).pendingChange())
      return true;
  }

  return false;
}


const QStringList ClipConfigFile::contentToWrite () {
  QStringList res;
  QList<ClipConfigFileLine>::iterator i;
  for (i=content.begin(); i!=content.end(); ++i)
    res.append ((*i).getLineAndReset ());
  return res;
}
