include $(call select_from_repositories,lib/mk/nss3.inc)

NSPR_DIR := $(NSS_PORT_DIR)/nspr
NSS_DIR  := $(NSS_PORT_DIR)/nss
LIBS     += libc

INC_DIR += $(NSPR_DIR)/pr/include \
           $(NSPR_DIR)/pr/include/private \
           $(NSS_DIR)/cmd/lib \
           $(NSS_DIR)/lib/certhigh \
           $(NSS_DIR)/lib/dev \
           $(NSS_DIR)/lib/libpkix/include \
           $(NSS_DIR)/lib/libpkix/pkix/certsel \
           $(NSS_DIR)/lib/libpkix/pkix/checker \
           $(NSS_DIR)/lib/libpkix/pkix/crlsel \
           $(NSS_DIR)/lib/libpkix/pkix/params \
           $(NSS_DIR)/lib/libpkix/pkix/results \
           $(NSS_DIR)/lib/libpkix/pkix/store \
           $(NSS_DIR)/lib/libpkix/pkix/top \
           $(NSS_DIR)/lib/libpkix/pkix/util \
           $(NSS_DIR)/lib/libpkix/pkix_pl_nss/module \
           $(NSS_DIR)/lib/libpkix/pkix_pl_nss/pki \
           $(NSS_DIR)/lib/libpkix/pkix_pl_nss/system \
           $(NSS_DIR)/lib/pki \
           $(NSS_DIR)/lib/util

# nspr
CC_OPT += -D_PR_PTHREADS 
CC_OPT += -UHAVE_CVAR_BUILT_ON_SEM

# nspr/libplds4
SRC_C += lib/ds/plarena.c \
         lib/ds/plhash.c

# nspr/libplc4
SRC_C += lib/libc/src/strlen.c \
         lib/libc/src/strcpy.c \
         lib/libc/src/strdup.c \
         lib/libc/src/strcase.c \
         lib/libc/src/strcat.c \
         lib/libc/src/strcmp.c \
         lib/libc/src/strchr.c \
         lib/libc/src/strpbrk.c \
         lib/libc/src/strstr.c \
         lib/libc/src/strtok.c \
         lib/libc/src/base64.c \
         lib/libc/src/plerror.c \
         lib/libc/src/plgetopt.c

# nspr/libnspr4
SRC_C += pr/src/io/prfdcach.c \
         pr/src/io/priometh.c \
         pr/src/io/prlayer.c \
         pr/src/io/prlog.c \
         pr/src/io/prmwait.c \
         pr/src/io/prprf.c \
         pr/src/linking/prlink.c \
         pr/src/malloc/prmem.c \
         pr/src/md/prosdep.c \
         pr/src/md/unix/freebsd.c \
         pr/src/md/unix/unix.c \
         pr/src/md/unix/unix_errors.c \
         pr/src/memory/prseg.c \
         pr/src/misc/pratom.c \
         pr/src/misc/prdtoa.c \
         pr/src/misc/prenv.c \
         pr/src/misc/prerr.c \
         pr/src/misc/prerror.c \
         pr/src/misc/prerrortable.c \
         pr/src/misc/prinit.c \
         pr/src/misc/prinrval.c \
         pr/src/misc/prlog2.c \
         pr/src/misc/prnetdb.c \
         pr/src/misc/prsystem.c \
         pr/src/misc/prtime.c \
         pr/src/pthreads/ptio.c \
         pr/src/pthreads/ptmisc.c \
         pr/src/pthreads/ptsynch.c \
         pr/src/pthreads/ptthread.c \
         pr/src/threads/prcmon.c \
         pr/src/threads/prrwlock.c \
         pr/src/threads/prtpd.c

# nss/libnssutil3
SRC_C += lib/util/quickder.c \
         lib/util/secdig.c \
         lib/util/derdec.c \
         lib/util/derenc.c \
         lib/util/dersubr.c \
         lib/util/dertime.c \
         lib/util/errstrs.c \
         lib/util/nssb64d.c \
         lib/util/nssb64e.c \
         lib/util/nssrwlk.c \
         lib/util/nssilock.c \
         lib/util/oidstring.c \
         lib/util/pkcs1sig.c \
         lib/util/portreg.c \
         lib/util/secalgid.c \
         lib/util/secasn1d.c \
         lib/util/secasn1e.c \
         lib/util/secasn1u.c \
         lib/util/secitem.c \
         lib/util/secload.c \
         lib/util/secoid.c \
         lib/util/sectime.c \
         lib/util/secport.c \
         lib/util/templates.c \
         lib/util/utf8.c \
         lib/util/utilmod.c \
         lib/util/utilpars.c \
         lib/util/pkcs11uri.c

