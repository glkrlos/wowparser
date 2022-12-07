#ifndef _PROGRESSBAR_H
#define _PROGRESSBAR_H

#include "pch.h"

class ProgressBar
{
    static char const * const empty;
    static char const * const full;

    int rec_no;
    int rec_pos;
    int num_rec;
    int indic_len;

    public:

        void step( string filename );
        ProgressBar( int );
        ~ProgressBar();
};
#endif

