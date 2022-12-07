#include "ProgressBar.h"

char const* const ProgressBar::empty = " ";
char const* const ProgressBar::full  = "*";

ProgressBar::~ProgressBar()
{
    printf("\r-----> Finished                                                               ");
    printf( "\n" );
    fflush(stdout);
}

ProgressBar::ProgressBar( int row_count )
{
    rec_no    = 0;
    rec_pos   = 0;
    indic_len = 20;
    num_rec   = row_count;

    for ( int i = 0; i < indic_len; i++ )
        printf( empty );

    printf("\r Loading list of files to parse, Please Wait....");
    fflush(stdout);
}

void ProgressBar::step(string filename)
{
    int i, n;

    if (num_rec == 0)
        return;

    ++rec_no;
    n = rec_no * indic_len / num_rec;
    if (n != rec_pos)
    {
        printf("\r                                                  [");

        for (i = 0; i < n; i++) printf(full);

        for (; i < indic_len; i++) printf(empty);

        float percent = (((float)n / (float)indic_len) * 100);

        string outText = "";
        if (filename.size() > 34)
        {
            outText.append("...");
            outText.append(filename.substr(filename.size() - 31));
        }
        else
            outText = filename;

        printf("] %i%%  \r Loading file: %s", (int)percent, outText.c_str());

        fflush(stdout);

        rec_pos = n;
    }
}

