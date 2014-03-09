//
// Copyright (C) 2007 Davide De Rosa
// License: http://www.gnu.org/licenses/gpl.html GPL version 3 or higher
//

#include "common.h"

using namespace crucio;
using namespace std;

static nullbuf null_obj;
static wnullbuf wnull_obj;
ostream crucio::cnull(&null_obj);
wostream crucio::wcnull(&wnull_obj);

ostream* crucio::crucio_vout = &cnull;

string crucio::ABMaskString(const Alphabet alphabet, const ABMask mask)
{
    string s = "";

    s += "{";
    uint32_t i;
    const size_t size = alphabetSize(alphabet);
    for (i = 0; i < size; ++i) {
        if (mask[i]) {
            s += index2Character(alphabet, i);
        }
    }
    s += "}";

    return s;
}
