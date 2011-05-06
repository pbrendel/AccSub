/*
 * File:   MPITest.h
 * Author: Piotr Brendel
 */

#ifndef MPITEST_H
#define	MPITEST_H

#include "../AcyclicSubset/Simplex.h"
#include "../AcyclicSubset/IncidenceGraph.h"

class MPITest
{
    static int rank;

    static void Master(int argc, char **argv);
    static void Slave(int processRank);

public:

    static void Test(int argc, char **argv);
    
    static int GetProcessRank() { return rank; }

};

#endif	/* MPITEST_H */

