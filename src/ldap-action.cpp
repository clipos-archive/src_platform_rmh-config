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

#include <openssl/buffer.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <openssl/rand.h>
#include <openssl/err.h>

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <stdio.h>

#include "ldap-action.h"


char* errString = NULL;

static void feedback_err (const char *fmt, ...) {
  int unused __attribute__((unused));

  if (errString != NULL) {
    free (errString);
    errString = NULL;
  }

  va_list va;
  va_start (va, fmt);
  unused = vasprintf (&errString, fmt, va);
  va_end (va);
}



LDAP* get_ldap_connection (const char* server_uri) {
  static const int protocol = 3;
  LDAP* ld;
  int rc;

  rc = ldap_initialize (&ld, server_uri);
  if (rc != LDAP_SUCCESS) {
    feedback_err ("Impossible de créer une session LDAP vers %s : erreur %d (%s).\n",
	     server_uri, rc, ldap_err2string(rc));
    return 0;
  }

  if (ldap_set_option (ld, LDAP_OPT_PROTOCOL_VERSION, &protocol) != LDAP_OPT_SUCCESS) {
    feedback_err ("Impossible de configurer LDAP_OPT_PROTOCOL_VERSION.\n");
    goto glc_free;
  }

  return ld;

 glc_free:
  free (ld);
  return 0;
}




int bind_as (LDAP** ld, const char* dn, const char* old_pass) {
  int ldap_errno;
  if ((ldap_errno = ldap_simple_bind_s (*ld, dn, old_pass)) != 0) {
    feedback_err ("Erreur d'association LDAP (bind) : %s", ldap_err2string (ldap_errno));
    if (ld) {
      free (*ld);
      *ld = 0;
    }
    return 1;
  }

  return 0;
}



static int bind_anonymously (LDAP** ld) {
  return bind_as (ld, "", "");
}



int get_dn (LDAP** ld, const char* search_base, const char* uid_field, char** dn) {
  static const char filter[]="(& (employeeType=active) (uid=%s))";
  int ldap_errno;

  char* custom_filter;
  LDAPMessage* msg, * result;
  int ret_val = 0, nEntries;
  char* given_dn;

  ret_val = bind_anonymously (ld);
  if (ret_val != 0)
    goto gd_free;   

  if (asprintf (&custom_filter, filter, uid_field) < 0) {
    feedback_err ("Impossible d'allouer de la mémoire pour le filtre de recherche.\n");
    ret_val = 2;
    goto gd_unbind;   
  }

  if ((ldap_errno = ldap_search_s (*ld, search_base, LDAP_SCOPE_SUBTREE, custom_filter, 0, 0, &result)) != 0) {
    feedback_err ("Erreur lors de la recherche LDAP : %s", ldap_err2string (ldap_errno));
    ret_val = 3;
    goto gd_free_filter;
  }

  nEntries = ldap_count_entries (*ld, result);
  if (nEntries > 1) {
    feedback_err ("Base de données corrompue : plusieurs enregistrements ont le même identifiant.\n");
    ret_val = 4;
    goto gd_msg_result_free;
  }
  if (nEntries != 1) {
    feedback_err ("Impossible de trouver votre identifiant.\n");
    ret_val = 5;
    goto gd_msg_result_free;
  }
  msg = ldap_first_entry (*ld, result);
  given_dn = ldap_get_dn (*ld, msg);
  *dn = (char*) malloc (strlen (given_dn) + 1);
  if (!(*dn)) {
    feedback_err ("Impossible d'allouer de la mémoire pour le nom distinctif.\n");
    ret_val = 6;
    goto gd_msg_result_free;
  }
  strcpy(*dn, given_dn);
  (*dn)[strlen (given_dn)]=0;
 
 gd_msg_result_free:
  ldap_msgfree (result);

 gd_free_filter:
  free (custom_filter);

 gd_unbind:
  if ((ldap_errno = ldap_unbind_s (*ld)) != 0) {
    feedback_err ("Erreur lors de la désassociation LDAP (unbind) : %s", ldap_err2string (ldap_errno));
    if (!ret_val)
      ret_val = 7;
  }
  *ld = 0;

 gd_free:
  if (ld) {
    free (*ld);
    *ld = 0;
  }
  return ret_val;
}


