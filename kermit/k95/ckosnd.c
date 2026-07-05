#include "ckcdeb.h"
#include "ckosnd.h"

#ifdef NT
#include <windows.h>
#ifdef CK_HAVE_DSOUND
#include <DSound.h>
#endif /* CK_HAVE_DSOUND */
#else
#define INCL_DOSPROCESS
#include <os2.h>
#undef COMMENT                /* COMMENT is defined in os2.h */
#endif

#ifndef __cplusplus
typedef int bool;
#else
#ifndef __BOOL_DEFINED
#ifndef __WATCOMC__
#ifndef __GNUC__
typedef int bool;
#endif
#endif
#endif
#endif

#ifdef NT
extern HWND hwndConsole ;
#endif /* NT */

/* VT level 5 Sound Support
 * ------------------------
 * While this *probably* could be supported on OS/2, for now its Windows only.
 *
 */

/* Available sound output methods */
#define SOUND_OUT_UNINITIALISED     0   /* Unknown */
#define SOUND_OUT_NONE              1   /* Just silence */
#define SOUND_OUT_MIDI              2   /* Windows only, sounds OK */
#define SOUND_OUT_BEEP              3   /* Everywhere, but not every good */
#define SOUND_OUT_DIRECT            4   /* Most Windows, sounds best */
/* TODO: XAudio2 for Windows 10/11 ? */

/* The sound output device to be used.
 * Start by trying sound output methods in order until we find one that works */
static int soundOutput = SOUND_OUT_UNINITIALISED;

/* Turn a MIDI note into a frequency */
static int
noteToFrequency(UCHAR note) {
    int freq;
    switch (note) {
        /* Turn the MIDI Note into a frequency. Frequencies in comments are
         * those given by the VT520 manual - unfortunately it doesn't give them
         * for every note, so they're not currently used. */
        case 72: /* C5 */   freq = 523; break;
        case 73: /* C#5 */  freq = 554; break;
        case 74: /* D5 */   freq = 587; break;
        case 75: /* D#5 */  freq = 622; break;  /* 632 Hz */
        case 76: /* E5 */   freq = 659; break;
        case 77: /* F5 */   freq = 698; break;
        case 78: /* F#5 */  freq = 740; break;
        case 79: /* G5 */   freq = 784; break;
        case 80: /* G#5 */  freq = 830; break; /* 847 Hz */
        case 81: /* A5 */   freq = 880; break;
        case 82: /* A#5 */  freq = 932; break; /* 944 Hz */
        case 83: /* B5 */   freq = 988; break;
        case 84: /* C6 */   freq = 1047; break;
        case 85: /* C#6 */  freq = 1109; break; /* 1047 Hz*/
        case 86: /* D6 */   freq = 1175; break;
        case 87: /* D#6 */  freq = 1245; break;
        case 88: /* E6 */   freq = 1319; break;
        case 89: /* F6 */   freq = 1397; break;
        case 90: /* F#6 */  freq = 1479; break;
        case 91: /* G6 */   freq = 1568; break;
        case 92: /* G#6 */  freq = 1661; break;
        case 93: /* A6 */   freq = 1760; break;
        case 94: /* A#6 */  freq = 1865; break;
        case 95: /* B6 */   freq = 1976; break;
        case 96: /* C7 */   freq = 2093; break;
        default: /* Unsupported note - just sleep */
            return 0;
    }
    return freq;
}


/****************** Null Output ******************/
/* Makes no sound, but it takes a while doing it. If nothing else works, we'll
 * at least block for the required timeframe. */
static void
MakeNoSound(int duration_ms) {
    msleep(duration_ms);
}

/****************** Beep Output ******************/
/* Plays a single note poorly via the system beeper. Starting with Windows XP
 * 64bit and Windows Vista this plays through the sound device. The result isn't
 * very good, but its perhaps better than silence.
 *
 * The beeper has no volume control.
 */
