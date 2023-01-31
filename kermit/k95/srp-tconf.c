/*
 * Copyright (c) 1997 Stanford University
 *
 * The use of this software for revenue-generating purposes may require a
 * license from the owners of the underlying intellectual property.
 *
 * Within that constraint, permission to use, copy, modify, and distribute
 * this software and its documentation for any purpose is hereby granted
 * without fee, provided that the above copyright notices and this permission
 * notice appear in all copies of the software and related documentation.
 *
 * THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY
 * WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.
 *
 * IN NO EVENT SHALL STANFORD BE LIABLE FOR ANY SPECIAL, INCIDENTAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND, OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER OR NOT ADVISED OF
 * THE POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF LIABILITY, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef OS2
#ifndef _WIN32
#include <unistd.h>     /* close getlogin */
#endif
#endif
#include <stdlib.h>     /* atexit exit */
#include <stdio.h>
#include <stdarg.h>
#ifdef OS2
#ifndef _WIN32
#include <io.h>
#endif /* _WIN32 */
#endif /* OS2 */
#include <string.h>
#include "getopt.h"

#include <t_pwd.h>

#ifndef SSLDLL
#define ck_OPENSSL_add_all_algorithms_noconf OPENSSL_add_all_algorithms_noconf
#endif /* SSLDLL */

#define MIN_BASIS_BITS 257
#define BASIS_BITS 2048

#ifndef _WIN32
extern int errno;
#endif

struct pre_struct {
  char * pre_mod;
  char * pre_gen;
  char * comment;
} pre_params[] = {
  { "HMujfBWu4LfBFA0j3PpN7UbgUYfv.rMoMNuVRMoekpZ",
    "2",
    NULL },
  { "W2KsCfRxb3/ELBvnVWufMA0gbdBlLXbJihgZkgp3xLTKwtPCUhSOHNZ5VLb9pBGR",
    "2",
    NULL },
  { "3Kn/YYiomHkFkfM1x4kayR125MGkzpLUDy3y14FlTMwYnhZkjrMXnoC2TcFAecNlU5kFzgcpKYUbBOPZFRtyf3",
    "2",
    NULL },
  { "CbDP.jR6YD6wAj2ByQWxQxQZ7.9J9xkn2.Uqb3zVm16vQyizprhBw9hi80psatZ8k54vwZfiIeEHZVsDnyqeWSSIpWso.wh5GD4OFgdhVI3",
    "2",
    NULL },
  { "iqJ7nFZ4bGCRjE1F.FXEwL085Zb0kLM2TdHDaVVCdq0cKxvnH/0FLskJTKlDtt6sDl89dc//aEULTVFGtcbA/tDzc.bnFE.DWthQOu2n2JwKjgKfgCR2lZFWXdnWmoOh",
    "2",
    NULL },
  { "///////////93zgY8MZ2DCJ6Oek0t1pHAG9E28fdp7G22xwcEnER8b5A27cED0JTxvKPiyqwGnimAmfjybyKDq/XDMrjKS95v8MrTc9UViRqJ4BffZes8F//////////",
    "7",
    "oakley prime 1" },
  { "Ewl2hcjiutMd3Fu2lgFnUXWSc67TVyy2vwYCKoS9MLsrdJVT9RgWTCuEqWJrfB6uE3LsE9GkOlaZabS7M29sj5TnzUqOLJMjiwEzArfiLr9WbMRANlF68N5AVLcPWvNx6Zjl3m5Scp0BzJBz9TkgfhzKJZ.WtP3Mv/67I/0wmRZ",
    "2",
    NULL },
  { "F//////////oG/QeY5emZJ4ncABWDmSqIa2JWYAPynq0Wk.fZiJco9HIWXvZZG4tU.L6RFDEaCRC2iARV9V53TFuJLjRL72HUI5jNPYNdx6z4n2wQOtxMiB/rosz0QtxUuuQ/jQYP.bhfya4NnB7.P9A6PHxEPJWV//////////",
    "5",
    "oakley prime 2" },
  { "3NUKQ2Re4P5BEK0TLg2dX3gETNNNECPoe92h4OVMaDn3Xo/0QdjgG/EvM.hiVV1BdIGklSI14HA38Mpe5k04juR5/EXMU0r1WtsLhNXwKBlf2zEfoOh0zVmDvqInpU695f29Iy7sNW3U5RIogcs740oUp2Kdv5wuITwnIx84cnO.e467/IV1lPnvMCr0pd1dgS0a.RV5eBJr03Q65Xy61R",
    "2",
    NULL },
  { "dUyyhxav9tgnyIg65wHxkzkb7VIPh4o0lkwfOKiPp4rVJrzLRYVBtb76gKlaO7ef5LYGEw3G.4E0jbMxcYBetDy2YdpiP/3GWJInoBbvYHIRO9uBuxgsFKTKWu7RnR7yTau/IrFTdQ4LY/q.AvoCzMxV0PKvD9Odso/LFIItn8PbTov3VMn/ZEH2SqhtpBUkWtmcIkEflhX/YY/fkBKfBbe27/zUaKUUZEUYZ2H2nlCL60.JIPeZJSzsu/xHDVcx",
    "2",
    NULL },
  { "2iQzj1CagQc/5ctbuJYLWlhtAsPHc7xWVyCPAKFRLWKADpASkqe9djWPFWTNTdeJtL8nAhImCn3Sr/IAdQ1FrGw0WvQUstPx3FO9KNcXOwisOQ1VlL.gheAHYfbYyBaxXL.NcJx9TUwgWDT0hRzFzqSrdGGTN3FgSTA1v4QnHtEygNj3eZ.u0MThqWUaDiP87nqha7XnT66bkTCkQ8.7T8L4KZjIImrNrUftedTTBi.WCi.zlrBxDuOM0da0JbUkQlXqvp0yvJAPpC11nxmmZOAbQOywZGmu9nhZNuwTlxjfIro0FOdthaDTuZRL9VL7MRPUDo/DQEyW.d4H.UIlzp",
    "2",
    NULL }
};

