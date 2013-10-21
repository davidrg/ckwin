#include <stdio.h>
#include <windows.h>
#include <shellapi.h>
int __cdecl
main(int argc, char **argv) {
   if (argc == 2) {
      ShellExecute(0, 0, argv[1], 0, 0, SW_NORMAL);
   } else {
      fputs("Please call with a single argument (a URL)\n", stderr);
      exit(2);
   }
   return 0;
}

