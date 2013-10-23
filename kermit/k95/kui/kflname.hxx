#ifndef kflname_hxx_included
#define kflname_hxx_included

typedef struct _K_FILENAME {
    char* name;
    char* mode;
} K_FILENAME;

class KArray;
class KFileName 
{
public:
    KFileName();
    ~KFileName();

    K_FILENAME* add( char* n, char* m );
    void remove( K_FILENAME* findfn );
    void removeAll();

    void toggle( K_FILENAME* findfn );
    long numFiles() { return arraycount; }

    void executeSend();

private:
    KArray* filearray;
    long arraycount;

};

#endif
