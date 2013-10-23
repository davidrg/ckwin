#ifndef kszpopup_hxx_included
#define kszpopup_hxx_included

#include "kwin.hxx"

class KFont;
class KSizePopup : public KWin
{
public:
    KSizePopup( K_GLOBAL* );
    ~KSizePopup();

    void getCreateInfo( K_CREATEINFO* info );
    void createWin( KWin* par );
    void paint( int w, int h );

private:
    char tmp[16];
    HDC hdc;
    KFont* font;
};

#endif
