/*****************************************************************************/
/*             Copyright (c) 1994 by Jyrki Salmi <jytasa@jyu.fi>             */
/*        You may modify, recompile and distribute this file freely.         */
/*****************************************************************************/

#ifdef XYZ_DLL

#ifdef NT
#define CKDEV_MODULE_NAME   "P95"
#define CKDEVDLLENTRY
#define CKDEV_ENTRY_NAME    "p_transfer"
#else
#define CKDEV_MODULE_NAME   "P2"
#define CKDEVDLLENTRY       _System
#define CKDEV_ENTRY_NAME    "p_transfer"
#endif

typedef U32 CKDEVDLLENTRY p_transfer_t(P_CFG *);

#ifdef XYZ_DLL_CLIENT

extern p_transfer_t *p_transfer;

#else /* XYZ_DLL */

extern p_transfer_t p_transfer;

#endif

extern int load_p_dll(void);
extern int unload_p_dll(void);

#else /* XYZ_DLL */

extern p_transfer_t p_transfer;

#endif /* XYZ_DLL */

extern int p(int sstate);
