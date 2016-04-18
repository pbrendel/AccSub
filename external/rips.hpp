/*
 * Authors: Pawel Dlotko, Huber Wagner
 */
#include <iostream>
#include <iostream>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <cmath>
#include <algorithm>
#include <cstdlib>
#include <cstdio>
#include <iterator>
#include<ctime>
#include <sstream>
#include <fstream>
#include <limits>
#include "reduction.h"


//z kobrzegami jest droby problem, gdyz potrzebuje je do tworzenia samego kompleksu, potem sa juz niepotrzebne. Dlatego, tak samo jak w przypadku wspolrzednych wierzcholkow
//sympleksow mozna je powywalac jak juz stworzymy kompleks, tylko potem trzeba uwazac na to, by ich nie urzywac.


std::vector< std::vector< double > > gradationOfSimplicesInFollowingDimensions;

namespace rips
{

template <typename T> class simplicialComplex;

//print matrix from persistence code
void print_matrix(const std::vector<std::vector<int> > &v)
{
	for (int i = 0; i < v.size(); i++, std::cout << std::endl)
	{
		std::cout << "column " << i << " : ";
		std::copy(v[i].begin(), v[i].end(), std::ostream_iterator<int>(std::cout, " "));
	}
}


//euclidan element for test
class euclidanPoint
{
public:
    template <typename VT>
    euclidanPoint( std::vector<VT> elements )
    {
        for ( size_t i = 0 ; i != elements.size() ; ++i )
        {
            this->coords.push_back( elements[i] );
        }
    }
    std::vector<double> coords;
    friend std::ostream& operator << ( std::ostream& out , euclidanPoint point );

    friend bool operator == ( euclidanPoint& first , euclidanPoint& second )
    {
        if ( first.coords.size() != second.coords.size() ){return false;}
        for ( size_t i = 0 ; i != first.coords.size() ; ++i )
        {
            if ( first.coords[i] != second.coords[i] )
            {
                return false;
            }
        }
        return true;
    }
    std::string writeForTest()
    {
        //do not change this method, it is used in unit tests!
        std::string result;
        result += "(";
        for ( size_t i = 0 ; i != this->coords.size() ; ++i )
        {
            char s[20];
            sprintf(s,"%f",this->coords[i]);
            result += s;
            if ( i != this->coords.size()-1 ){result += ",";}
        }
        result += ")";
        return result;
    }
};
std::ostream& operator << ( std::ostream& out , euclidanPoint point )
{
    out << "(";
    for ( size_t i = 0 ; i != point.coords.size() ; ++i )
    {
        out << point.coords[i];
        if ( i != point.coords.size()-1 ){out << ",";}
    }
    out << ")";
    return out;
}
double computeDistance( euclidanPoint first , euclidanPoint second )
{
    if ( first.coords.size() != second.coords.size() )
    {
        return 0;
    }
    double suma = 0;
    for ( size_t i = 0 ; i != first.coords.size() ; ++i )
    {
        suma += (first.coords[i] - second.coords[i])*(first.coords[i] - second.coords[i]);
    }
    return sqrt(suma);
}//computeDistance



//Data generation -- points from cycle
std::vector< euclidanPoint > generatePointsFromCircle( int iloscPunktow )
{
    srand(time(0));
    std::vector< euclidanPoint > points;
    while ( iloscPunktow-- )
    {
        double ra = (rand()/(double)(RAND_MAX));
        ra *= 2*3.14159265358979323846;
        //std::cout << ra << "\n";
        std::vector<double> po;
        po.push_back( sin(ra) );po.push_back( cos(ra) );
        euclidanPoint point(po);

        bool isAlreadyThere = false;
        for ( size_t i = 0 ; i != points.size() ; ++i )
        {
            if ( points[i] == point )
            {
                isAlreadyThere = true;
            }
        }
        if ( !isAlreadyThere )
        {
            points.push_back( point );
        }
    }
    return points;
}//generatePointsFromCircle


//Data generation -- points from cycle
std::vector< euclidanPoint > generatePointsFromFigure8( int iloscPunktow )
{
    std::vector< euclidanPoint > points1 = generatePointsFromCircle( iloscPunktow / 2 );
    std::vector< euclidanPoint > points2 = generatePointsFromCircle( iloscPunktow / 2 );

    //shift one cycle by 1:
    for ( size_t i = 0 ; i != points2.size() ; ++i )
    {
        points2[i].coords[0] += 2;
    }

    points1.insert( points1.end(), points2.begin(), points2.end() );

    return points1;

}//generatePointsFromFigure8





//Class of simplex and simplicial (Rips) complex

//T is a coordinate of vertex.
template <typename T>
class simplex
{
public:
    simplex()
    {
        this->nr = -1;
    };
    int dim(){return vertCoords.size()-1;}
    friend class simplicialComplex<T>;