static void
MakeBeepSound(UCHAR note, int duration_ms) {
    int freq = noteToFrequency(note);
    if (freq == 0) {
        msleep(duration_ms);
        return;
    }

#ifdef NT
    if (Beep(freq, duration_ms) == 0) {
        debug(F100, "Beeper failed! Turning off sound out", 0, 0);
        soundOutput = SOUND_OUT_NONE;
    }
#else
    if (DosBeep(freq, duration_ms)) {
        debug(F100, "Beeper failed! Turning off sound out", 0, 0);
        soundOutput = SOUND_OUT_NONE;
    }
#endif
}


#ifdef NT

/****************** Windows MM Midi Output ******************/
/* This should work pretty much everywhere */
static HMIDIOUT midiDeviceHandle = NULL;

static bool
OpenMidiDevice() {
    debug(F100, "Attempting to open MIDI device...", 0, 0);
    midiOutOpen(
        &midiDeviceHandle,      /* [out] handle */
        MIDI_MAPPER,            /* Device to open */
        (DWORD_PTR)NULL,        /* progress callback - not needed */
        (DWORD_PTR)NULL,        /* User data - not needed */
        CALLBACK_NULL);         /* No callback */
    if (midiDeviceHandle != NULL) {
        /* Success! We have a midi device, now set it up. This doc gives the
         * MIDI commands we'll need:
         * https://learn.microsoft.com/en-us/windows/win32/multimedia/summary-of-maps-and-midi-messages
         * 0xC0 is Program Change, 0x80 is note off, 0x90 is note on.
         *
         * And 80 and 81 are square and sawtooth instruments respectively
         * https://learn.microsoft.com/en-us/windows/win32/multimedia/standard-midi-patch-assignments
         * I'm not really sure which is a closer match for the VT520.
         */
        midiOutShortMsg(midiDeviceHandle, MAKELONG(MAKEWORD(0xC0, 81), 0));
        soundOutput = SOUND_OUT_MIDI;
        debug(F100, "MIDI Device Open!", 0, 0);
        return TRUE;
    }

    debug(F100, "Failed to open MIDI device", 0, 0);
    return FALSE;
}

/* Plays a single note via MIDI with the specified velocity (volume) for the
 * specified duration in milliseconds. This function blocks.
 *
 * Its not a perfect match to how the VT520 sounds, but close enough. The VT520
 * is perhaps a little higher pitched, though that might just be the result of
 * the low quality audio hardware in the VT520. Just a little piezo beeper
 * buried between the PCB and the rear plastic of the otherwise fairly open
 * enclosure. Or maybe using a sawtooth instrument (81) is closer? hard to say.
 */
static void
MakeMidiSound(UCHAR note, UCHAR velocity, int duration_ms) {
    if (velocity && midiDeviceHandle != NULL) {
        /* note on */
        midiOutShortMsg(midiDeviceHandle,
            MAKELONG(MAKEWORD(0x90, note), MAKEWORD(velocity, 0)));
    }
    msleep(duration_ms);
    if (velocity && midiDeviceHandle != NULL) {
        /* note off */
        midiOutShortMsg(midiDeviceHandle,
            MAKELONG(MAKEWORD(0x80, note), MAKEWORD(velocity, 0)));
    }
}

#ifdef CK_HAVE_DSOUND
/****************** DirectSound Output ******************/
/* Sounds much better than the MIDI output, but needs DirectSound which isn't
 * available by default (or at all) on some ancient windows. */

/* DirectSound function pointers, etc */
static HANDLE hDirectSound = NULL;
#if DIRECTSOUND_VERSION >= 0x0800
typedef HRESULT (WINAPI *DirectSoundCreate8_t)(
    const GUID * lpcGuidDevice,
    LPDIRECTSOUND8 * ppDS8,
    LPUNKNOWN pUnkOuter
);
static DirectSoundCreate8_t p_DirectSoundCreate8=NULL;
#else
typedef HRESULT (WINAPI *DirectSoundCreate_t)(
    const GUID * lpcGuidDevice,
    LPDIRECTSOUND * ppDS,
    LPUNKNOWN pUnkOuter
);
static DirectSoundCreate_t p_DirectSoundCreate=NULL;
#ifndef DSBVOLUME_MIN
#define DSBVOLUME_MIN   -10000
#endif /* DSBVOLUME_MIN */
#ifndef DSBLOCK_ENTIREBUFFER
#define DSBLOCK_ENTIREBUFFER 0x00000002
#endif /* DSBLOCK_ENTIREBUFFER */
#endif /* DIRECTSOUND_VERSION >= 0x0800 */

