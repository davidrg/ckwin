LOCAL_PATH:= $(call my-dir)

# ========================================================
# nano
# ========================================================
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	ckcmai.c ckclib.c ckutio.c ckufio.c \
	ckcfns.c ckcfn2.c ckcfn3.c ckuxla.c \
	ckcpro.c ckucmd.c ckuus2.c ckuus3.c \
	ckuus4.c ckuus5.c ckuus6.c ckuus7.c \
	ckuusx.c ckuusy.c ckuusr.c ckucns.c \
	ckudia.c ckuscr.c ckcnet.c ckusig.c \
	ckctel.c ckcuni.c ckupty.c ckcftp.c \
	ckuath.c ck_crp.c ck_ssl.c
LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)
LOCAL_CFLAGS += \
	-DFNFLOAT -DCK_NEWTERM -DTCPSOCKET \
	-DLINUXFSSTND -DNOCOTFMC -DUSE_STRERROR \
	-DHAVE_PTMX -D_LARGEFILE_SOURCE -DNO_OPENPTY \
	-D_FILE_OFFSET_BITS=64 -DPOSIX -DUSE_FILE_R\
	-DKTARGET=\"android\" -DNO_DNS_SRV -DNOIKSD \
	-DNOTIMESTAMP -DNOZLOCALTIME -DNOUUCP \
	-DNO_NL_LANGINFO -DNO_LOCALE
LOCAL_MODULE := kermit
LOCAL_MODULE_PATH := $(TARGET_OUT_OPTIONAL_EXECUTABLES)
LOCAL_MODULE_TAGS := eng
include $(BUILD_EXECUTABLE)

