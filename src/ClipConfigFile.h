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

#ifndef CLIPCONFIGFILE_H
#define CLIPCONFIGFILE_H

#include "ConfigFile.h"

#include <QList>


class ClipConfigFileLine {
 public:
  
  ClipConfigFileLine ();
  ClipConfigFileLine (const QString& line, char sep);
  ClipConfigFileLine (const QString& vname, const QString& val, char sep);
  bool getVal (const QString& vname, QString& res) const;
  bool getVal (const QString& vname, QString& res, const QRegExp& filter) const;
  bool setVal (const QString& vname, const QString& val);
  const QString& getLineAndReset ();
  bool pendingChange() const;

 private:
  char delimiter;

  QString varname;
  QString value;
  QString rawline;
  
  QString originalValue;
  bool isModified;
  bool isNew;
};


class ClipConfigFile : public ConfigFile {
 public:
  ClipConfigFile ();
  void setDelimiter (char sep);
  virtual QString getVal (const QString& vname) const;
  virtual QString getVal (const QString& vname, const QRegExp& filter) const;
  virtual void setVal (const QString& vname, const QString& val);
  virtual bool pendingChanges() const;

 protected:
  virtual void resetContent ();
  virtual void readLine (const QString& line);
  virtual const QStringList contentToWrite ();  

 private:
  char delimiter;
  QList<ClipConfigFileLine> content;
};

#endif // CLIPCONFIGFILE_H