/* DirectSound */
#define DS_BUFFERS 2
#if DIRECTSOUND_VERSION >= 0x0800
static LPDIRECTSOUND8 pDirectSound = NULL;
#else
static LPDIRECTSOUND pDirectSound = NULL;
#endif

static LPDIRECTSOUNDBUFFER pDirectSoundBuffer[DS_BUFFERS] = {NULL, NULL};

/* Triangle wave in 8bit PCM */
#define WAVE_DATA_SIZE 16
static char waveData[WAVE_DATA_SIZE] = {
    0, 32, 64, 96, 128, 159, 191, 223, 255, 223, 191, 159, 128, 96, 64, 32
};


/* I'm sure there must be an easier way of doing this, but I couldn't find it
 * in the docs... */
static char*
DSHresultString(HRESULT hr) {
    /* return reuslts from here:
     * https://learn.microsoft.com/en-us/previous-versions/windows/desktop/ee416775(v=vs.85)
     */
    switch (hr) {
        case DS_OK:
            return "DS_OK";
#ifdef DS_NO_VIRTUALIZATION
        case DS_NO_VIRTUALIZATION:
            return "DS_NO_VIRTUALIZATION";
#endif /* DS_NO_VIRTUALIZATION */
#ifdef DS_INCOMPLETE
        case DS_INCOMPLETE:
            return "DS_INCOMPLETE";
#endif /* DS_INCOMPLETE */
#ifdef DSERR_ACCESSDENIED
        case DSERR_ACCESSDENIED:
            return "DSERR_ACCESSDENIED";
#endif /* DSERR_ACCESSDENIED */
        case DSERR_ALLOCATED:
            return "DSERR_ALLOCATED";
        case DSERR_ALREADYINITIALIZED:
            return "DSERR_ALREADYINITIALIZED";
        case DSERR_BADFORMAT:
            return "DSERR_BADFORMAT";
#ifdef DSERR_BADSENDBUFFERGUID
        case DSERR_BADSENDBUFFERGUID:
            return "DSERR_BADSENDBUFFERGUID";
#endif /* DSERR_BADSENDBUFFERGUID */
        case DSERR_BUFFERLOST:
            return "DSERR_BUFFERLOST";
#ifdef DSERR_BUFFERTOOSMALL
        case DSERR_BUFFERTOOSMALL:
            return "DSERR_BUFFERTOOSMALL";
#endif /* DSERR_BUFFERTOOSMALL */
        case DSERR_CONTROLUNAVAIL:
            return "DSERR_CONTROLUNAVAIL";
#ifdef DSERR_DS8_REQUIRED
        case DSERR_DS8_REQUIRED:
            return "DSERR_DS8_REQUIRED";
#endif /* DSERR_DS8_REQUIRED */
#ifdef DSERR_FXUNAVAILABLE
        case DSERR_FXUNAVAILABLE:
            return "DSERR_FXUNAVAILABLE";
#endif /* DSERR_FXUNAVAILABLE */
        case DSERR_GENERIC:
            return "DSERR_GENERIC";
        case DSERR_INVALIDCALL:
            return "DSERR_INVALIDCALL";
        case DSERR_INVALIDPARAM:
            return "DSERR_INVALIDPARAM";
        case DSERR_NOAGGREGATION:
            return "DSERR_NOAGGREGATION";
        case DSERR_NODRIVER:
            return "DSERR_NODRIVER";
#if DIRECTSOUND_VERSION >= 0x0800
        case DSERR_NOINTERFACE:
            return "DSERR_NOINTERFACE";
#endif /* DIRECTSOUND_VERSION >= 0x0800 */
#ifdef DSERR_OBJECTNOTFOUND
        case DSERR_OBJECTNOTFOUND:
            return "DSERR_OBJECTNOTFOUND";
#endif /* DSERR_OBJECTNOTFOUND */
        case DSERR_OTHERAPPHASPRIO:
            return "DSERR_OTHERAPPHASPRIO";
        case DSERR_OUTOFMEMORY:
            return "DSERR_OUTOFMEMORY";
        case DSERR_PRIOLEVELNEEDED:
            return "DSERR_PRIOLEVELNEEDED";
#ifdef DSERR_SENDLOOP
        case DSERR_SENDLOOP:
            return "DSERR_SENDLOOP";
#endif /* DSERR_SENDLOOP */
        case DSERR_UNINITIALIZED:
            return "DSERR_UNINITIALIZED";
        case DSERR_UNSUPPORTED:
            return "DSERR_UNSUPPORTED";
        default:
            return "unknown";
    }
}

