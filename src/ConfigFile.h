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

#ifndef CONFIGFILE_H
#define CONFIGFILE_H

#include <QString>
#include <QStringList>

class ConfigFile {
 public:
  void readFile (const QString& filename);

  virtual QString getVal (const QString& vname) const;
  virtual QString getVal (const QString& vname, const QRegExp& filter) const;
  virtual void setVal (const QString& vname, const QString& val);
  virtual QString getNS (int ni) const;
  virtual void setNS (int n, const QString& val);

  virtual bool pendingChanges() const = 0;
  void writeFile ();
  void writeFileAs (const QString& filename);

 protected:
  virtual void resetContent () = 0;
  virtual void readLine (const QString& line) = 0;
  virtual const QStringList contentToWrite () = 0;

 private:
  QString filename;
};

#endif // CONFIGFILE_H