    friend std::ostream& operator << ( std::ostream& out , simplex* sim )
    {
        //redHom standard:
        if ( sim->dim() == 0 )
        {
            out << sim->nr;
        }
        else
        {
            for ( size_t i = 0 ; i != sim->vertices.size() ; ++i )
            {
                out << sim->vertices[i]->nr << " ";
            }
        }
        /*
        out << "[";
        for ( size_t i = 0 ; i != sim->vertCoords->size() ; ++i )
        {
            out << (*sim->vertCoords)[i];
            if ( i != sim->vertCoords->size()-1 ){ out << ","; }
        }
        out << "]";
        */
        return out;
    }

    std::string writeSimplexForUnitTests()
    {
        //do not change this method, it is used in unit tests!
        std::string result;
        result+="[";
        for ( size_t i = 0 ; i != this->vertCoords->size() ; ++i )
        {
            result += (*this->vertCoords)[i].writeForTest();
            if ( i != this->vertCoords->size()-1 ){ result += ","; }
        }
        result += "]";
        return result;
    }


//private:
    std::vector< T > vertCoords;
    std::vector< simplex<T>* > vertices;
    double gradationLevel;
    std::vector< simplex<T>* > boundary;
    std::vector< simplex<T>* > coBoundary;
    int nr;
};

template< typename T >
bool compare(simplex<T>* first, simplex<T>* second )
{
    return (first->gradationLevel < second->gradationLevel);
}


//T is a coordinate of vertex.
template <typename T>
class simplicialComplex
{
public:
    simplicialComplex(){};
    simplicialComplex( const std::vector< T >& pointCloud , double epsilon , int maxCreatedDimension );
    friend class simplex<T>;
    std::vector<std::vector<std::vector<int> > > createBoundaryMatrices();

    void outputMaxSimplicesToFile( char* filename );
	  void outputMaxSimplices(std::vector<std::set<int> > &simplices);

    friend std::ostream& operator << ( std::ostream& out , simplicialComplex& cmplx )
    {
        for ( size_t i = 0 ; i != cmplx.elements.size() ; ++i )
        {
            out << "Dimension : " << i << "\n";
            for ( size_t j = 0 ; j != cmplx.elements[i].size() ; ++j )
            {
                out << cmplx.elements[i][j] << "\n";
                out << "Gradation : " << cmplx.elements[i][j]->gradationLevel << "\n";
                out << "Boundary elements : \n";
                for ( size_t k = 0 ; k != cmplx.elements[i][j]->boundary.size() ; ++k )
                {
                    out << cmplx.elements[i][j]->boundary[k] << " , ";
                }
                out << "\n coboundary elements : \n";
                for ( size_t k = 0 ; k != cmplx.elements[i][j]->coBoundary.size() ; ++k )
                {
                    out << cmplx.elements[i][j]->coBoundary[k] << " , ";
                }
                out << "\n\n\n";
            }
        }
        return out;
    }

    ~simplicialComplex()
    {
        for ( size_t i = 0 ; i != this->elements.size() ; ++i )
        {
            for ( size_t j = 0 ; j != this->elements[i].size() ; ++j )
            {
                delete this->elements[i][j];
            }
        }
    }
    std::vector< simplex<T>* > createVertices( const std::vector< T >& pointCloud );
    std::vector< simplex<T>* > createEdges( std::vector< simplex<T>* >& vertices , std::map< simplex<T>* , std::set<simplex<T>*> >& neigh , double epsilon );
    std::map< simplex<T>* , std::set< simplex<T>* > > createSimplicesNeigh( std::vector< simplex<T>* >& edges ,  std::map< simplex<T>* , std::set<simplex<T>*> >& neigh);
    void creadeHigherDimensionalSimplices( std::vector< simplex<T>* >& edges  , std::map< simplex<T>* , std::set<simplex<T>*> >& neigh , double epsilon , int maxCreatedDimension );