#define NPARAMS (sizeof(pre_params) / sizeof(struct pre_struct))

char *progName;

int  debug   = 0;
int  verbose = 0;
int  composite = 0;

/* Some C-Kermit modules redefine printf to Vscrnprintf which normally only
 * exists within the main CKW (k95g.exe/k95.exe) so we've got to provide a
 * compatible definition here.
 */
int Vscrnprintf(const char * format, ...) {
    int result = 0;
    va_list ap;

    va_start(ap, format);
    result = vprintf(format, ap);
    va_end(ap);

    return result;
}

#ifndef NOT_KERMIT
/* ckossl.c only provides this function when NOT_KERMIT is supplied, which it
 * isn't when building srp-tconf */
void fatal(char *msg) {
    if (!msg) msg = "";

    printf(msg);
    exit(1);        /* Exit indicating failure */
}
#endif /*NOT_KERMIT */

int
main(argc, argv)
     int argc;
     char *argv[];
{
  char *chp;
  char *configFile = NULL;
  char cbuf[256];
  char b64buf[MAXB64PARAMLEN];
  int c, ch, i, lastidx, keylen, yesno, fsize, status;
  FILE *efp;

  struct t_conf * tc = NULL;
  struct t_confent * tcent;

  progName = *argv;
  if ((chp = strrchr(progName, '/')) != (char *) 0) progName = chp + 1;

  while ((ch = getopt(argc, argv, "dv2c:")) != EOF)
    switch(ch) {
    case 'c':
      configFile = optarg;
      break;
    case 'v':
      verbose++;
      break;
    case 'd':
      debug++;
      break;
    case '2':
      composite++;
      break;
    default:
      fprintf(stderr, "usage: %s [-dv2] [-c configfile]\n", progName);
      exit(1);
    }

#ifdef SSLDLL
    if ( !ck_crypto_loaddll() ) {
        fprintf(stderr, "%s: unable to load crypto dll\n",progName);
        exit(1);
    }
#endif /* SSLDLL */

    ck_OPENSSL_add_all_algorithms_noconf();
  argc -= optind;
  argv += optind;

    if (configFile == NULL)
        configFile = (char *)t_defaultconf();

    efp = fopen(configFile, "a+");
    if(efp == NULL) {
        if (creat(configFile, 0644) < 0 || (efp = fopen(configFile, "a+")) == NULL) {
            fprintf(stderr, "%s: unable to create %s (errno = %d)\n",
                    progName, configFile, errno);
            exit(2);
        } else {
            printf("%s: Creating new configuration file %s\n", progName, configFile);
        }
    }

  tc = t_openconf(efp);
  if(tc == NULL) {
    fprintf(stderr, "%s: unable to open configuration file %s\n",
            progName, configFile);
    exit(2);
  }

  tcent = t_getconflast(tc);
  if(tcent == NULL)
    lastidx = 0;
  else
    lastidx = tcent->index;

  if(lastidx > 0) {
    keylen = 8 * tcent->modulus.len;
    printf("Current field size is %d bits.\n", keylen);
    printf("\nIncrease the default field size? [y] ");
    yesno = 0;
    while((c = getchar()) != '\n' && c != EOF)
      if(yesno == 0)
        if(c == 'n' || c == 'N')
          yesno = -1;
        else if(c == 'y' || c == 'Y')
          yesno = 1;
    if(c == EOF || yesno < 0)
      exit(0);
  }
  else {
    lastidx = 0;
    keylen = 0;
  }

  tcent = t_newconfent(tc);

  printf("\nGenerate a (n)ew field or use a (p)redefined field? [nP] ");
  fgets(cbuf, sizeof(cbuf), stdin);
  if(*cbuf != 'n' && *cbuf != 'N') {
    for(i = 0; i < NPARAMS; ++i) {
      tcent->modulus.len = t_fromb64(tcent->modulus.data,
        pre_params[i].pre_mod);
      printf("(%d) [%d bits]  %s\n    Modulus = %s\n  Generator = %s\n",
             i + 1, 8 * tcent->modulus.len,
             pre_params[i].comment ? pre_params[i].comment : "",
             pre_params[i].pre_mod, pre_params[i].pre_gen);
    }
    printf("\nSelect a field (1-%d): ", NPARAMS);
    fgets(cbuf, sizeof(cbuf), stdin);
    i = atoi(cbuf);
    if(i <= 0 || i > NPARAMS) {
      fprintf(stderr, "Index not in range\n");
      exit(1);
    }
    tcent->index = lastidx + 1;
    tcent->modulus.len = t_fromb64(tcent->modulus.data,
      pre_params[i - 1].pre_mod);
    tcent->generator.len = t_fromb64(tcent->generator.data,
      pre_params[i - 1].pre_gen);
    t_putconfent(tcent, efp);
    t_closeconf(tc);
    fclose(efp);
    printf("Configuration file updated.\n");
    exit(0);
  }

  printf("\nEnter the new field size, in bits.  Suggested sizes:\n\n");
  printf(" %3d (minimum, testing only)\n", MIN_BASIS_BITS);
  printf(" 384 (low security, but fast)\n");
  printf(" 512 (reasonable default)\n");
  printf(" 768 (better security)\n");
  printf("1024 (PGP-level security)\n");
  printf("1536 (extremely secure, possibly slow)\n");
  printf("2048 (maximum supported security level)\n");
  printf("\nField size (%d to %d): ", MIN_BASIS_BITS, BASIS_BITS);

  fgets(cbuf, sizeof(cbuf), stdin);
  fsize = atoi(cbuf);
  if(fsize < MIN_BASIS_BITS || fsize > BASIS_BITS) {
    fprintf(stderr, "%s: field size must be between %d and %d\n",
            progName, MIN_BASIS_BITS, BASIS_BITS);
    exit(1);
  }

  if(fsize <= keylen)
    fprintf(stderr, "Warning: new field size is not larger than old field size\n");

  printf("\nInitializing random number generator...");
  fflush(stdout);
  t_stronginitrand();

  if(composite)
    printf("done.\n\nGenerating a %d-bit composite with safe prime factors.  This may take a while.\n", fsize);
  else
    printf("done.\n\nGenerating a %d-bit safe prime.  This may take a while.\n", fsize);

  while(1) {
    while((tcent = (composite ? t_makeconfent_c(tc, fsize) :
                                t_makeconfent(tc, fsize))) == NULL)
      printf("Parameter generation failed, retrying...\n");
    tcent->index = lastidx + 1;

    printf("\nParameters successfully generated.\n");
    printf("N = [%s]\n", t_tob64(b64buf,
                                 tcent->modulus.data, tcent->modulus.len));
    printf("g = [%s]\n", t_tob64(b64buf,
                                 tcent->generator.data, tcent->generator.len));
    printf("\nUpdate the configuration file with these parameters? [Ynq] ");

    fgets(cbuf, sizeof(cbuf), stdin);
    switch(*cbuf) {
    case 'q':
    case 'Q':
      fclose(efp);
      exit(0);
    case 'n':
    case 'N':
      printf("\nGenerating another set of parameters, please wait...\n");
      break;
    default:
      t_putconfent(tcent, efp);
      t_closeconf(tc);
      fclose(efp);
      printf("Configuration file updated.\n");
      exit(0);
    }
  }
}
