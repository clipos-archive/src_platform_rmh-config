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

#ifndef LDAP_ACTION_H
#define LDAP_ACTION_H

#define LDAP_DEPRECATED 1
#include <ldap.h>

extern char* errString;

LDAP* get_ldap_connection (const char* uri);
int get_dn (LDAP** ld, const char* base, const char* uid_field, char** dn);
int bind_as (LDAP** ld, const char* dn, const char* old_pass);
int change_password (LDAP** ld, const char* dn, const char* new_pass);


#endif // ifndef LDAP_ACTION_H
