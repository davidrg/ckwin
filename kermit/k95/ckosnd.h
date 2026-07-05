#ifndef CKOSND_H
#define CKOSND_H
/*========================================================================
    Copyright (c) 2026, David Goodwin
-----------------------------------------------------------------------

    File.......: ckosnd.h
    Author.....: David Goodwin (david@zx.net.nz)
    Created....: July 5, 2026
    License....: BSD-3-Clause

    Description: Sound related functions

========================================================================*/

void OpenSoundDevice();
void CloseSoundDevice();
void MakeSound(UCHAR note, UCHAR velocity, int duration_ms);

#endif /* CKOSND_H */