char* prepare_password (const char* pass) {
  char buf[SHA_DIGEST_LENGTH + 4];
  char* salt = &buf[20];
  int len;
  char* tobehashed, *result = 0;
  BIO *mem, *b64;
  BUF_MEM *bptr;
  static const char prefix[] = "{SSHA}";

  if (RAND_bytes((unsigned char*) salt, 4) != 1) {
    feedback_err ("Impossible de tirer un sel au hasard : erreur %lu (%s).\n", ERR_get_error(), ERR_error_string (ERR_get_error(), 0));
    goto pp_end;
  }

  len = strlen(pass);
  tobehashed = (char*) malloc (len+4);
  if (! tobehashed) {
    feedback_err ("Impossible d'allouer de la mémoire pour le traitement du mot de passe.\n");
    goto pp_end;
  }
  strcpy(tobehashed, pass);
  strncpy(tobehashed + len, salt, 4);
  if (SHA1 ((const unsigned char*) tobehashed, len+4, (unsigned char*) buf) != (unsigned char*) buf) {
    feedback_err ("Erreur lors du hachage du mot de passe : Erreur %lu (%s)\n", ERR_get_error(), ERR_error_string (ERR_get_error(), 0));
    goto pp_tbh_free;
  }

  b64 = BIO_new(BIO_f_base64());
  mem = BIO_new(BIO_s_mem());
  mem = BIO_push(b64, mem);
  BIO_write(mem, buf, SHA_DIGEST_LENGTH + 4);
  BIO_flush(mem);
  BIO_get_mem_ptr(mem, &bptr);

  result = (char*) malloc (strlen (prefix) + bptr->length + 1);
  if (! result) {
    feedback_err ("Impossible d'allouer de la mémoire pour le traitement du mot de passe.\n");
    goto pp_bio_free;
  }
  strcpy(result, prefix);
  strncpy(result+strlen (prefix), bptr->data, bptr->length);
  result[strlen (prefix) + bptr->length] = 0;

 pp_bio_free:
  BIO_free(mem);

 pp_tbh_free:
  free (tobehashed);
  
 pp_end:
  return result;
}



int change_password (LDAP** ld, const char* dn, const char* new_pass) {
  int ldap_errno;
   int ret_val = 0;
  LDAPMod modif;
  LDAPMod *modifs[] = {&modif, 0};
  char** new_pass_encoded = 0;
  char attr_name[] = "userPassword";

  modif.mod_op = LDAP_MOD_REPLACE;
  modif.mod_type = attr_name;
  new_pass_encoded = (char**) malloc (2 * sizeof (char*));
  if (!new_pass_encoded){
    feedback_err ("Impossible d'allouer de la mémoire pour le traitement du mot de passe.\n");
    return 1;
  }
  new_pass_encoded[0] = prepare_password (new_pass);
  if (!new_pass_encoded [0]){
    feedback_err ("Impossible d'encoder le mot de passe.\n");
    ret_val = 2;
    goto cp_free_encods;
  }
  new_pass_encoded[1] = 0;
  modif.mod_values = new_pass_encoded;
  
  if ((ldap_errno = ldap_modify_s (*ld, dn, modifs)) != LDAP_SUCCESS) {
    feedback_err ("Erreur lors de la modification du mot de passe : %s", ldap_err2string (ldap_errno));
    ret_val = 3;
    goto cp_free_encod;
  }

 cp_free_encod:
  free (new_pass_encoded[0]);

 cp_free_encods:
  free (new_pass_encoded);
  new_pass_encoded = 0;

  return ret_val;
}
