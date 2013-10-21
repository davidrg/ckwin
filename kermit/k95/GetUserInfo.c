/* By Peter 'Luna' Runestig <peter@runestig.com> 1996, 99
 */
#include <windows.h>
#include <lmaccess.h>
#include <lmapibuf.h>
#include <stdio.h>

int GetUserInfo(LPWSTR wCompName, LPWSTR wUserName);
int UnicodeToOEM(LPWSTR InString, char *OutString, int OutStringSize);
int OEMToUnicode(char *InString, LPWSTR OutString, int OutStringSize);

char HomeDir[100] = "", ProfilePath[100] = "";

int main(int argc, char *argv[], char *envp[])
{
	unsigned short wCompName[128], wDomainName[128], wUserName[128], *wPDCName;
	char *CompName = NULL;
	char *DomainName = "BOLL.KL.SE";
//	char *CompName = "\\\\BOLLNT01";
//	char *DomainName = NULL;
	char *UserName = "LunaUser";

	OEMToUnicode(UserName, wUserName, sizeof(wUserName));

	if (DomainName) {
		OEMToUnicode(DomainName, wDomainName, sizeof(wDomainName));
		if (NetGetDCName(NULL, wDomainName, (LPBYTE *)&wPDCName)) {
			printf("Cannot find PDC!\n");
			return 2;
		} 
		GetUserInfo(wPDCName, wUserName);
		NetApiBufferFree(wPDCName);
	}
	else if (CompName) {
		OEMToUnicode(CompName, wCompName, sizeof(wCompName));
		GetUserInfo(wCompName, wUserName);
	}

	if (*HomeDir && *ProfilePath)
		printf("User:\t\t%s\nHome Dir:\t%s\nProfile Path:\t%s\n", UserName, HomeDir, ProfilePath);
	return 0;
}



int GetUserInfo(LPWSTR wCompName, LPWSTR wUserName)
{
  int err;
  LPUSER_INFO_3 UserInfo;
/*typedef struct _USER_INFO_3 {
    LPWSTR    usri3_name;
    LPWSTR    usri3_password;
    DWORD     usri3_password_age;
    DWORD     usri3_priv;
    LPWSTR    usri3_home_dir;
    LPWSTR    usri3_comment;
    DWORD     usri3_flags;
    LPWSTR    usri3_script_path;
    DWORD     usri3_auth_flags;
    LPWSTR    usri3_full_name;
    LPWSTR    usri3_usr_comment;
    LPWSTR    usri3_parms;
    LPWSTR    usri3_workstations;
    DWORD     usri3_last_logon;
    DWORD     usri3_last_logoff;
    DWORD     usri3_acct_expires;
    DWORD     usri3_max_storage;
    DWORD     usri3_units_per_week;
    PBYTE     usri3_logon_hours;
    DWORD     usri3_bad_pw_count;
    DWORD     usri3_num_logons;
    LPWSTR    usri3_logon_server;
    DWORD     usri3_country_code;
    DWORD     usri3_code_page;
    DWORD     usri3_user_id;
    DWORD     usri3_primary_group_id;
    LPWSTR    usri3_profile;
    LPWSTR    usri3_home_dir_drive;
    DWORD     usri3_password_expired;
}USER_INFO_3, *PUSER_INFO_3, *LPUSER_INFO_3;*/
  
	err = NetUserGetInfo(wCompName, wUserName, 3, (LPBYTE *)&UserInfo);
	if (!err) {
		UnicodeToOEM(UserInfo->usri3_home_dir, HomeDir, sizeof(HomeDir));
		UnicodeToOEM(UserInfo->usri3_profile, ProfilePath, sizeof(ProfilePath));
		NetApiBufferFree(UserInfo);
	}
	return err;
}

int UnicodeToOEM(LPWSTR InString, char *OutString, int OutStringSize)
{
  return WideCharToMultiByte(CP_OEMCP, 0, InString, -1, OutString, OutStringSize, NULL, NULL);
}

int OEMToUnicode(char *InString, LPWSTR OutString, int OutStringSize)
{
  return MultiByteToWideChar(CP_OEMCP, 0, InString, -1, OutString, OutStringSize);
}
