/*****************************************************************************/
/*             Copyright (c) 1994 by Jyrki Salmi <jytasa@jyu.fi>             */
/*        You may modify, recompile and distribute this file freely.         */
/*****************************************************************************/

#ifdef XYZ_DLL
extern U32 (* _System p_transfer)(P_CFG *);
#else /* XYZ_DLL */
U32 _System p_transfer(P_CFG *);
#endif /* XYZ_DLL */
extern int load_p_dll(void);
extern int unload_p_dll(void);
extern int p(int sstate);
