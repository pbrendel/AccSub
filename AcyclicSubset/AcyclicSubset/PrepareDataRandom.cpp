/* 
 * File:   PrepareDataRandom.cpp
 * Author: Piotr Brendel
 */

#include "PrepareDataRandom.h"
#include <cstdlib>


////////////////////////////////////////////////////////////////////////////////

void PrepareDataRandom::Prepare(SimplexList& simplexList, int packSize)
{
    srand(clock());
    for (int i = simplexList.size() - 1; i > 1; i--)
    {
        int index = rand() % i;
        Simplex s = simplexList[index];
        simplexList[index] = simplexList[i];
        simplexList[i] = s;
    }
}

////////////////////////////////////////////////////////////////////////////////
// eof