static void
CloseDirectSoundDevice() {
    int i;
    for (i = 0; i < DS_BUFFERS; i++) {
        if (pDirectSoundBuffer[i] != NULL) {
            debug(F101, "Closing DirectSound Buffer", 0, 1);
            IDirectSoundBuffer_Release(pDirectSoundBuffer[i]);
            pDirectSoundBuffer[i] = NULL;
        } else {
            debug(F101, "DirectSound Buffer is null", 0, 1);
        }
    }
    if (pDirectSound != NULL) {
        debug(F100, "Closing DirectSound Device...", 0, 0);
        IDirectSound_Release(pDirectSound);
        pDirectSound = NULL;
    }
}

static LPDIRECTSOUNDBUFFER
CreateDSBuffer() {
    LPDIRECTSOUNDBUFFER pDsBuf;
    HRESULT hr;
    WAVEFORMATEX waveFormat;
    DSBUFFERDESC dsBufferDesc;
    LPVOID bufferPtr;
    DWORD bufferSize;

    debug(F100, "Creating a DirectSoundBuffer...", 0, 0);

    /* Create a DirectSound Buffer to hold our triangle wave PCM sample */
    waveFormat.wFormatTag = WAVE_FORMAT_PCM;
    waveFormat.nChannels = 1;
    waveFormat.nSamplesPerSec = 8000; /* Sample rate - 8Hz */
    waveFormat.wBitsPerSample = 8;
    waveFormat.nBlockAlign = 1; /* = channels (1) * bitsPerSample (8) / 8 */
    waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec; /* * nBlockAlign (1) */

    dsBufferDesc.dwSize = sizeof(DSBUFFERDESC);
    dsBufferDesc.dwFlags =
          DSBCAPS_CTRLVOLUME         /* Volume control capability */
#ifdef DSBCAPS_GLOBALFOCUS
        | DSBCAPS_GLOBALFOCUS        /* Global sound buffer - focus not needed */
#endif /* DSBCAPS_GLOBALFOCUS */
        | DSBCAPS_CTRLFREQUENCY;     /* Frequency control capability */

    dsBufferDesc.dwBufferBytes = WAVE_DATA_SIZE;
    dsBufferDesc.lpwfxFormat = &waveFormat;

    hr = IDirectSound_CreateSoundBuffer(
        pDirectSound,       /* this */
        &dsBufferDesc,      /* Description of sound buffer we want */
        &pDsBuf,            /* Out: pointer to receive created sound buffer */
        NULL);              /* COM stuff. Must be null */
    if (FAILED(hr)) {
        debug(F111, "CreateSoundBuffer failed", DSHresultString(hr), hr);
        return NULL;
    }

    hr = IDirectSoundBuffer_Lock(
        pDsBuf,     /* this */
        0,                      /* Offset */
        WAVE_DATA_SIZE,         /* Size */
        &bufferPtr,             /* Pointer to start of locked buffer area */
        &bufferSize,            /* Number of bytes pointed to by bufferPtr */
        NULL,                   /* second region - don't care */
        NULL,                   /* size of second region - don't care */
        DSBLOCK_ENTIREBUFFER    /* Lock entire buffer */
        );
    if (FAILED(hr)) {
        debug(F111, "DirectSound Buffer Lock failed", DSHresultString(hr), hr);
        return NULL;
    }

    /* Copy our PCM sample into the buffer */
    memcpy(bufferPtr, waveData, WAVE_DATA_SIZE);

    hr = IDirectSoundBuffer_Unlock(
        pDsBuf,     /* this */
        bufferPtr,              /* Pointer to region being unlocked */
        bufferSize,             /* Size of region being unlocked */
        NULL,                   /* Don't have a second region */
        0                       /* So its size is 0 */
        );
    if (FAILED(hr)) {
        debug(F111, "DirectSound Buffer Unlock failed", DSHresultString(hr), hr);
        return NULL;
    }

    /* Mute the sound buffer */
    hr = IDirectSoundBuffer_SetVolume(
        pDsBuf,     /* this */
        DSBVOLUME_MIN           /* desired volume */
        );
    if (FAILED(hr)) {
        debug(F111, "DirectSound Buffer SetVolume failed", DSHresultString(hr), hr);
        return NULL;
    }

    /* And set it playing */
    hr = IDirectSoundBuffer_Play(
        pDsBuf,                 /* this */
        0,                      /* reserved */
        0,                      /* priority - none */
        DSBPLAY_LOOPING         /* loop forever */
        );
    if (FAILED(hr)) {
        debug(F111, "DirectSound Buffer Play failed", DSHresultString(hr), hr);
        return NULL;
    }

    debug(F100, "DirectSoundBuffer created!", 0, 0);
    return pDsBuf;
}

