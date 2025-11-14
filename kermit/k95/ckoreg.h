#ifndef CKOREG_H
#define CKOREG_H

char * GetAppData( int common );
char * GetHomeDrive(void);
char * GetHomePath(void);
char * GetPersonal(void);
char * GetDesktop(void);

#ifdef CK_LOGIN
int IsSSPLogonAvail( void );
#endif /* CK_LOGIN */

const char * SSPLogonDomain( void );
char * GetEditorCommand( void );
char * GetBrowserCommand( void );
char * GetFtpCommand( void );

#ifdef BROWSER
#ifdef NT
void
Real_Win32ShellExecute( void* param );
int
Win32ShellExecute( char * object );
#endif /* NT */
#endif /* BROWSER */

void os2InitFromRegistry( void );

#endif /* CKOREG_H */
