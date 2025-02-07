/*
 * WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING
 *
 *	Openvision retains the copyright to derivative works of
 *	this source code.  Do *NOT* create a derivative of this
 *	source code before consulting with your legal department.
 *	Do *NOT* integrate *ANY* of this source code into another
 *	product before consulting with your legal department.
 *
 *	For further information, read the top-level Openvision
 *	copyright which is contained in the top-level MIT Kerberos
 *	copyright.
 *
 * WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING
 *
 */


#include    <unistd.h>
#include    <string.h>
#include    <stdlib.h>
#include    "autoconf.h"
#ifdef HAVE_MEMORY_H
#include    <memory.h>
#endif

#include    <k5-int.h>
#include <kadm5/admin.h>
#include <kadm5/server_internal.h>
#include    <kdb.h>
#include    "import_err.h"
#include    "kdb5_util.h"
#include    "nstrtok.h"

#define LINESIZE	32768 /* XXX */
#define PLURAL(count)	(((count) == 1) ? error_message(IMPORT_SINGLE_RECORD) : error_message(IMPORT_PLURAL_RECORDS))

extern caddr_t xdralloc_getdata(XDR *xdrs);

static int parse_pw_hist_ent(current, hist)
   char *current;
   osa_pw_hist_ent *hist;
{
     int tmp, i, j, ret;
     char *cp;

     ret = 0;
     hist->n_key_data = 1;

     hist->key_data = (krb5_key_data *) malloc(hist->n_key_data *
					       sizeof(krb5_key_data));
     if (hist->key_data == NULL)
	  return ENOMEM;
     memset(hist->key_data, 0, sizeof(krb5_key_data)*hist->n_key_data);

     for (i = 0; i < hist->n_key_data; i++) {
	  krb5_key_data *key_data = &hist->key_data[i];

	  key_data->key_data_ver = 1;

	  if((cp = nstrtok((char *) NULL, "\t")) == NULL) {
	       com_err(NULL, IMPORT_BAD_RECORD, "%s", current);
	       ret = IMPORT_FAILED;
	       goto done;
	  }
	  key_data->key_data_type[0] = atoi(cp);

	  if((cp = nstrtok((char *) NULL, "\t")) == NULL) {
	       com_err(NULL, IMPORT_BAD_RECORD, "%s", current);
	       ret =  IMPORT_FAILED;
	       goto done;
	  }
	  key_data->key_data_length[0] = atoi(cp);

	  if((cp = nstrtok((char *) NULL, "\t")) == NULL) {
	       com_err(NULL, IMPORT_BAD_RECORD, "%s", current);
	       ret = IMPORT_FAILED;
	       goto done;
	  }
	  if(!(key_data->key_data_contents[0] =
	       (krb5_octet *) malloc(key_data->key_data_length[0]+1))) {
	       ret = ENOMEM;
	       goto done;
	  }
	  for(j = 0; j < key_data->key_data_length[0]; j++) {
	       if(sscanf(cp, "%02x", &tmp) != 1) {
		    com_err(NULL, IMPORT_BAD_RECORD, "%s", current);
		    ret = IMPORT_FAILED;
		    goto done;
	       }
	       key_data->key_data_contents[0][j] = tmp;
	       cp = strchr(cp, ' ') + 1;
	  }
     }

done:
     return ret;
}

/*
 * Function: parse_principal
 *
 * Purpose: parse principal line in db dump file
 *
 * Arguments:
 *	<return value>	0 on success, error code on failure
 *
 * Requires:
 *	principal database to be opened.
 *	nstrtok(3) to have a valid buffer in memory.
 *
 * Effects:
 *	[effects]
 *
 * Modifies:
 *	[modifies]
 *
 */