    std::vector< std::vector< simplex<T>* > > elements;
};



template <typename T>
void simplicialComplex<T>::outputMaxSimplicesToFile( char* filename )
{
    std::ofstream file;
    file.open( filename );
    for ( size_t dim = 0 ; dim != this->elements.size() ; ++dim )
    {
        for ( size_t i = 0 ; i != this->elements[dim].size() ; ++i )
        {
            //if coboundary of this->elements[dim][i] is empty, then output the simplex to the file.
            if ( this->elements[dim][i]->coBoundary.size() == 0 )
            {
                //write this simplex to the file.
                file << this->elements[dim][i] << std::endl;
            }
        }
    }
    file.close();
}

template <typename T>
void simplicialComplex<T>::outputMaxSimplices(std::vector<std::set<int> > &simplices)
{
    for ( size_t dim = 0 ; dim != this->elements.size() ; ++dim )
    {
        for ( size_t i = 0 ; i != this->elements[dim].size() ; ++i )
        {
            //if coboundary of this->elements[dim][i] is empty, then output the simplex to the vector
            if ( this->elements[dim][i]->coBoundary.size() == 0 )
            {
                std::set<int> s;
							  if(this->elements[dim][i]->vertices.size()== 0)
									s.insert(this->elements[dim][i]->nr);
                else 
									for (int k = 0; k < this->elements[dim][i]->vertices.size(); k++)
                	{
                   	 s.insert(this->elements[dim][i]->vertices[k]->nr);
                	}
                simplices.push_back(s);
            }
        }
    }
}

template <typename T>
std::vector<std::vector<std::vector<int> > > simplicialComplex<T>::createBoundaryMatrices()
{
    std::vector<std::vector<std::vector<int> > > result;



    //partial_0!!
    std::vector< std::vector<int> > partial0;
    for ( size_t i = 0 ; i != this->elements[0].size() ; ++i )
    {
        std::vector<int> s;
        partial0.push_back(s);
    }
    result.push_back(partial0);


    for ( size_t i = 1 ; i != this->elements.size() ; ++i )
    {
        //first create a map simplex* -> its position for dimension i-1
        std::map< simplex<T>* , int > simplex2pos;
        for ( size_t j = 0 ; j != this->elements[i-1].size() ; ++j )
        {
            simplex2pos.insert( std::make_pair( this->elements[i-1][j] , j ) );
        }
        std::vector< std::vector<int> > bdMatrix;
        for ( size_t j = 0 ; j != this->elements[i].size() ; ++j )
        {
            std::vector<int> bdOfThisElement;
            for ( size_t k = 0 ; k != this->elements[i][j]->boundary.size() ; ++k )
            {
                bdOfThisElement.push_back(simplex2pos.find(this->elements[i][j]->boundary[k])->second);
            }
            std::sort( bdOfThisElement.begin() , bdOfThisElement.end() );
            if (std::unique(bdOfThisElement.begin(), bdOfThisElement.end()) != bdOfThisElement.end())
            {
                std::cout << "probelm z brzegiem :" << this->elements[i][j] << "\n";



                std::cout << "aaaaaaaaaaaaa!";
                std::cin.ignore();

            }
            // bdOfThisElement.erase(std::unique(bdOfThisElement.begin(), bdOfThisElement.end()), bdOfThisElement.end());
            bdMatrix.push_back( bdOfThisElement );
        }
        result.push_back( bdMatrix );
    }
    return result;
}//createBoundaryMatrices


template <typename T>
std::vector< simplex<T>* > simplicialComplex<T>::createVertices( const std::vector< T >& pointCloud )
{
    //create vertices
    std::vector< simplex<T>* > vertices;
    for ( size_t i = 0 ; i != pointCloud.size() ; ++i )
    {
        simplex<T>* vertex = new simplex<T>;
        vertex->nr = i;
        vertex->vertCoords.push_back( pointCloud[i] );
        vertex->gradationLevel = 0;
        vertices.push_back( vertex );
    }
    this->elements.push_back( vertices );
    return vertices;
}//createVertices




template <typename T>
std::vector< simplex<T>* > simplicialComplex<T>::createEdges( std::vector< simplex<T>* >& vertices , std::map< simplex<T>* , std::set<simplex<T>*> >& neigh , double epsilon )
{
    //to create complex & avoid repetiotions we will use neighbourList
    std::vector< std::vector<int> > neighbourList;
    std::map< std::pair< simplex<T>* , simplex<T>* > , double > distanceMap;
    for ( size_t i = 0 ; i != vertices.size() ; ++i )
    {
        std::vector<int> neighOfIth;
        for ( size_t j = i+1 ; j != vertices.size() ; ++j )
        {
             double dist = computeDistance( vertices[i]->vertCoords[0] , vertices[j]->vertCoords[0] );
             if ( dist <= epsilon )
             {
                   neighOfIth.push_back( j );
                   neigh.find( vertices[i] )->second.insert( vertices[j] );
                   distanceMap.insert( std::make_pair( std::make_pair(vertices[i],vertices[j]) , dist) );
                   distanceMap.insert( std::make_pair( std::make_pair(vertices[j],vertices[i]) , dist) );
             }
        }
        neighbourList.push_back( neighOfIth );
    }

    int dim = 1;

    //creating edges:
    std::vector< simplex<T>* > edges;
    for ( size_t i = 0 ; i != this->elements[ dim-1 ].size() ; ++i )
    {
        for ( size_t j = 0 ; j != neighbourList[i].size() ; ++j )
        {
            //create an edge from vertices[i] to vertices[j] having gradation distanceList[i][j]
            simplex<T>* edge = new simplex<T>;
            edge->vertCoords.push_back( vertices[i]->vertCoords[0] );
            edge->vertCoords.push_back( vertices[neighbourList[i][j]]->vertCoords[0] );
            edge->gradationLevel = distanceMap.find( std::make_pair( vertices[i] , vertices[ neighbourList[i][j] ] ) )->second;
            edge->boundary.push_back( vertices[i] );
            edge->boundary.push_back( vertices[ neighbourList[i][j] ] );
            edge->vertices.push_back( vertices[i] );
            edge->vertices.push_back( vertices[ neighbourList[i][j] ] );
            vertices[i]->coBoundary.push_back( edge );
            vertices[ neighbourList[i][j] ]->coBoundary.push_back( edge );
            edges.push_back(edge);
        }
    }
    this->elements.push_back(edges);
    return edges;
}

template <typename T>
std::map< simplex<T>* , std::set< simplex<T>* > > simplicialComplex<T>::createSimplicesNeigh( std::vector< simplex<T>* >& edges  , std::map< simplex<T>* , std::set<simplex<T>*> >& neigh)
{
    int dbg = 0;

    std::map< simplex<T>* , std::set< simplex<T>* > > simplicesNeigh;
    for ( size_t i = 0 ; i != edges.size() ; ++i )
    {
        std::set< simplex<T>* > s;
        simplicesNeigh.insert( std::make_pair( edges[i] , s ) );
        //pick an edge and compute its neighbours
        for ( typename std::set<simplex<T>*>::iterator it = neigh.find(edges[i]->boundary[0])->second.begin() ; it != neigh.find(edges[i]->boundary[0])->second.end() ; ++it )
        {
             if ( neigh.find(edges[i]->boundary[1])->second.find( *it ) != neigh.find(edges[i]->boundary[1])->second.end() )
             {
                 simplicesNeigh.find( edges[i] )->second.insert( *it );
             }
        }
        //dbg
        if (dbg)
        {
            /*
            std::cerr << "Neighbours of : " << edges[i] << "\n";
            for ( typename std::set<simplex<T>*>::iterator it = simplicesNeigh.find(edges[i])->second.begin() ; it != simplicesNeigh.find(edges[i])->second.end() ; ++it )
            {
                std::cerr << *it << "\n";
            }
            getchar();
            */
        }
    }

    return simplicesNeigh;
}//createSimplicesNeigh


template <typename T>
void simplicialComplex<T>::creadeHigherDimensionalSimplices( std::vector< simplex<T>* >& edges  , std::map< simplex<T>* , std::set<simplex<T>*> >& neigh , double epsilon , int maxCreatedDimension )
{
    int dbg = 0;
    std::map< simplex<T>* , std::set< simplex<T>* > > simplicesNeigh = this->createSimplicesNeigh( edges  , neigh );

    int dim = 1;
    for ( dim = 2 ; dim <= maxCreatedDimension ; ++dim )
    {
        std::map< simplex<T>* , std::set< simplex<T>* > > simplicesNeighNext;

        std::vector< simplex<T>* > elems;
        for ( size_t i = 0 ; i != this->elements[ dim-1 ].size() ; ++i )
        {
             //if there is nothing to create, continue.
             if ( simplicesNeigh.find( this->elements[ dim-1 ][i] )->second.size() == 0 ){continue;}

             for ( typename std::set< simplex<T>* >::iterator it = simplicesNeigh.find(this->elements[ dim-1 ][i])->second.begin() ; it != simplicesNeigh.find( this->elements[ dim-1 ][i] )->second.end() ; ++it )
             {
                 if (dbg){std::cerr << "\n\n\n Building simplex based on : " << this->elements[ dim-1 ][i] << " and a vertex :  " << *it << "\n";}
                 //getchar();
                 simplex<T>* elem = new simplex<T>;

                 //fill in vertices:
                 for ( size_t ver = 0 ; ver != this->elements[ dim-1 ][i]->vertices.size() ; ++ver )
                 {
                     elem->vertices.push_back( this->elements[ dim-1 ][i]->vertices[ver] );
                 }
                 elem->vertices.push_back( *it );

                 //fill simplicesNeighNext map:
                 std::set< simplex<T>* > neighsOfNewSimplex;
                 for ( typename std::set< simplex<T>* >::iterator nb = simplicesNeigh.find(this->elements[ dim-1 ][i])->second.begin() ; nb != simplicesNeigh.find( this->elements[ dim-1 ][i] )->second.end() ; ++nb )
                 {
                     if ( neigh.find( *it )->second.find( *nb ) != neigh.find( *it )->second.end())
                     {
                          neighsOfNewSimplex.insert( *nb );
                     }
                 }
                 simplicesNeighNext.insert( std::make_pair( elem , neighsOfNewSimplex ) );
                 if (dbg){std::cerr << "neighsOfNewSimplex.size() : " << neighsOfNewSimplex.size() << "\n";getchar();}

                 if (dbg){std::cerr << "set vertCoords for elem: \n";getchar();}
                 //set vertCoords for elem:
                 for ( size_t j = 0 ; j != this->elements[ dim-1 ][i]->vertCoords.size() ; ++j )
                 {
                     elem->vertCoords.push_back( this->elements[ dim-1 ][i]->vertCoords[j] );
                 }
                 elem->vertCoords.push_back( (*it)->vertCoords[0] );



                 if (dbg){std::cerr << "set gradationLevel for elem \n";}
                 //set gradationLevel for elem:
                 double gradLev = this->elements[ dim-1 ][i]->gradationLevel;
                 for ( size_t j = 0 ; j != this->elements[ dim-1 ][i]->vertCoords.size() ; ++j )
                 {
                      double dst = computeDistance( this->elements[ dim-1 ][i]->vertCoords[j] , (*it)->vertCoords[0] );
                      if ( gradLev < dst ){gradLev = dst;}
                 }
                 elem->gradationLevel = gradLev;
                 if (dbg){std::cerr << "Gradation level : " << gradLev << "\n";}

                 if (dbg)
                 {
                     std::cerr << "elem : " << elem << "\n";
                     std::cerr << "it : " << *it << "\n";
                     std::cerr << "Set boundary and coBoundary for elem \n";
                     //getchar();
                 }
                 //set boundary and coBoundary for elem:
                 //the easiest part is to set it for this->elements[ dim-1 ][i]:
                 this->elements[ dim-1 ][i]->coBoundary.push_back( elem );
                 elem->boundary.push_back( this->elements[ dim-1 ][i] );
                 //now we need to set all remaining elements:
                 for ( typename std::vector< simplex<T>* >::iterator bd = this->elements[ dim-1 ][i]->boundary.begin() ; bd != this->elements[ dim-1 ][i]->boundary.end() ; ++bd )
                 {
                     if (dbg){std::cerr << "*bd :  " << *bd << "\n";}
                     for ( typename std::vector< simplex<T>* >::iterator cbd = (*bd)->coBoundary.begin() ; cbd != (*bd)->coBoundary.end() ; ++cbd )
                     {
                          if ( *cbd == elem ){continue;}
                          if (dbg){std::cerr << "Cbd : " << *cbd << "\n";}
                          //if (*cbd) has (*it) as one of vertices, then we are done!
                          for ( typename std::vector<T>::iterator fi = (*cbd)->vertCoords.begin(); fi != (*cbd)->vertCoords.end() ; ++fi )
                          {
                              if ( (*it)->vertCoords[0] == *fi )
                              {
                                  if (dbg){std::cerr << "Ustawiam !! \n";getchar();getchar();}
                                  //set boundary and coboundary!
                                  (*cbd)->coBoundary.push_back( elem );
                                  elem->boundary.push_back( *cbd );
                              }
                          }
                     }
                 }
                 elems.push_back( elem );
             }
        }
        this->elements.push_back( elems );
                //dbg
        if (dbg)
        {
            std::cout << "elems.size() :  " << elems.size() << "\n";
            getchar();
        }
        //tutaj trzeba jeszcze uzupe�ni� list� neigth
        simplicesNeigh.clear();
        simplicesNeigh = simplicesNeighNext;
    }
}


template <typename T>
simplicialComplex<T>::simplicialComplex( const std::vector< T >& pointCloud , double epsilon , int maxCreatedDimension )
{
    int dbg = 0;
    if (dbg){std::cerr << "Rips complex construction \n";getchar();}

    std::vector< simplex<T>* > vertices = this->createVertices( pointCloud );


    std::map< simplex<T>* , std::set<simplex<T>*> > neigh;
    for ( size_t i = 0 ; i != vertices.size() ; ++i )
    {
        std::set<simplex<T>*> s;
        neigh.insert( std::make_pair( vertices[i] , s ) );
    }

    std::vector< simplex<T>* > edges = createEdges( vertices , neigh , epsilon );


    if (dbg){std::cerr << "edges.size() : " << edges.size() << "\n";}


    if ( maxCreatedDimension == 1 ){return;}


    //creating higher dim elements
    creadeHigherDimensionalSimplices( edges  ,  neigh , epsilon , maxCreatedDimension );


    //at the end it would be good to sort simplices in elements according to gradation, so after we can easilly produce boundary matrices.
    for ( size_t i = 0 ; i != this->elements.size() ; ++i )
    {
        std::sort( this->elements[i].begin() , this->elements[i].end() , compare<T> );
    }


    //two aims for this set of instructions: first is to fill in gradationOfSimplicesInFollowingDimensions vector, so at the end we will have length of barcodes.
    //second is to remove from all simplices vertCoords vector (although complex can not be written anymore after that, since one will get seg fault) -- therefore this
    //instruction is commented for now!
    //This is to spare the memory, since we need vertCoords for construction, but we do not need it after.
    //The same story with coboundary list, although coBoundary maybe also delted on fly when needed.
    for ( size_t i = 0 ; i != this->elements.size() ; ++i )
    {
        std::vector< double > grad;
        for ( size_t j = 0 ; j != this->elements[i].size() ; ++j )
        {
             grad.push_back( this->elements[i][j]->gradationLevel );
             //delete this->elements[i][j]->vertCoords;
             //delete this->elements[i][j]->coBoundary;
        }
        gradationOfSimplicesInFollowingDimensions.push_back(grad);
    }
}


bool compareBarcodes( std::pair< double, double > first , std::pair< double, double > second )
{
    return ( (first.second - first.first) < (second.second - second.first) );
}

void printBarcodesToPostscript( std::vector< std::vector< std::pair< double, double > > >& barcodes , std::string filename )
{

    std::vector< std::vector< std::pair< double, double > > > infiniteBarcodes;
    std::vector< std::vector< std::pair< double, double > > > finiteBarcodes;
    //extract finite and infinite barcodes:
    for ( size_t i = 0 ; i != barcodes.size() ; ++i )
    {

        std::cerr << " i : " << i  << "\n";

        std::vector< std::pair< double, double > > infiniteBar;
        std::vector< std::pair< double, double > > finiteBar;

        for ( size_t j = 0 ; j != barcodes[i].size() ; ++j )
        {
            if ( barcodes[i][j].second == -1 )
            {
                infiniteBar.push_back(std::make_pair( barcodes[i][j].first ,barcodes[i][j].second ));
            }
            else
            {
                finiteBar.push_back(std::make_pair(barcodes[i][j].first ,barcodes[i][j].second ));
            }
        }
        infiniteBarcodes.push_back(infiniteBar);
        finiteBarcodes.push_back(finiteBar);
    }


    //std::cerr << "infiniteBarcodes : " << infiniteBarcodes.size() << "\n";
    //std::cerr << "finiteBarcodes : " << finiteBarcodes.size() << "\n";
    //std::cerr << "starting to sort \n";

    //now let us sort only finite barcodes:
    for ( size_t i = 0 ; i != barcodes.size() ; ++i )
    {
        std::sort( finiteBarcodes[i].begin() , finiteBarcodes[i].end() , compareBarcodes );
    }

    //now it is time to write them donw into a file:


    for ( size_t dim = 0 ; dim != finiteBarcodes.size() ; ++dim )
    {
        //calculate maximal value from dinite barcodes:
        double maxVal = 0;
        for ( size_t j = 0 ; j != finiteBarcodes[dim].size() ; ++j )
        {
            if ( finiteBarcodes[dim][j].second > maxVal ){maxVal = finiteBarcodes[dim][j].second;}
        }

        std::ofstream file;
        std::ostringstream ss;
        ss << dim;
        std::string str = ss.str();
        std::string thisFile(filename);
        thisFile += str;
        thisFile += ".ps";
        file.open(thisFile.c_str());


        file << "newpath\n";

        //scale on axies:
        file << "/Times-Roman findfont \n 12 scalefont\n setfont \n newpath \n 0 2 moveto\n ";
        file << "(0) show \n";
        file << "570 0 moveto \n";
        file <<"(" << ceil(maxVal) << ") show \n";

        std::cerr << ceil(maxVal) << "\n";
        std::cerr << maxVal << "\n";
        getchar();




        //first infinite barcodes
        int elevation = 5;
        for ( size_t j = 0 ; j != infiniteBarcodes[dim].size() ; ++j )
        {
             int begin = (int)(infiniteBarcodes[dim][j].first/maxVal * 600);

             file << begin << " " << elevation << " moveto\n";
             file << "600 " << elevation << " lineto\n";

            std::cerr << "NIESKOCZONA linia od : (" << elevation << "," << begin <<") do (" <<elevation << ",600) w wymiarze : " << dim << "\n";

             elevation += 3;
        }

        elevation += 40;

        std::cerr << "\n";
        //and now finite dimensional barcodes:
        for ( size_t j = 0 ; j != finiteBarcodes[dim].size() ; ++j )
        {
             int begin = (int)(finiteBarcodes[dim][j].first/maxVal * 550);
             int end = (int)(finiteBarcodes[dim][j].second/maxVal * 550);
             file << begin << " " << elevation << " moveto\n";
             file << end << " " <<elevation << " lineto\n";

             //std::cerr << "linia od : (" << elevation << "," << begin <<") do (" <<elevation << "," <<end << ")\n";
             elevation += 3;
        }
        file << "2 setlinewidth\n stroke\n";
        file.close();
    }
}//printBarcodesToPostscript


std::vector<int> bettiNumbers( std::vector< std::vector< std::pair< double, double > > > barcode )
{
    std::vector<int> betti;
    for ( size_t dim = 0 ; dim != barcode.size() ; ++dim )
    {
        int bettiInThisDim = 0;
        for ( size_t i = 0 ; i != barcode[dim].size() ; ++i )
        {
            if ( barcode[dim][i].second == -1 )
            {
                ++bettiInThisDim;
            }
        }
        betti.push_back( bettiInThisDim );
    }
    return betti;
}

}