static bool
OpenDirectSoundDevice() {
    HRESULT hr;
    int i;

    debug(F100, "Attempting to open DirectSound device...", 0, 0);
    if (hDirectSound == NULL) {
        FARPROC p;

        hDirectSound = LoadLibrary("dsound.dll");
        if (hDirectSound == NULL) return FALSE;

#if DIRECTSOUND_VERSION >= 0x0800
        p = GetProcAddress(hDirectSound, "DirectSoundCreate8");
        if (p == NULL) return FALSE;
        p_DirectSoundCreate8=(DirectSoundCreate8_t)p;
#else
        p = GetProcAddress(hDirectSound, "DirectSoundCreate");
        if (p == NULL) return FALSE;
        p_DirectSoundCreate=(DirectSoundCreate_t)p;
#endif
    }

#if DIRECTSOUND_VERSION >= 0x0800
    hr = p_DirectSoundCreate8(NULL, &pDirectSound, NULL);
#else
    hr = p_DirectSoundCreate(NULL, &pDirectSound, NULL);
#endif
    if (FAILED(hr)) {
        debug(F111, "DirectSoundCreate8 failed", DSHresultString(hr), hr);
        return FALSE;
    }

    hr = IDirectSound_SetCooperativeLevel(
        pDirectSound, hwndConsole, DSSCL_PRIORITY);
    if (FAILED(hr)) {
        debug(F111, "SetCooperativeLevel failed", DSHresultString(hr), hr);
        CloseDirectSoundDevice();
        return FALSE;
    }

    /* Create DirectSound buffers */
    for (i = 0; i < DS_BUFFERS; i++) {
        pDirectSoundBuffer[i] = CreateDSBuffer();
        if (pDirectSoundBuffer[i] == NULL) {
            CloseDirectSoundDevice();
            return FALSE;
        }
    }

    /* Well, if we get this far it should work! */

    debug(F100, "DirectSound device open!", 0, 0);
    soundOutput = SOUND_OUT_DIRECT;
    return TRUE;
}

