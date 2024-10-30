/*****************************************************************************/
/*             Copyright (c) 1994 by Jyrki Salmi <jytasa@jyu.fi>             */
/*        You may modify, recompile and distribute this file freely.         */
/*****************************************************************************/

#ifdef NT
#define CKDEVDLLENTRY
#else /* NT */
#ifdef OS2
#define CKDEVDLLENTRY	_System
#else /* OS2 */
#define CKDEVDLLENTRY
#endif /* OS2 */
#endif /* NT */

typedef U32 CKDEVDLLENTRY p_transfer_func(P_CFG *);
#ifdef XYZ_DLL
#ifdef XYZ_DLL_CLIENT
extern p_transfer_func * p_transfer;
#else /* XYZ_DLL_CLIENT */
extern p_transfer_func p_transfer;
#endif /* XYZ_DLL_CLIENT */
#else /* XYZ_DLL */
extern p_transfer_func p_transfer;
#endif /* XYZ_DLL */
extern int load_p_dll(void);
extern int unload_p_dll(void);
extern int p(int sstate);
