#include "ProgressBar.h"

char const* const ProgressBar::empty = " ";
char const* const ProgressBar::full  = "*";

ProgressBar::~ProgressBar()
{
    _filename.clear();
    printf("\r");

    for (auto x = 0; x < 78; x++)
        printf(empty);

    printf("\r");

    fflush(stdout);
}

ProgressBar::ProgressBar( int row_count )
{
    rec_no    = 0;
    rec_pos   = 0;
    indic_len = 20;
    num_rec   = row_count;
    percent = 0.0f;
    i = 0;
    n = 0;

    for ( int i = 0; i < indic_len; i++ )
        printf( empty );

    printf("\r Loading list of files to parse, Please Wait....");
    fflush(stdout);
}
void ProgressBar::SetFileName(string filename)
{
    string _filename = "";
    if (filename.size() > 34)
    {
        _filename.append("...");
        _filename.append(filename.substr(filename.size() - 31));
    }
    else
        _filename = filename;

    printf("\r");

    for (auto x = 0; x < 50; x++)
        printf(empty);

    printf("[");

    for (i = 0; i < n; i++) printf(full);

    for (; i < indic_len; i++) printf(empty);

    printf("] %i%%  \r Loading file: %s", (int)percent, _filename.c_str());

    fflush(stdout);
}

void ProgressBar::step()
{
    if (num_rec == 0)
        return;

    ++rec_no;
    n = rec_no * indic_len / num_rec;
    if (n != rec_pos)
    {
        printf("\r");

        for (auto x = 0; x < 50; x++)
            printf(empty);

        printf("[");

        for (i = 0; i < n; i++) printf(full);

        for (; i < indic_len; i++) printf(empty);

        percent = (((float)n / (float)indic_len) * 100);

        printf("] %i%%  \r Loading file: %s", (int)percent, _filename.c_str());

        fflush(stdout);

        rec_pos = n;
    }
}

