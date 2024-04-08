#ifndef PROGRESSBAR_H
#define PROGRESSBAR_H

#include "pch.h"

class ProgressBar
{
    static char const * const empty;
    static char const * const full;

    int rec_no;
    int rec_pos;
    int num_rec;
    int indic_len;
    string _filename;
    float percent;
    int i, n;

    public:
        void SetFileName(const string& filename);
        void step( );
        explicit ProgressBar( int );
        ~ProgressBar();
};
#endif

