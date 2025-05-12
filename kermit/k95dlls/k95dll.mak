# Visual C++ 2.x, 4.x, and 5.x makefile                               
                                                                      
# Does not depend on the presence of any environment variables in     
# order to compile tcl; all needed information is derived from        
# location of the compiler directories.                               
                                                                      
# Project directories                                                 
#                                                                     
# TOOLS32 = location of VC++ 32-bit development tools. Note that the  
#	    VC++ 2.0 header files are broken, so you need to use the  
#	    ones that come with the developer network CD's, or later  
#	    versions of VC++.                                         
#                                                                     
                                                                      
!IFNDEF TOOLS32                                                       
TOOLS32		= f:\visual_studio\vc98\
!ENDIF                                                                
                                                                      
# Set this to the appropriate value of /MACHINE: for your platform    
MACHINE	= IX86                                                        
                                                                      
# Comment the following line to compile with symbols                  
NODEBUG=1                                                             
                                                                      
######################################################################
# Do not modify below this line                                       
######################################################################
                                                                      
ZLIB_LIB	= k95dll.lib
ZLIB_DLL	= k95dll.dll                                            
                                                                      
ZLIB_OBJS       = k95dll.obj      
                                                                      
cc32		= $(TOOLS32)\bin\cl.exe                               
link32		= $(TOOLS32)\bin\link.exe                             
rc32		= $(TOOLS32)\bin\rc.exe                               
nmake		= $(TOOLS32)\bin\nmake.exe                            
include32	= -I$(TOOLS32)\include                                
                                                                      
DEFINES	= -D__WIN32__ -DWIN32 $(DEBUGDEFINES)                         
                                                                      
CFLAGS	= $(cdebug) $(cflags) $(cvarsdll) $(include32) \
			$(INCLUDES) $(DEFINES)                        
CON_CFLAGS	= $(cdebug) $(cflags) $(cvars) $(include32) -DCONSOLE 
DOS_CFLAGS	= $(cdebug) $(cflags) $(include16) -AL                
DLL16_CFLAGS	= $(cdebug) $(cflags) $(include16) -ALw               
                                                                      
######################################################################
# Link flags                                                          
######################################################################
                                                                      
!IFDEF NODEBUG                                                        
ldebug = /RELEASE                                                     
!ELSE                                                                 
ldebug = -debug:full -debugtype:cv                                    
!ENDIF                                                                
                                                                      
# declarations common to all linker options                           
lcommon = /NODEFAULTLIB /RELEASE /NOLOGO                              
                                                                      
# declarations for use on Intel i386, i486, and Pentium systems       
!IF "$(MACHINE)" == "IX86"                                            
DLLENTRY = @12                                                        
lflags   = $(lcommon) -align:0x1000 /MACHINE:$(MACHINE)               
!ELSE                                                                 
lflags   = $(lcommon) /MACHINE:$(MACHINE)                             
!ENDIF                                                                
                                                                      
conlflags = $(lflags) -subsystem:console -entry:mainCRTStartup        
guilflags = $(lflags) -subsystem:windows -entry:WinMainCRTStartup     
dlllflags = $(lflags) -entry:_DllMainCRTStartup$(DLLENTRY) -dll       
                                                                      
!IF "$(MACHINE)" == "PPC"                                             
libc = libc.lib                                                       
libcdll = crtdll.lib                                                  
!ELSE                                                                 
libc = libc.lib oldnames.lib                                          
libcdll = msvcrt.lib oldnames.lib                                     
!ENDIF                                                                
                                                                      
baselibs   = kernel32.lib $(optlibs) advapi32.lib                     
winlibs    = $(baselibs) user32.lib gdi32.lib comdlg32.lib
                                                                      
guilibs	   = $(libc) $(winlibs)                                       
conlibs	   = $(libc) $(baselibs)                                      
guilibsdll = $(libcdll) $(winlibs)                                    
conlibsdll = $(libcdll) $(baselibs)                                   
                                                                      
######################################################################
# Compile flags                                                       
######################################################################
                                                                      
!IFDEF NODEBUG                                                        
cdebug = -Ox                                                          
!ELSE                                                                 
cdebug = -Z7 -Od                                                      
!ENDIF                                                                
                                                                      
# declarations common to all compiler options                         
ccommon = -c -W3 -nologo -YX -Dtry=__try -Dexcept=__except            
                                                                      
!IF "$(MACHINE)" == "IX86"                                            
cflags = $(ccommon) -D_X86_=1 -J
!ELSE                                                                 
!IF "$(MACHINE)" == "MIPS"                                            
cflags = $(ccommon) -D_MIPS_=1 -J
!ELSE                                                                 
!IF "$(MACHINE)" == "PPC"                                             
cflags = $(ccommon) -D_PPC_=1 -J
!ELSE                                                                 
!IF "$(MACHINE)" == "ALPHA"                                           
cflags = $(ccommon) -D_ALPHA_=1 -J
!ENDIF                                                                
!ENDIF                                                                
!ENDIF                                                                
!ENDIF                                                                
                                                                      
cvars      = -DWIN32 -D_WIN32                                         
cvarsmt    = $(cvars) -D_MT                                           
cvarsdll   = $(cvarsmt) -D_DLL                                        
                                                                      
######################################################################
# Project specific targets                                            
######################################################################
                                                                      
release:    $(ZLIB_DLL) $(ZLIB_LIB)                                   
all:	    $(ZLIB_DLL) $(ZLIB_LIB)                                   
                                                                      
$(ZLIB_DLL): $(ZLIB_OBJS) k95dll.def
	set LIB=$(TOOLS32)\lib                                        
	$(link32) $(ldebug) $(dlllflags) -def:k95dll.def \
		-out:$@ $(guilibsdll) @<<                             
$(ZLIB_OBJS)                                                          
<<                                                                    
                                                                      
$(ZLIB_LIB): $(ZLIB_DLL)                                              
                                                                      
#                                                                     
# Implicit rules                                                      
#                                                                     
                                                                      
.c.obj:                                                               
    $(cc32) $(CFLAGS) $<                                              
                                                                      
clean:                                                                
	-@del *~                                                      
	-@del *.ncb                                                   
        -@del *.exp                                                   
	-@del *.lib                                                   
	-@del *.dll                                                   
	-@del *.exe                                                   
        -@del *.obj                                                   
        -@del *.res                                                   
	-@del *.pch                                                   
	-@del *.plg                                                   
