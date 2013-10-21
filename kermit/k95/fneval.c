From fdc@watsun.cc.columbia.edu Mon Mar 31 18:52:00 1997
Flags: 000000000005
Received: (from fdc@localhost)
	by watsun.cc.columbia.edu (8.8.5/8.8.5) id SAA12688;
	Mon, 31 Mar 1997 18:51:59 -0500 (EST)
Date: Mon, 31 Mar 1997 18:51:59 -0500 (EST)
From: Frank da Cruz <fdc@watsun.cc.columbia.edu>
Message-Id: <199703312351.SAA12688@watsun.cc.columbia.edu>
To: jaltman@watsun.cc.columbia.edu
Subject: New fneval()

I'm also going to put this in the patch file, but without the #ifdef OS2
parts...

---(cut)---
static char *				/* Evaluate builtin functions */
fneval(fn,argp,argn,xp) char *fn, *argp[]; int argn; char * xp; {
    int i, j, k, len1, len2, len3, n, x, y;
    char *bp[FNARGS + 1];		/* Pointers to malloc'd strings */
    char c;
    char *p, *s;
    char *val1, *val2;			/* Pointers to numeric string values */
/*
  IMPORTANT: Note that argn is not an accurate count of the number of
  arguments.  We can't really tell if an argument is null until after we
  execute the code below.  So argn is really the maximum number of arguments
  we might have.  In particular note that argn is always at least 1, even
  if the function is called with empty parentheses (but don't count on it).
*/
    if (!fn) fn = "";			/* Protect against null pointers */
    debug(F111,"fneval",fn,argn);
    debug(F110,"fneval",argp[0],0);
    if (argn > FNARGS) argn = FNARGS;	/* And too many arguments */

    y = lookup(fnctab,fn,nfuncs,&x);
    if (y < 0)				/* bad function name */
      return("");			/* so value is null */

#ifdef DEBUG
    if (deblog) {
	int j;
	for (j = 0; j < argn; j++)
	  debug(F111,"fneval function arg",argp[j],j);
    }
#endif /* DEBUG */
/*
  \fliteral() and \fcontents() are special functions that do not evaluate
  their arguments, and are treated specially here.  After these come the
  functions whose arguments are evaluated in the normal way.
*/
    if (y == FN_LIT) {			/* literal(arg1) */
	debug(F110,"flit",xp,0);
	return(xp ? xp : "");		/* return a pointer to arg itself */
    }
    if (y == FN_CON) {			/* Contents of variable, unexpanded. */
	char c;
	if (!(p = argp[0]) || !*p) return("");
	p = brstrip(p);
	if (*p == CMDQ) p++;
	if ((c = *p) == '%') {		/* Scalar variable. */
	    c = *++p;			/* Get ID character. */
	    p = "";			/* Assume definition is empty */
	    if (!c) return(p);		/* Double paranoia */
	    if (c >= '0' && c <= '9') { /* Digit for macro arg */
		if (maclvl < 0)		/* Digit variables are global */
		  p = g_var[c];		/* if no macro is active */
		else			/* otherwise */
		  p = m_arg[maclvl][c - '0']; /* they're on the stack */
	    } else {
		if (isupper(c)) c -= ('a'-'A');
		p = g_var[c];		/* Letter for global variable */
	    }
	    return(p ? p : "");
	}
	if (c == '&') {			/* Array reference. */
	    int vbi, d;
	    if (arraynam(p,&vbi,&d) < 0) /* Get name and subscript */
	      return("");
	    if (chkarray(vbi,d) > 0) {	/* Array is declared? */
		vbi -= ARRAYBASE;	/* Convert name to index */
		if (a_dim[vbi] >= d) {	/* If subscript in range */
		    char **ap;
		    ap = a_ptr[vbi];	/* get data pointer */
		    if (ap) {		/* and if there is one */
			return(ap[d]);	/* return what it points to */
		    }
		}
	    } else return("");
	}
    }

    for (i = 0; i < FNARGS; i++)	/* Initialize all argument pointers */
      bp[i] = NULL;

    for (i = 0; i < argn; i++) {	/* Loop to expand each argument */
	n = MAXARGLEN;			/* Allow plenty of space */
	bp[i] = s = malloc(n+1);	/* Allocate space for this argument */
	if (bp[i] == NULL) {		/* handle failure to get space */
	    for (k = 0; k < i; k++) if (bp[k]) free(bp[k]);
	    debug(F101,"fneval malloc failure, arg","",i);
	    return("");
	}
	p = argp[i] ? argp[i] : "";	/* Point to this argument */

/*
  Trim leading and trailing spaces from the original argument, before
  evaluation.  This code new to edit 184.
*/
	{
	    int x, j;
	    x = strlen(p);
	    if (*p == '{' && *(p+x-1) == '}') {
		p[x-1] = NUL;
		p++;
		x -= 2;
	    } else {
		j = x - 1;		/* Trim trailing whitespace */
		while (j > 0 && (*(p + j) == SP || *(p + j) == HT))
		  *(p + j--) = NUL;
		while (*p == SP || *p == HT) /* Strip leading whitespace */
		  p++;
	    }
	}

/* Now evaluate the argument */

	if (zzstring(p,&s,&n) < 0) {	/* Expand arg into new space */
	    debug(F101,"fneval xxstring fails, arg","",i);
	    for (k = 0; k <= i; k++)	/* Free up previous space on error */
	      if (bp[k]) free(bp[k]);
	    return("");			/* and return null string. */
	}
	debug(F111,"fneval arg",bp[i],i);
    }

#ifdef DEBUG
    if (deblog) {
	int j;
	for (j = 0; j < argn; j++) {
	    debug(F111,"fneval arg post eval",argp[j],j);
	    debug(F111,"fneval evaluated arg",bp[j],j);
	}
    }
#endif /* DEBUG */
/*
  At this point bp[0..argn-1] are not NULL and all must be freed
  before returning.
*/
    switch (y) {			/* Do function on expanded args */

      case FN_DEF:			/* \fdefinition(arg1) */
	if (!bp[0]) return("");
	k = mlook(mactab,bp[0],nmac);
	p = (k > -1) ? mactab[k].mval : "";
	for (k = 0; k < argn; k++) if (bp[k]) free(bp[k]);
	return(p ? p : "");

      case FN_EVA:			/* \fevaluate(arg1) */
	p = "";
	if (argn > 0)
	  p = *(bp[0]) ? evala(bp[0]) : "";
	for (k = 0; k < argn; k++) if (bp[k]) free(bp[k]);
	return(p ? p : "");

      case FN_EXE:			/* \fexecute(arg1) */
	if (argn < 1) return("");
	j = (int)strlen(s = bp[0]);	/* Length of macro invocation */
	p = "";				/* Initialize return value to null */
	if (j) {			/* If there is a macro to execute */
	    while (*s == SP) s++,j--;	/* strip leading spaces */
	    p = s;			/* remember beginning of macro name */
	    for (i = 0; i < j; i++) {	/* find end of macro name */
		if (*s == SP)
		  break;
		s++;
	    }
	    if (*s == SP) 	{	/* if there was a space after */
		*s++ = NUL;		/* terminate the macro name */
		while (*s == SP) s++;	/* skip past any extra spaces */
	    } else s = "";		/* maybe there are no arguments */
	    if (p && *p)
	      k = mlook(mactab,p,nmac);	/* Look up the macro name */
	    else k = -1;
/*
  This is just a WEE bit dangerous because we are copying up to 9 arguments
  into the space reserved for one.  It won't overrun the buffer, but if there
  are lots of long arguments we might lose some.  The other problem is that if
  the macro has more than 3 arguments, the 4th through last are all
  concatenated onto the third.  (The workaround is to use spaces rather than
  commas to separate them.)  Leaving it like this to avoid having to allocate
  tons more buffers.
*/
	    if (argn > 1) {		/* Commas used instead of spaces */
		int i;
		char *p = bp[0];	/* Reuse this space */
		*p = NUL;		/* Make into dodo() arg list */
		for (i = 1; i < argn; i++) {
		    strncat(p,bp[i],MAXARGLEN);
		    strncat(p," ",MAXARGLEN);
		}		    
		s = bp[0];		/* Point to new list */
	    }
	    p = "";			/* Initialize return value */
	    if (k >= 0) {		/* If macro found in table */
		/* Go set it up (like DO cmd) */
		if ((j = dodo(k,s,cmdstk[cmdlvl].ccflgs)) > 0) {
		    if (cmpush() > -1) { /* Push command parser state */
			extern int ifc;
			int ifcsav = ifc; /* Push IF condition on stack */
			k = parser(1);	/* Call parser to execute the macro */
			cmpop();	/* Pop command parser */
			ifc = ifcsav;	/* Restore IF condition */
			if (k == 0) {	/* No errors, ignore action cmds. */
			    p = mrval[maclvl+1]; /* If OK, set return value. */
			    if (p == NULL) p = "";
			}
		    } else {		/* Can't push any more */
			debug(F100,"fexec pushed too deep","",0);
                        printf("\n?\\fexec() too deeply nested\n");
			while (cmpop() > -1) ;
			p = "";
		    }
		}
	    }
	}
	for (k = 0; k < argn; k++) if (bp[k]) free(bp[k]);
	return(p ? p : "");

      case FN_FC:			/* \ffiles() - File count. */
	if (argn < 1) return("0");
	if (*(bp[0])) {
	    k = zxpand(bp[0]);
	    sprintf(fnval,"%d",k);
	    p = fnval;
	} else p = "0";
	for (k = 0; k < argn; k++) if (bp[k]) free(bp[k]);
	return(p);

      case FN_FIL:			/* \fnextfile() - Next file in list. */
	p = fnval;			/* (no args) */
	*p = NUL;
	znext(p);
	for (k = 0; k < argn; k++) if (bp[k]) free(bp[k]);
	return(p ? p : "");

      case FN_IND:			/* \findex(arg1,arg2,arg3) */
      case FN_RIX:			/* \frindex(arg1,arg2,arg3) */
	p = "0";
	if (argn > 1) {			/* Only works if we have 2 or 3 args */
	    int start;
	    len1 = (int)strlen(bp[0]);	/* length of string to look for */
	    len2 = (int)strlen(s = bp[1]); /* length of string to look in */
	    if (len1 < 1 || len2 < 1)	/* Watch out for empty strings */
	      goto indexfin;
	    j = len2 - len1;		/* length difference */
	    start = (y == FN_IND) ? 0 : j; /* Starting position */
	    if (argn > 2) {
		val1 = *(bp[2]) ? evala(bp[2]) : "";
		if (chknum(val1)) {
		    int t;
		    t = atoi(val1) - 1;
		    if (t < 0) t = 0;
		    start = (y == FN_IND) ? t : start - t - 1;
		    if (start < 0) start = 0;
		}
	    }
	    start = ckindex(bp[0],bp[1],start,(y==FN_IND)?0:1,inpcas[cmdlvl]);
	    sprintf(fnval,"%d",start);
	    p = fnval;
	}
      indexfin:
	for (k = 0; k < argn; k++) if (bp[k]) free(bp[k]);
	return(p);

      case FN_RPL:			/* \freplace(s1,s2,s3) */
      /*
	s = bp[0] = source string
	    bp[1] = match string
	    bp[2] = replacement string
	p = fnval = destination (result) string
      */
	if (argn < 1) return("");
	p = fnval;
	if (argn < 2) {			/* Only works if we have 2 or 3 args */
	    strcpy(p,bp[0]);
	} else  {			
	    len1 = (int)strlen(bp[0]);	/* length of string to look in */
	    len2 = (int)strlen(bp[1]);	/* length of string to look for */
	    len3 = (argn < 3) ? 0 : (int)strlen(bp[2]); /* Len of replacemnt */
	    j = len1 - len2 + 1;
	    if (j < 1 || len1 == 0 || len2 == 0) { /* Args out of whack */
		strcpy(p,bp[0]);	/* so just return original string */
	    } else {
		s = bp[0];		/* Point to beginning of string */
		while (j--) {		/* For each character */
		    if (inpcas[cmdlvl] ?
			!strncmp(bp[1],s,len2) :
			!xxstrcmp(bp[1],s,len2) ) { /* To be replaced? */
			if (len3) {		    /* Yes, */
			    strncpy(p,bp[2],len3);  /* replace it */
			    p += len3;
			}
			s += len2;	            /* and skip past it. */
		    } else {		/* No, */
			*p++ = *s++;	/* just copy this character */
		    }
		}
		*p = NUL;
		while (*p++ = *s++);
	    }
	}
	for (k = 0; k < argn; k++) if (bp[k]) free(bp[k]);
	return(p = fnval);

      case FN_CHR:			/* \fcharacter(arg1) */
	if (argn < 1) return("");
	val1 = *(bp[0]) ? evala(bp[0]) : "";
	if (chknum(val1)) {		/* Must be numeric */
	    i = atoi(val1);
	    if (i >= 0 && i < 256) {	/* Must be an 8-bit value */
		p = fnval;
		*p++ = (char) i;
		*p = NUL;
		p = fnval;
	    } else p = "";		/* Otherwise return empty string */
	} else p = "";			/* Otherwise return empty string */
	for (k = 0; k < argn; k++) if (bp[k]) free(bp[k]);
	return(p);

      case FN_COD:			/* \fcode(char) */
	if (argn < 1) return("");
	if ((int)strlen(bp[0]) > 0) {
	    p = fnval;
	    i = *bp[0];
	    sprintf(p,"%d",(i & 0xff));
	} else p = "";
	for (k = 0; k < argn; k++) if (bp[k]) free(bp[k]);
	return(p);

      case FN_LEN:			/* \flength(arg1) */
	if (argn > 0) {
	    p = fnval;
	    sprintf(p,"%d",(int)strlen(bp[0]));
	} else p = "0";
	for (k = 0; k < argn; k++) if (bp[k]) free(bp[k]);
	return(p);

      case FN_LOW:			/* \flower(arg1) */
	if (argn < 1) return("");
	s = bp[0];
	p = fnval;

	while (*s) {
	    if (isupper(*s))
	      *p = (char) tolower(*s);
	    else
	      *p = *s;
	    p++; s++;
	}
	*p = NUL;
	for (k = 0; k < argn; k++) if (bp[k]) free(bp[k]);
	p = fnval;
	return(p);

      case FN_MAX:			/* \fmax(arg1,arg2) */
      case FN_MIN:			/* \fmin(arg1,arg2) */
      case FN_MOD:			/* \fmod(arg1,arg2) */
	if (argn < 2) return("");
	val1 = *(bp[0]) ? evala(bp[0]) : "";
	free(bp[0]);			/* Copy this because evala() returns */
	bp[0] = malloc((int)strlen(val1)+1); /* pointer to same */
	strcpy(bp[0],val1);
	val1 = bp[0];			/* buffer next time. */
	val2 = *(bp[1]) ? evala(bp[1]) : "";
	if (chknum(val1) && chknum(val2)) {
	    i = atoi(val1);
	    j = atoi(val2);
	    switch (y) {
	      case FN_MAX:
		if (j < i) j = i;
		break;
	      case FN_MIN:
		if (j > i) j = i;
		break;
	      case FN_MOD:
		if (j == 0) { p = ""; goto modfin; }
		j = i % j;
		break;
	    }
	    p = fnval;
	    sprintf(p,"%d",j);
	} else p = "";
      modfin:
	for (k = 0; k < argn; k++) if (bp[k]) free(bp[k]);
	return(p);

      case FN_SUB:			/* \fsubstr(arg1,arg2,arg3) */
      case FN_RIG:			/* \fright(arg1,arg2) */
	if (argn < 1) return("");
	val1 = "";
	if (argn > 1)
	  if (*(bp[1]))
	    val1 =  evala(bp[1]);
	free(bp[1]);			/* Have to copy this */
	bp[1] = malloc((int)strlen(val1)+1);
	strcpy(bp[1],val1);
	val1 = bp[1];
	val2 = "";
	if (argn > 2)
	  if (*(bp[2]))
	    val2 = evala(bp[2]);
	if (
	    ((argn > 1) && (int)strlen(val1) && !rdigits(val1)) ||
	    ((y == FN_SUB) &&
	      ((argn > 2) && (int)strlen(val2) && !rdigits(val2)))
	    ) {
	    p = "";			/* if either, return null */
	} else {
	    int lx;
	    p = fnval;			/* pointer to result */
	    lx = strlen(bp[0]);		/* length of arg1 */
	    if (y == FN_SUB) {		/* substring */
		k = (argn > 2) ? atoi(val2) : MAXARGLEN; /* length */
		j = (argn > 1) ? atoi(val1) : 1; /* start pos for substr */
	    } else {				 /* right */
		k = (argn > 1) ? atoi(val1) : lx; /* length */
		j = lx - k + 1;			 /* start pos for right */
		if (j < 1) j = 1;
	    }
	    if (k > 0 && j <= lx) { 		 /* if start pos in range */
		s = bp[0]+j-1;    		 /* point to source string */
		for (i = 0; (i < k) && (*p++ = *s++); i++) ;  /* copy */
	    }
	    *p = NUL;			/* terminate the result */
	    p = fnval;			/* and point to it. */
	}
	for (k = 0; k < argn; k++) if (bp[k]) free(bp[k]); /* Free temp mem */
	return(p);

      case FN_UPP:			/* \fupper(arg1) */
	s = bp[0] ? bp[0] : "";
	p = fnval;
	while (*s) {
	    if (islower(*s))
	      *p = (char) toupper(*s);
	    else
	      *p = *s;
	    p++; s++;
	}
	*p = NUL;
	for (k = 0; k < argn; k++) if (bp[k]) free(bp[k]);
	p = fnval;
	return(p);

      case FN_REP:			/* \frepeat(text,number) */
	if (argn < 1) return("");
	p = fnval;			/* Return value */
	*p = NUL;
	val1 = "1";
	if (argn > 1)
	  if (*(bp[1]))
	    val1 = evala(bp[1]);
	if (chknum(val1)) {		/* Repeat count */
	    n = atoi(val1);
	    if (n > 0) {		/* Make n copies */
		p = fnval;
		*p = '\0';
		k = (int)strlen(bp[0]);	/* Make sure string has some length */
		if (k > 0) {
		    for (i = 0; i < n; i++) {
			s = bp[0];
			for (j = 0; j < k; j++) {
			    if ((p - fnval) >= FNVALL) { /* Protect against */
				p = "";	             /* core dumps... */
				break;
			    } else *p++ = *s++;
			}
		    }
		    *p = NUL;
		}
	    }
	}
	for (k = 0; k < argn; k++) if (bp[k]) free(bp[k]);
	p = fnval;
	return(p);

#ifndef NOFRILLS
      case FN_REV:			/* \freverse() */
	if (argn < 1) return("");
	p = fnval;
	yystring(bp[0],&p);
	for (k = 0; k < argn; k++) if (bp[k]) free(bp[k]);
	return(p);
#endif /* NOFRILLS */

      case FN_RPA:			/* \frpad() and \flpad() */
      case FN_LPA:
	if (argn < 1) return("");
	*fnval = NUL;			/* Return value */
	val1 = "";
	if (argn > 1)
	  if (*(bp[1]))
	    val1 = evala(bp[1]);
	if (argn == 1 || !*val1) {	/* If a number wasn't given */
	    p = fnval;			/* just return the original string */
	    strncpy(p,bp[0],FNVALL);
	} else if (chknum(val1)) {	/* Repeat count */
	    char pc;
	    n = atoi(val1);
	    if (n >= 0) {
		p = fnval;
		k = (int)strlen(bp[0]);	/* Length of string to be padded */
		if (k >= n) {		/* It's already long enough */
		    strncpy(p,bp[0],FNVALL);
		} else {
		    if (n + k <= FNVALL) {
			pc = (char) ((argn < 3) ? SP : *bp[2]);
			if (!pc) pc = SP;
			if (y == FN_RPA) { /* RPAD */
			    strncpy(p,bp[0],k);
			    p += k;
			    for (i = k; i < n; i++)
			      *p++ = pc;
			} else {	/* LPAD */
			    n -= k;
			    for (i = 0; i < n; i++)
			      *p++ = pc;
			    strncpy(p,bp[0],k);
			    p += k;
			}
		    }
		    *p = NUL;
		}
	    }
	}
	for (k = 0; k < argn; k++) if (bp[k]) free(bp[k]);
	p = fnval;
	return(p);

#ifdef ZFCDAT
      case FN_FD:			/* \fdate(filename) */
	p = fnval;
	*p = NUL;
	if (argn > 0)
	  sprintf(fnval,"%s",*(bp[0]) ? zfcdat(bp[0]) : "");
	for (k = 0; k < argn; k++) if (bp[k]) free(bp[k]);
	return(p);
#endif /* ZFCDAT */

      case FN_FS:			/* \fsize(filename) */
	p = fnval;
	*p = NUL;
	if (argn > 0)
	  sprintf(fnval,"%ld",*(bp[0]) ? zchki(bp[0]) : 0L);
	for (k = 0; k < argn; k++) if (bp[k]) free(bp[k]);
	return(p);

      case FN_VER:			/* \fverify() */
	p = "0";
	if (argn > 1) {			/* Only works if we have 2 or 3 args */
	    int start;
	    char *s2, ch1, ch2;
	    start = 0;
	    if (argn > 2) {		/* Starting position specified */
		val1 = *(bp[2]) ? evala(bp[2]) : "";
		if (chknum(val1)) {
		    start = atoi(val1) /* - 1 */;
		    if (start < 0) start = 0;
		    if (start > (int)strlen(bp[1]))
		      goto verfin;
		}
	    }
	    i = start;
	    p = "0";
	    for (s = bp[1] + start; *s; s++,i++) {
		ch1 = *s;
		if (!inpcas[cmdlvl]) if (islower(ch1)) ch1 = toupper(ch1);
		j = 0;
		for (s2 = bp[0]; *s2; s2++) {
		    ch2 = *s2;
		    if (!inpcas[cmdlvl]) if (islower(ch2)) ch2 = toupper(ch2);
		    if (ch1 == ch2) {
			j = 1;
			break;
		    }
		}
		if (j == 0) {
		    sprintf(fnval,"%d",i+1);
		    p = fnval;
		    break;
		}
	    }
	}
      verfin:
	for (k = 0; k < argn; k++) if (bp[k]) free(bp[k]);
	return(p);

      case FN_IPA:			/* Find and return IP address */
	if (argn > 0) {			/* in argument string. */
	    int start;
	    char *s2;
	    start = 0;
	    if (argn > 1) {		/* Starting position specified */
		if (chknum(bp[1])) {
		    start = atoi(bp[1]) - 1;
		    if (start < 0) start = 0;
		}
	    }
	    p = getip(bp[0]+start);
	} else p = "";
	for (k = 0; k < argn; k++) if (bp[k]) free(bp[k]);
	return(p);

#ifdef OS2
      case FN_CRY:
	p = "";
	if (argn > 0) {
	    p = fnval;
	    strcpy(p,bp[0]);
            ck_encrypt(p);
	}
	for (k = 0; k < argn; k++) if (bp[k]) free(bp[k]);
	return(p);

      case FN_OOX:
	p = "";
	if (argn > 0)
	  p = (char *) ck_oox(bp[0], (argn > 1) ? bp[1] : "");
	for (k = 0; k < argn; k++) if (bp[k]) free(bp[k]);
	return(p);
#endif /* OS2 */

      case FN_HEX:			/* \fhexify(arg1) */
	if (argn < 1) return("");
	p = "";
	if ((int)strlen(bp[0]) < (FNVALL / 2)) {
	    s = bp[0];
	    p = fnval;
	    while (*s) {
		x = (*s >> 4) & 0x0f;
		*p++ = hexdigits[x];
		x = *s++ & 0x0f;
		*p++ = hexdigits[x];	    
	    }
	    *p = NUL;
	    p = fnval;
	}
	for (k = 0; k < argn; k++) if (bp[k]) free(bp[k]);
	return(p);

      case FN_UNH: {			/* \funhex(arg1) */
	  int c[2], i;
	  if (argn < 1) return("");
	  p = "";
	  if ((int)strlen(bp[0]) < (FNVALL * 2)) {
	      s = bp[0];
	      p = fnval;
	      while (*s) {
		  for (i = 0; i < 2; i++) {
		      c[i] = *s++;
		      if (!c[i]) { p = ""; goto unhexfin; }
		      if (islower(c[i])) c[i] = toupper(c[i]);
		      if (c[i] >= '0' && c[i] <= '9')
			c[i] -= 0x30;
		      else if (c[i] >= 'A' && c[i] <= 'F')
			c[i] -= 0x37;
		      else { p = ""; goto unhexfin; }
		  }
		  *p++ = ((c[0] << 4) & 0xf0) | (c[1] & 0x0f);
	      }
	      *p = NUL;
	      p = fnval;
	  }
  unhexfin:
	  for (k = 0; k < argn; k++) if (bp[k]) free(bp[k]);
	  return(p);
      }

      case FN_BRK: {			/* \fbreak() */
	  char * c;			/* Characters to break on */
	  char c2, s2;
	  int start = 0;
	  int done = 0;
	  if (argn < 1) return("");
	  p = fnval;			/* Destination pointer */
	  *p = NUL;
	  if (argn > 2) {
	      s = bp[2] ? bp[2] : "";
	      if (chknum(s)) {
		  start = atoi(s);
		  if (start < 0) start = 0;
		  if (start > (int)strlen(bp[0]))
		    goto brkfin;
	      }
	  }
	  s = bp[0] + start;		/* Source pointer */

	  while (*s && !done) {
	      s2 = *s;
	      if (!inpcas[cmdlvl] && islower(s2)) s2 = toupper(s2);
	      c = bp[1] ? bp[1] : "";	/* Character to break on */
	      while (*c) {
		  c2 = *c;
		  if (!inpcas[cmdlvl] && islower(c2)) c2 = toupper(c2);
		  if (c2 == s2) {
		      done = 1;
		      break;
		  }
		  c++;
	      }
	      if (done) break;
	      *p++ = *s++;
	  }
	  *p = NUL;			/* terminate the result */
	  p = fnval;			/* and point to it. */
	brkfin:
	  for (k = 0; k < argn; k++) if (bp[k]) free(bp[k]);
	  return(p);
      }

      case FN_SPN: {			/* \fspan() */
	  char *q;
	  char c1, c2;
	  int start = 0;
	  if (argn < 1) return("");
	  p = fnval;			/* Result pointer */
	  *p = NUL;
	  if (argn > 2) {		/* Starting position */
	      s = bp[2] ? bp[2] : "";
	      if (chknum(s)) {
		  start = atoi(s);
		  if (start < 0) start = 0;
	      }
	  }
	  s = bp[0] + start;		/* Source pointer */
	  if (argn > 1 &&
	      (int)strlen(bp[1]) > 0 &&
	      start <= (int)strlen(bp[0])) {
	      while (*s) {		/* Loop thru source string */
		  q = bp[1];		/* Span string */
		  c1 = *s;
		  if (!inpcas[cmdlvl])
		    if (islower(c1)) c1 = toupper(c1);
		  x = 0;
		  while (c2 = *q++) {
		      if (!inpcas[cmdlvl])
			if (islower(c2)) c2 = toupper(c2);
		      if (c1 == c2) { x = 1; break; }
		  }
		  if (!x) break;
		  *p++ = *s++;
	      }
	      *p = NUL;			/* Terminate and return the result */
	      p = fnval;
	  }
	  for (k = 0; k < argn; k++) if (bp[k]) free(bp[k]);
	  return(p);
      }

      case FN_TRM:			/* \ftrim(s1[,s2]) */
      case FN_LTR:			/* \fltrim(s1[,s2]) */
	if (argn < 1) return("");
	if ((len1 = (int)strlen(bp[0])) > 0) {
	    if (len1 > FNVALL)
	      len1 = FNVALL;
	    s = " \t";
	    if (argn > 1)		/* Trim list given */
	      s = bp[1];
	    len2 = (int)strlen(s);
	    if (len2 < 1) {		/* or not... */
		s = " \t";		/* Default is to trim whitespace */
		len2 = 2;
	    }
	    if (y == FN_TRM) {		/* Trim from right */
		char * q, p2, q2;
		strncpy(fnval,bp[0],FNVALL); /* Copy string to output */
		p = fnval + len1 - 1;	/* Point to last character */

		while (p >= (char *)fnval) { /* Go backwards */
		    q = s;		/* Point to trim list */
		    p2 = *p;
		    if (!inpcas[cmdlvl])
		      if (islower(p2)) p2 = toupper(p2);
		    while (*q) {	/* Is this char in trim list? */
			q2 = *q;
			if (!inpcas[cmdlvl])
			  if (islower(q2)) q2 = toupper(q2);
			if (p2 == q2) {	/* Yes, null it out */
			    *p = NUL;
			    break;
			}
			q++;
		    }
		    if (!*q)		/* Trim list exhausted */
		      break;		/* So we're done. */
		    p--;		/* Else keep trimming */
		}
	    } else {			/* Trim from left */
		char * q, p2, q2;
		p = bp[0];		/* Source */
		while (*p) {
		    p2 = *p;
		    if (!inpcas[cmdlvl])
		      if (islower(p2)) p2 = toupper(p2);
		    q = s;
		    while (*q) {	/* Is this char in trim list? */
			q2 = *q;
			if (!inpcas[cmdlvl])
			  if (islower(q2)) q2 = toupper(q2);
			if (p2 == q2) {	/* Yes, point past it */
			    p++;	/* and try next source character */
			    break;
			}
			q++;		/* No, try next trim character */
		    }
		    if (!*q)		/* Trim list exhausted */
		      break;		/* So we're done. */
		}
		strncpy(fnval,p,FNVALL);
	    }
	    p = fnval;
	} else p = "";
	for (k = 0; k < argn; k++) if (bp[k]) free(bp[k]);
	return(p);

      case FN_CAP:			/* \fcapitalize(arg1) */
	if (argn < 1) return("");
	s = bp[0];
	p = fnval;
	x = 0;
	while (c = *s++) {
	    if (isalpha(c)) {
		if (x == 0) {
		    x = 1;
		    if (islower(c))
		      c = toupper(c);
		} else if (isupper(c))
		  c = tolower(c);
	    }
	    *p++ = c;
	}
	*p = NUL;
	for (k = 0; k < argn; k++) if (bp[k]) free(bp[k]);
	p = fnval;
	return(p);

      case FN_TOD:			/* Time of day to secs since midnite */
	if (argn < 1) return("");
	p = fnval;
	sprintf(p,"%ld",tod2sec(bp[0]));
	for (k = 0; k < argn; k++) if (bp[k]) free(bp[k]);
	return(p);

      case FN_FFN:			/* Full pathname of file */
	p = fnval;
	*p = NUL;
	if (argn > 0)
#ifdef ZFNQFP
	  zfnqfp(bp[0],FNVALL,p);
#else
	  strcpy(p,bp[0]);
#endif /* ZFNQFP */
	for (k = 0; k < argn; k++) if (bp[k]) free(bp[k]);
	return(p ? p : "");

      case FN_CHK: {			/* \fchecksum() */
	  long chk = 0;
	  p = (argn > 0) ? bp[0] : "";
	  while (*p) chk += *p++;
	  sprintf(fnval,"%lu",chk);
	  for (k = 0; k < argn; k++) if (bp[k]) free(bp[k]);
	  return((char *)fnval);
      }
      case FN_CRC:			/* \fcrc16() */
	*fnval = NUL;
	if (argn > 0)
	  sprintf(fnval,"%u",chk3((CHAR *)bp[0],0));
	for (k = 0; k < argn; k++) if (bp[k]) free(bp[k]);
	return((char *)fnval);

      case FN_BSN:			/* \fbasename() */
	if (argn > 0)
	  zstrip(bp[0],&p);
	for (k = 0; k < argn; k++) if (bp[k]) free(bp[k]);
	return(p);

      default:
	return("");
    }
}