/* Play a sound using DirectSound */
static void
MakeDirectSound(UCHAR note, UCHAR velocity, int duration_ms) {
    static UCHAR lastNote = 0;
    static int lastDsBuf = 0;
    int freq;
    LONG volume;
    HRESULT hr;
    LPDIRECTSOUNDBUFFER dsBuf;

    if (note == lastNote) {
        /* Switch to the other buffer to get a bit of separation between
         * repetitions of the same note - the two buffers were created at
         * different times so should be at different playback positions. Not
         * very nice to *rely* on that, but it's not the end of the world if
         * somehow they end up synchronised. */
        lastDsBuf = (lastDsBuf + 1) % DS_BUFFERS;
    }

    dsBuf = pDirectSoundBuffer[lastDsBuf];

    freq = noteToFrequency(note);

    if (freq == 0 || note == 0 || velocity == 0 || dsBuf == 0) {
        MakeNoSound(duration_ms);
        return;
    }

    /* The triangle wave we're looping is just 16 samples long which, at 8000Hz,
     * will loop at 500Hz. So if we pass the desired frequency into
     * SetFrequency we'll get something way too low. Multiplying by the number
     * of samples we're looping will raise the pitch to what it should be. */
    freq *= WAVE_DATA_SIZE;

    hr = IDirectSoundBuffer_SetFrequency(
        dsBuf,
        (DWORD)freq
        );
    if (FAILED(hr)) {
        debug(F111, "MakeDirectSound: SetFrequency failed", DSHresultString(hr), hr);
        return;
    }

    /* Unmute the sound buffer at the corret volume.
     * General MIDI 2 (version 1.21, 6 February 2007) on page 7 (11 of the pdf)
     * says the volume in dB is 40*log(velocity/127). DirectSound uses
     * hundreds of a decibel though, so 4000*log(velocity/127) */
    volume = (LONG)(4000.0 * log10(velocity / 127.0));
    IDirectSoundBuffer_SetVolume(
        dsBuf,     /* this */
        volume);                /* desired volume */
    if (FAILED(hr)) {
        debug(F111, "MakeDirectSound: SetVolume failed", DSHresultString(hr), hr);
        return;
    }

    msleep(duration_ms);

    /* Done playing - mute the buffer again */
    IDirectSoundBuffer_SetVolume(
        dsBuf,     /* this */
        DSBVOLUME_MIN);         /* desired volume - none at all */
    if (FAILED(hr)) {
        debug(F111, "MakeDirectSound: SetVolume to mute failed", DSHresultString(hr), hr);
        return;
    }

    lastNote = note;
}
#endif /* CK_HAVE_DSOUND */
#endif /* NT */


void
OpenSoundDevice() {
#ifdef NT
#ifdef CK_HAVE_DSOUND
    if (OpenDirectSoundDevice()) {  /* Best, if its available */
        return;
    } else
#endif /* CK_HAVE_DSOUND */
    if (OpenMidiDevice()) {         /* Better than the beeper... */
        return;
    }

    debug(F100, "Failed to open any sound output device. Falling back to beeper.", 0, 0);
#endif /* NT */

    /* The Beeper is always available! At least until it returns an error */
    soundOutput = SOUND_OUT_BEEP;
}


void
CloseSoundDevice() {
#ifdef NT
    if (midiDeviceHandle != NULL) {
        debug(F100, "Closing MIDI device...", 0, 0);
        midiOutClose(midiDeviceHandle);
    }
#ifdef CK_HAVE_DSOUND
    CloseDirectSoundDevice();
#endif /* CK_HAVE_DSOUND */
#endif /* NT */
}


/* Tries to open a SoundDevice if one is not already open, then plays a single
 * note with the specified velocity (volume) for the specified duration (in
 * milliseconds) on it. This function blocks.
 *
 * If opening a sound device fails, then the function simply blocks for the
 * specified duration.
 */
void
MakeSound(UCHAR note, UCHAR velocity, int duration_ms) {
    /* Try to open the MIDI device if it isn't already open. If this fails,
     * we'll just treat every note as silent, but still do the sleep which is
     * probably better than nothing as applications using it might be relying
     * on its blocking nature. */
    if (soundOutput == SOUND_OUT_UNINITIALISED) {
        OpenSoundDevice();
    }

    if (note == 0) velocity = 0;

    switch (soundOutput) {
#ifdef NT
        case SOUND_OUT_MIDI:
            MakeMidiSound(note, velocity, duration_ms);
            break;
#ifdef CK_HAVE_DSOUND
        case SOUND_OUT_DIRECT:
            MakeDirectSound(note, velocity, duration_ms);
            break;
#endif /* CK_HAVE_DSOUND */
#endif /* NT */
        case SOUND_OUT_BEEP:
            MakeBeepSound(note, duration_ms);
            break;
        case SOUND_OUT_UNINITIALISED:
            /* This should never happen */
            debug(F100, "Error - OpenSoundDevice failed", 0, 0);
            /* fall through */
        case SOUND_OUT_NONE:
        default:
            MakeNoSound(duration_ms);
            break;
    }
}
