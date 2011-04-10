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

    static void Master(int argc, char **argv);
    static void Slave();

public:

    static void Test(int argc, char **argv);

};

#endif	/* MPITEST_H */