# nss/libnss3
SRC_C += lib/base/arena.c \
         lib/base/error.c \
         lib/base/errorval.c \
         lib/base/hashops.c \
         lib/base/libc.c \
         lib/base/tracker.c \
         lib/base/item.c \
         lib/base/utf8.c \
         lib/base/list.c \
         lib/base/hash.c \
         lib/certdb/alg1485.c \
         lib/certdb/certdb.c \
         lib/certdb/certv3.c \
         lib/certdb/certxutl.c \
         lib/certdb/crl.c \
         lib/certdb/genname.c \
         lib/certdb/stanpcertdb.c \
         lib/certdb/polcyxtn.c \
         lib/certdb/secname.c \
         lib/certdb/xauthkid.c \
         lib/certdb/xbsconst.c \
         lib/certdb/xconst.c \
         lib/certhigh/certhtml.c \
         lib/certhigh/certreq.c \
         lib/certhigh/crlv2.c \
         lib/certhigh/ocsp.c \
         lib/certhigh/ocspsig.c \
         lib/certhigh/certhigh.c \
         lib/certhigh/certvfy.c \
         lib/certhigh/certvfypkix.c \
         lib/certhigh/xcrldist.c \
         lib/ckfw/crypto.c \
         lib/ckfw/find.c \
         lib/ckfw/hash.c \
         lib/ckfw/instance.c \
         lib/ckfw/mutex.c \
         lib/ckfw/object.c \
         lib/ckfw/session.c \
         lib/ckfw/sessobj.c \
         lib/ckfw/slot.c \
         lib/ckfw/token.c \
         lib/ckfw/wrap.c \
         lib/ckfw/mechanism.c \
         lib/cryptohi/sechash.c \
         lib/cryptohi/seckey.c \
         lib/cryptohi/secsign.c \
         lib/cryptohi/secvfy.c \
         lib/cryptohi/dsautil.c \
         lib/dev/devslot.c \
         lib/dev/devtoken.c \
         lib/dev/devutil.c \
         lib/dev/ckhelper.c \
         lib/libpkix/pkix/certsel/pkix_certselector.c \
         lib/libpkix/pkix/certsel/pkix_comcertselparams.c \
         lib/libpkix/pkix/checker/pkix_basicconstraintschecker.c \
         lib/libpkix/pkix/checker/pkix_certchainchecker.c \
         lib/libpkix/pkix/checker/pkix_crlchecker.c \
         lib/libpkix/pkix/checker/pkix_ekuchecker.c \
         lib/libpkix/pkix/checker/pkix_expirationchecker.c \
         lib/libpkix/pkix/checker/pkix_namechainingchecker.c \
         lib/libpkix/pkix/checker/pkix_nameconstraintschecker.c \
         lib/libpkix/pkix/checker/pkix_ocspchecker.c \
         lib/libpkix/pkix/checker/pkix_revocationmethod.c \
         lib/libpkix/pkix/checker/pkix_revocationchecker.c \
         lib/libpkix/pkix/checker/pkix_policychecker.c \
         lib/libpkix/pkix/checker/pkix_signaturechecker.c \
         lib/libpkix/pkix/checker/pkix_targetcertchecker.c \
         lib/libpkix/pkix/crlsel/pkix_crlselector.c \
         lib/libpkix/pkix/crlsel/pkix_comcrlselparams.c \
         lib/libpkix/pkix/params/pkix_trustanchor.c \
         lib/libpkix/pkix/params/pkix_procparams.c \
         lib/libpkix/pkix/params/pkix_valparams.c \
         lib/libpkix/pkix/params/pkix_resourcelimits.c \
         lib/libpkix/pkix/results/pkix_buildresult.c \
         lib/libpkix/pkix/results/pkix_policynode.c \
         lib/libpkix/pkix/results/pkix_valresult.c \
         lib/libpkix/pkix/results/pkix_verifynode.c \
         lib/libpkix/pkix/store/pkix_store.c \
         lib/libpkix/pkix/top/pkix_validate.c \
         lib/libpkix/pkix/top/pkix_lifecycle.c \
         lib/libpkix/pkix/top/pkix_build.c \
         lib/libpkix/pkix/util/pkix_tools.c \
         lib/libpkix/pkix/util/pkix_error.c \
         lib/libpkix/pkix/util/pkix_logger.c \
         lib/libpkix/pkix/util/pkix_list.c \
         lib/libpkix/pkix/util/pkix_errpaths.c \
         lib/libpkix/pkix_pl_nss/module/pkix_pl_aiamgr.c \
         lib/libpkix/pkix_pl_nss/module/pkix_pl_colcertstore.c \
         lib/libpkix/pkix_pl_nss/module/pkix_pl_httpcertstore.c \
         lib/libpkix/pkix_pl_nss/module/pkix_pl_httpdefaultclient.c \
         lib/libpkix/pkix_pl_nss/module/pkix_pl_ldaptemplates.c \
         lib/libpkix/pkix_pl_nss/module/pkix_pl_ldapcertstore.c \
         lib/libpkix/pkix_pl_nss/module/pkix_pl_ldapresponse.c \
         lib/libpkix/pkix_pl_nss/module/pkix_pl_ldaprequest.c \
         lib/libpkix/pkix_pl_nss/module/pkix_pl_ldapdefaultclient.c \
         lib/libpkix/pkix_pl_nss/module/pkix_pl_nsscontext.c \
         lib/libpkix/pkix_pl_nss/module/pkix_pl_pk11certstore.c \
         lib/libpkix/pkix_pl_nss/module/pkix_pl_socket.c \
         lib/libpkix/pkix_pl_nss/pki/pkix_pl_basicconstraints.c \
         lib/libpkix/pkix_pl_nss/pki/pkix_pl_cert.c \
         lib/libpkix/pkix_pl_nss/pki/pkix_pl_certpolicyinfo.c \
         lib/libpkix/pkix_pl_nss/pki/pkix_pl_certpolicymap.c \
         lib/libpkix/pkix_pl_nss/pki/pkix_pl_certpolicyqualifier.c \
         lib/libpkix/pkix_pl_nss/pki/pkix_pl_crl.c \
         lib/libpkix/pkix_pl_nss/pki/pkix_pl_crldp.c \
         lib/libpkix/pkix_pl_nss/pki/pkix_pl_crlentry.c \
         lib/libpkix/pkix_pl_nss/pki/pkix_pl_date.c \
         lib/libpkix/pkix_pl_nss/pki/pkix_pl_generalname.c \
         lib/libpkix/pkix_pl_nss/pki/pkix_pl_infoaccess.c \
         lib/libpkix/pkix_pl_nss/pki/pkix_pl_nameconstraints.c \
         lib/libpkix/pkix_pl_nss/pki/pkix_pl_ocsprequest.c \
         lib/libpkix/pkix_pl_nss/pki/pkix_pl_ocspresponse.c \
         lib/libpkix/pkix_pl_nss/pki/pkix_pl_publickey.c \
         lib/libpkix/pkix_pl_nss/pki/pkix_pl_x500name.c \
         lib/libpkix/pkix_pl_nss/pki/pkix_pl_ocspcertid.c \
         lib/libpkix/pkix_pl_nss/system/pkix_pl_bigint.c \
         lib/libpkix/pkix_pl_nss/system/pkix_pl_bytearray.c \
         lib/libpkix/pkix_pl_nss/system/pkix_pl_common.c \
         lib/libpkix/pkix_pl_nss/system/pkix_pl_error.c \
         lib/libpkix/pkix_pl_nss/system/pkix_pl_hashtable.c \
         lib/libpkix/pkix_pl_nss/system/pkix_pl_lifecycle.c \
         lib/libpkix/pkix_pl_nss/system/pkix_pl_mem.c \
         lib/libpkix/pkix_pl_nss/system/pkix_pl_monitorlock.c \
         lib/libpkix/pkix_pl_nss/system/pkix_pl_mutex.c \
         lib/libpkix/pkix_pl_nss/system/pkix_pl_object.c \
         lib/libpkix/pkix_pl_nss/system/pkix_pl_oid.c \
         lib/libpkix/pkix_pl_nss/system/pkix_pl_primhash.c \
         lib/libpkix/pkix_pl_nss/system/pkix_pl_rwlock.c \
         lib/libpkix/pkix_pl_nss/system/pkix_pl_string.c \
         lib/nss/nssinit.c \
         lib/nss/nssoptions.c \
         lib/nss/nssver.c \
         lib/nss/utilwrap.c \
         lib/pk11wrap/dev3hack.c \
         lib/pk11wrap/pk11akey.c \
         lib/pk11wrap/pk11auth.c \
         lib/pk11wrap/pk11cert.c \
         lib/pk11wrap/pk11cxt.c \
         lib/pk11wrap/pk11err.c \
         lib/pk11wrap/pk11hpke.c \
         lib/pk11wrap/pk11kea.c \
         lib/pk11wrap/pk11list.c \
         lib/pk11wrap/pk11load.c \
         lib/pk11wrap/pk11mech.c \
         lib/pk11wrap/pk11merge.c \
         lib/pk11wrap/pk11nobj.c \
         lib/pk11wrap/pk11obj.c \
         lib/pk11wrap/pk11pars.c \
         lib/pk11wrap/pk11pbe.c \
         lib/pk11wrap/pk11pk12.c \
         lib/pk11wrap/pk11pqg.c \
         lib/pk11wrap/pk11sdr.c \
         lib/pk11wrap/pk11skey.c \
         lib/pk11wrap/pk11slot.c \
         lib/pk11wrap/pk11util.c \
         lib/pki/asymmkey.c \
         lib/pki/certificate.c \
         lib/pki/cryptocontext.c \
         lib/pki/symmkey.c \
         lib/pki/trustdomain.c \
         lib/pki/tdcache.c \
         lib/pki/certdecode.c \
         lib/pki/pkistore.c \
         lib/pki/pkibase.c \
         lib/pki/pki3hack.c

vpath %.c $(NSPR_DIR)
vpath %.c $(NSS_DIR)

SHARED_LIB = yes

CC_CXX_WARN_STRICT =
