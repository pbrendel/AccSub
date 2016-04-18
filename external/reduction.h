// Copyright Hubert Wagner, 2011
#ifndef _REDUCTION_H_INCLUDED_
#define _REDUCTION_H_INCLUDED_

#include <iostream>
#include <cassert>
#include <vector>
#include <limits>
#include <list>
#include <algorithm>

#include "utils.h"

// This function brings the 'upper_boundary' to a reduced form.
// Upper and lower boundary represent a boundary matrix of dimension
// d and d-1 respectively. Both matrices should be ordered according to the
// filtration.
//
// Each column in the reduced matrix is either empty or has a unique lowest (largest)
// element.
//
// ColumnType should typically be just a vector<unsigned int/long>
//
// Killed columns contain a single number -1 (KILLED). It's used in a lower dimension to
// handle columns which a zero (for example for Morse complexes).
template<typename ColumnType>
std::vector< std::pair<double , double> > reduce_one_dimension(std::vector<ColumnType> &upper_boundary, std::vector<ColumnType> &lower_boundary, int dim)
{
    extern std::vector< std::vector< double > > gradationOfSimplicesInFollowingDimensions;

    std::vector< std::pair<double , double> > barcode;

    const int KILLED = -1;

    typedef typename ColumnType::value_type IndexType;
    const IndexType UNDEFINED_INDEX = std::numeric_limits<IndexType>::max();

    std::vector<IndexType> low_array(lower_boundary.size(), UNDEFINED_INDEX);

    const size_t sz = upper_boundary.size();

    for (size_t i = 0; i < sz; i++)
    {
        if (upper_boundary[i].empty())
        {
            // std::cout << "infinite interval in dimension : " << dim << " is : "<< i << std::endl;
			//-1 istead of infinity!
            barcode.push_back(std::make_pair( gradationOfSimplicesInFollowingDimensions[dim][i], -1 ));
            deallocate(upper_boundary[i]);
            continue;
        }

        if (upper_boundary[i].size() == 1 && upper_boundary[i][0] == KILLED)
            continue;

        IndexType low = upper_boundary[i].back();

        while (!upper_boundary[i].empty() && low_array[low] != UNDEFINED_INDEX)
        {
            assert(low_array[low] < i);
            assert(low == upper_boundary[low_array[low]].back());
            assert(!upper_boundary[i].empty());
            assert(!upper_boundary[low_array[low]].empty());

            list_sym_diff(upper_boundary[i],
                          upper_boundary[low_array[low]],
                          upper_boundary[i]);

            if (!upper_boundary[i].empty()) {
                IndexType old_low = low;
                low = upper_boundary[i].back();
                assert(low < old_low);
            }
        }
        if (!upper_boundary[i].empty()) {
            assert(low >= 0);
            assert(low < low_array.size());
            assert(low_array[low] == UNDEFINED_INDEX);

			//std::cout << "finite interval" << low << " " << i << std::endl;
            barcode.push_back( std::make_pair( gradationOfSimplicesInFollowingDimensions[dim][low] , gradationOfSimplicesInFollowingDimensions[dim][i] ) );

            low_array[low] = i;
            // deallocate(lower_boundary[low]);
            lower_boundary[low].clear();
            lower_boundary[low].push_back(KILLED);

        }

        if (upper_boundary[i].empty()) {
            //if ( dim == 1 )
            //    std::cout << "infinite interval in dimension : " <<dim<< " is : "<< i << std::endl;
			//-1 istead of infinity!
            barcode.push_back( std::make_pair( gradationOfSimplicesInFollowingDimensions[dim][i] , -1 ) );
        }
    }

    return barcode;
}

// This unfction reduces all boundary matrices, from higher to lower dimensions.
//
// It is assumed that boundary_matrices[d] is the boundary matrix
// taking d-cells to (d-1)-cells.
//
// It uses a 'twist' by Kerber and Chen, clearing (zeroing) some columns of
// the lower dimensional boundary matrix.
//
// Ideally, the cleared columns not be calculated at all, but it required
// integrating this procedure with the code calculating the boundary matrix.
template<typename ColumnType>
        std::vector< std::vector< std::pair< double, double > > > reduction_with_a_twist(std::vector<std::vector<ColumnType> >& boundary_matrices)
{
    const int dimension = static_cast<int>(boundary_matrices.size());

    std::vector< std::vector< std::pair< double, double > > > barcodes( dimension );

    for (int d = dimension - 1; d >= 1; d--)
    {
        //std::cerr << "reducing: " << d << std::endl;
        std::vector< std::pair< double, double > > barcode = reduce_one_dimension(boundary_matrices[d], boundary_matrices[d-1],d);
        barcodes[ d ] =  barcode;
    }

    std::vector< std::pair< double, double > > barcode_0;

    extern std::vector< std::vector< double > > gradationOfSimplicesInFollowingDimensions;

    // handle NON-KILLED vertices separately!
    std::cout << "vertices..." << std::endl;
    for (size_t i = 0; i < boundary_matrices[0].size(); i++)
    {
        if (boundary_matrices[0][i].empty()) // NOT KILLED
        {
            barcode_0.push_back(std::make_pair(gradationOfSimplicesInFollowingDimensions[0][i], -1));
        }
        else deallocate(boundary_matrices[0][i]);
    }

    barcodes[0] = barcode_0;

    return barcodes;
}


#endif
