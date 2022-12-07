/*
 * Copyright (C) 2005-2008 MaNGOS <http://www.mangosproject.org/>
 *
 * Copyright (C) 2008 Trinity <http://www.trinitycore.org/>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

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