int process_ov_principal(fname, kcontext, filep, verbose, linenop)
    char		*fname;
    krb5_context	kcontext;
    FILE		*filep;
    int			verbose;
    int			*linenop;
{
    XDR			    xdrs;
    osa_princ_ent_t	    rec;
    krb5_error_code	    ret;
    krb5_tl_data	    tl_data;
    krb5_principal	    princ;
    krb5_db_entry	    kdb;
    char		    *current;
    char		    *cp;
    int			    x, one;
    krb5_boolean	    more;
    char		    line[LINESIZE];

    if (fgets(line, LINESIZE, filep) == (char *) NULL) {
	 return IMPORT_BAD_FILE;
    }
    if((cp = nstrtok(line, "\t")) == NULL)
	return IMPORT_BAD_FILE;
    if((rec = (osa_princ_ent_t) malloc(sizeof(osa_princ_ent_rec))) == NULL)
	return ENOMEM;
    memset(rec, 0, sizeof(osa_princ_ent_rec));
    if((ret = krb5_parse_name(kcontext, cp, &princ)))
	goto done;
    krb5_unparse_name(kcontext, princ, &current);
    if((cp = nstrtok((char *) NULL, "\t")) == NULL) {
	com_err(NULL, IMPORT_BAD_RECORD, "%s", current);
	ret =  IMPORT_FAILED;
	goto done;
    } else {
	if(strcmp(cp, "")) {
	    if((rec->policy = (char *) malloc(strlen(cp)+1)) == NULL)  {
		ret = ENOMEM;
		goto done;
	    }
	    strcpy(rec->policy, cp);
	} else rec->policy = NULL;
    }
    if((cp = nstrtok((char *) NULL, "\t")) == NULL) {
	com_err(NULL, IMPORT_BAD_RECORD, "%s", current);
	ret = IMPORT_FAILED;
	goto done;
    }
    rec->aux_attributes = strtol(cp, (char  **)NULL, 16);
    if((cp = nstrtok((char *) NULL, "\t")) == NULL) {
	com_err(NULL, IMPORT_BAD_RECORD, "%s", current);
	ret = IMPORT_FAILED;
	goto done;
    }
    rec->old_key_len = atoi(cp);
    if((cp = nstrtok((char *) NULL, "\t")) == NULL) {
	com_err(NULL, IMPORT_BAD_RECORD, "%s", current);
	ret = IMPORT_FAILED;
	goto done;
    }
    rec->old_key_next = atoi(cp);
    if((cp = nstrtok((char *) NULL, "\t")) == NULL) {
	com_err(NULL, IMPORT_BAD_RECORD, "%s", current);
	ret = IMPORT_FAILED;
	goto done;
    }
    rec->admin_history_kvno = atoi(cp);
    if (! rec->old_key_len) {
       rec->old_keys = NULL;
    } else {
       if(!(rec->old_keys = (osa_pw_hist_ent *)
	    malloc(sizeof(osa_pw_hist_ent) * rec->old_key_len))) {
	  ret = ENOMEM;
	  goto done;
       }
       memset(rec->old_keys,0,
	      sizeof(osa_pw_hist_ent) * rec->old_key_len);
       for(x = 0; x < rec->old_key_len; x++)
	    parse_pw_hist_ent(current, &rec->old_keys[x]);
    }

    xdralloc_create(&xdrs, XDR_ENCODE);
    if (! xdr_osa_princ_ent_rec(&xdrs, rec)) {
	 xdr_destroy(&xdrs);
	 ret = KADM5_XDR_FAILURE;
	 goto done;
    }

    tl_data.tl_data_type = KRB5_TL_KADM_DATA;
    tl_data.tl_data_length = xdr_getpos(&xdrs);
    tl_data.tl_data_contents = (krb5_octet *) xdralloc_getdata(&xdrs);

    one = 1;
    ret = krb5_db_get_principal(kcontext, princ, &kdb, &one, &more);
    if (ret)
	 goto done;

    ret = krb5_dbe_update_tl_data(kcontext, &kdb, &tl_data);
    if (ret)
	 goto done;

    ret = krb5_db_put_principal(kcontext, &kdb, &one);
    if (ret)
	 goto done;

    xdr_destroy(&xdrs);

    (*linenop)++;

done:
    free(current);
    krb5_free_principal(kcontext, princ);
    osa_free_princ_ent(rec);
    return ret;
}
