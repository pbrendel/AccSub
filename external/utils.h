// Copyright Hubert Wagner, 2011
#ifndef _UTILS_H_INCLUDED_
#define _UTILS_H_INCLUDED_

// Deallocates internal storage of a vector. (capacity() == 0 after calling this).
// Calling clear on a vector is not enough.
template<typename ListT>
void deallocate(ListT &l)
{
	ListT().swap(l);
}

// In the unlikely event we use lists, we don't need any tricks to
// deallocate memory.
template<typename ElemT>
void deallocate(std::list<ElemT> &l)
{
	l.clear();
}

// This is a simple output iterator which just counts
// the number of elements assigned/outputted to it,
// but disregards the content of these elements.
template<class Container>
struct CountIterator
 : public std::iterator<std::output_iterator_tag,
                         void, void, void, void>
{
	size_t &cnt;

    CountIterator(size_t &x) : cnt(x) {}

	template<typename t>
    CountIterator& operator=(t)
	{
        return *this;
    }

    CountIterator& operator* ()
	{
        return *this;
    }

    CountIterator& operator++(int)
	{
		++cnt;
        return *this;
    }

	CountIterator& operator++()
	{
		++cnt;
        return *this;
    }
};

// This function performs a XOR of two (pre-sorted) vectors.
//
// We avoid excessive allocations by calculating the size of the result.
// This has been experimentally checked to be faster than automatically
// growing the output vector. Also, it can help avoid memory fragmentation.
//
// We resize the result and populate it with the actual values in a
// second pass.
//
// TODO: It would be nice to do this in place...
template<typename ColumnType>
void list_sym_diff(const ColumnType &col1, const ColumnType &col2,
				   ColumnType &sorted_result){
	//assume a,b are both sorted increasingly
	size_t count = 0;
	CountIterator<ColumnType> counter(count);
	set_symmetric_difference(col1.begin(), col1.end(), col2.begin(), col2.end(), counter);

	ColumnType out;
	out.reserve(count);
	set_symmetric_difference(col1.begin(), col1.end(), col2.begin(), col2.end(), back_inserter(out));

	out.swap(sorted_result);
}

#endif
