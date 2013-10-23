#ifndef kcmd_h_included
#define kcmd_h_included

const int ACTION_NONE           = 0;
const int ACTION_SHOW_BUTTON    = 1;
const int ACTION_SHOW_EDIT      = 2;
const int ACTION_SHOW_COMBO     = 3;
const int ACTION_SHOW_DIALOG	= 4;

typedef struct _k_cell {
    int action;
    char* text;
} k_cell;

#endif
