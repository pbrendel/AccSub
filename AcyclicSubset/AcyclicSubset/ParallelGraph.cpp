#include "ParallelGraph.h"
#include "IncidenceGraph.h"
#include "IncidenceGraphUtils.h"
#include "../Helpers/Utils.h"
#include "redHom/complex/scomplex/SComplexBuilderFromSimplices.hpp"

#include <iostream>
#include <algorithm>
#include <set>
#include <map>
#include <list>
#include <cstdlib>
#include <cmath> // ceil()

#ifdef USE_MPI
#include <mpi.h>
#include "MPIData.h"
#endif


// test!!!
#include "../Helpers/Tests.h"

////////////////////////////////////////////////////////////////////////////////

void ParallelGraph::DataNode::CreateIncidenceGraphLocally(const IncidenceGraph::Params &params, AcyclicTest<IncidenceGraph::IntersectionFlags> *test)
{
    ig = IncidenceGraph::CreateWithBorderVerts(simplexPtrList, borderVerts, params);
    ig->CalculateAcyclicSubsetWithSpanningTree(test);
    ig->RemoveAcyclicSubset();
}

int ParallelGraph::DataNode::GetConstantSimplexSize()
{
   int size = simplexPtrList[0]->size();
   for (SimplexPtrList::iterator i = simplexPtrList.begin(); i != simplexPtrList.end(); i++)
   {
       if ((*i)->size() != size)
       {
           return 0;
       }
   }
   return size;
}

void ParallelGraph::DataNode::SendMPIData(const IncidenceGraph::Params &params, int processRank)
{
#ifdef USE_MPI    
    this->processRank = processRank;
#ifdef DEBUG_MPI
        std::cout<<"process 0 packing data at "<<MPI_Wtime()<<std::endl;
#endif
    MPIData::SimplexData *data = new MPIData::SimplexData(simplexPtrList, borderVerts, params.dim, 0, GetConstantSimplexSize());
    int dataSize = data->GetSize();
    MPI_Send(&dataSize, 1, MPI_INT, processRank, MPI_MY_DATASIZE_TAG, MPI_COMM_WORLD);
#ifdef DEBUG_MPI
        std::cout<<"process 0 sending data at "<<MPI_Wtime()<<std::endl;
#endif
    MPI_Send(data->GetBuffer(), dataSize, MPI_INT, processRank, MPI_MY_WORK_TAG, MPI_COMM_WORLD);
    delete data;
#endif
}

void ParallelGraph::DataNode::SetMPIIncidenceGraphData(int* buffer, int size)
{
#ifdef USE_MPI
    this->processRank = -1;
    MPIData::IncidenceGraphData *data = new MPIData::IncidenceGraphData(buffer, size);
    this->ig = data->GetIncidenceGraph(simplexPtrList);
#ifdef DEBUG_MPI
        std::cout<<"process 0 unpacked data at "<<MPI_Wtime()<<std::endl;
#endif
    delete data;
#endif
}

void ParallelGraph::DataNode::CreateIntNodesMapWithBorderNodes()
{
    if (H.size() > 0)
    {
        // juz zbudowany
        return;
    }
    for (IncidenceGraph::Nodes::iterator node = ig->nodes.begin(); node != ig->nodes.end(); node++)
    {
        if ((*node)->IsOnBorder())
        {
            for (Simplex::iterator v = (*node)->simplex->begin(); v != (*node)->simplex->end(); v++)
            {
                H[*v].push_back(*node);
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

void ParallelGraph::DataNode::RemoveChildAndCopySimplexPtrList(SpanningTreeNode* node, SimplexPtrList& simplexPtrList)
{
    // usuwamy node z listy dzieci
    std::vector<SpanningTreeNode *>::iterator it = std::find(spanningTreeNodes.begin(), spanningTreeNodes.end(), node);
    assert(it != spanningTreeNodes.end());
    delete *it;
    spanningTreeNodes.erase(it);

    // przelatujemy cala spojna skladowa i zaznaczamy elementy do niej nalezace
    // dodatkowo dodajemy do listy sympleksy z tej spojnej skladowej
    std::queue<IncidenceGraph::Node *> L;
    L.push(node->connectedComponent);
    node->connectedComponent->IsHelperFlag2(true);
    simplexPtrList.push_back(node->connectedComponent->simplex);
    while (!L.empty())
    {
        IncidenceGraph::Node *n = L.front();
        L.pop();
        for (IncidenceGraph::Edges::iterator edge = n->edges.begin(); edge != n->edges.end(); edge++)
        {
            if (edge->node->IsHelperFlag2())
            {
                continue;
            }
            edge->node->IsHelperFlag2(true);
            L.push(edge->node);
            simplexPtrList.push_back(edge->node->simplex);
        }
    }

    // na koncu wszystkie zaznaczone node'y usuwamy z grafu
    IncidenceGraph::Nodes::iterator i = ig->nodes.begin();
    while (i != ig->nodes.end())
    {
        if ((*i)->IsHelperFlag2())
        {
            delete *i;
            i = ig->nodes.erase(i);
        }
        else
        {
            i++;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

void ParallelGraph::SpanningTreeNode::FindAcyclicSubsetToBorderConnection(Vertex borderVertex, IncidenceGraph::Path &path)
{
    if (acyclicSubsetSize > 0)
    {
        // szukanie sciezki od wierzcholka w brzegu do najbliszego acyklicznego sempleksu
        
        path = FindPath(FindNode(parent->ig->nodes, FindNodeWithVertex(borderVertex)), FindPathToNodeWithAcyclicIntersection());
        assert(path.size() > 0);
    }
    else
    {
        singleBorderVerts.push_back(borderVertex);
    }
}

////////////////////////////////////////////////////////////////////////////////

void ParallelGraph::SpanningTreeNode::UpdateAcyclicSubsetToBorderConnection(Vertex borderVertex, IncidenceGraph::Path &path)
{
    if (acyclicSubsetSize > 0)
    {
        if (isConnectedToAcyclicSubset)
        {
            // std::cout<<"is connected to acyclic subset"<<std::endl;
            UpdatePathFromBorderToAcyclicSubset(borderVertex, path);
        }
        else
        {
            // std::cout<<"connecting to acyclic subset"<<std::endl;
            UpdatePathFromAcyclicSubsetToBorder(borderVertex, path);
            isConnectedToAcyclicSubset = true;
        }
    }
    else
    {
        // std::cout<<"updating border verts: "<<singleBorderVerts.size()<<std::endl;
        UpdateBorderVerts();
        singleBorderVerts.clear(); // zeby nie wykonalo sie drugi raz
    }
}

////////////////////////////////////////////////////////////////////////////////
// w ten funkcji dodajemy sciezke do podzbioru acyklicznego zaczynajac od
// wierzcholka na brzegu. zadaniem tej funkcji jest zagwarantowanie, zeby
// podany wierzcholek z brzegu mial polaczenie ze zbiorem acyklicznym,
// nie musimy zatem dojsc do koncowego node'a na sciezce, wystarczy ze gdzies
// wczesniej natrafimy na inny kawalek podzbioru acyklicznego
// zakladamy, ze jestesmy w "kawalku" kompleksu, ktory ma juz polaczenie
// ze zbiorem acyklicznym
void ParallelGraph::SpanningTreeNode::UpdatePathFromBorderToAcyclicSubset(Vertex borderVertex, IncidenceGraph::Path &path)
{
    IncidenceGraph::Node *prevNode = path.front();
    IncidenceGraph::Path::iterator i = path.begin();
    i++;
    // jezeli zaczynamy od acyklicznego wierzcholka to jestesmy w domu
    // (musimy zagwarantowac, ze wierzcholek na brzegu bedzie mial "dojscie"
    // do zbioru acyklicznego, wiec jezeli juz w nim jest, to znaczy, ze wczesniej
    // zostal do niego dolaczony)
    if (prevNode->GetAcyclicIntersectionFlags() & (1 << prevNode->NormalizeVertex(borderVertex)))
    {
        // std::cout<<"vertex "<<borderVertex<<" is in acyclic subset -> finishing"<<std::endl;
        return;
    }
    Vertex lastVertex = borderVertex;
    VertsSet vertsOnPath;
    vertsOnPath.insert(lastVertex);
    for (; i != path.end(); i++)
    {
        Vertex vertex = GetVertexFromIntersection(prevNode->simplex, (*i)->simplex);
        if ((*i)->GetAcyclicIntersectionFlags() & (1 << (*i)->NormalizeVertex(vertex)))     
        {
            // std::cout<<"adding acyclic edge "<<lastVertex<<" " <<vertex<<" -> finishing 1"<<std::endl;
            prevNode->UpdateAcyclicIntersectionWithEdge(lastVertex, vertex);
            prevNode = 0;
            break;
        }
        else
        {
            // moze sie zdazyc, ze w pierwsze dwa sympleksy beda sasiadowaly
            // wlasnie na wierzcholku w brzegu, dlatego sprawdzamy, czy nie
            // dodajemy zdegenerowanej krawedzi
            if (lastVertex != vertex)
            {
                vertsOnPath.insert(vertex);
                prevNode->UpdateAcyclicIntersectionWithEdge(lastVertex, vertex);
                // std::cout<<"adding acyclic edge "<<lastVertex<<" " <<vertex<<std::endl;
            }
            prevNode = (*i);
            lastVertex = vertex;
            vertex = prevNode->FindAcyclicVertexNotIn(vertsOnPath);
            if (vertex != -1)
            {
                // std::cout<<"adding acyclic edge "<<lastVertex<<" " <<vertex<<" -> finishing 2"<<std::endl;
                prevNode->UpdateAcyclicIntersectionWithEdge(lastVertex, vertex);
                prevNode = 0;
                break;
            }
        }
    }
    // jezeli na liscie byl tylko jeden node
    if (prevNode != 0)
    {
        Vertex vertex = prevNode->FindAcyclicVertexNotEqual(lastVertex);
        assert(vertex != -1);
        // std::cout<<"adding acyclic edge "<<lastVertex<<" " <<vertex<<" -> finishing 3"<<std::endl;
        prevNode->UpdateAcyclicIntersectionWithEdge(lastVertex, vertex);
           
    }
}

////////////////////////////////////////////////////////////////////////////////
// ta funkcja rozni sie od poprzedniej tym, ze tutaj dodajemy wierzcholki
// do sciezki zaczynajac od podzbioru acyklicznego. zadaniem tej funkcji
// jest "podlaczenie" zbioru acyklicznego do dodanego juz innego kawalka
// podzbioru acyklicznego. jezeli zatem trafimy na podzbior acykliczny to
// albo trafilismy na wierzcholek w brzegu, ktorego podlaczenie gwarantuje
// poprzednia funkcja albo trafilismy na inny podzbior acykliczny znajdujacy
// sie w brzegu => tez ok
void ParallelGraph::SpanningTreeNode::UpdatePathFromAcyclicSubsetToBorder(Vertex borderVertex, IncidenceGraph::Path &path)
{
    path.reverse();
    IncidenceGraph::Path::iterator i = path.begin();
    // pierwszy node sasiaduje ze zbiorem acyklicznym
    IncidenceGraph::Node *prevNode = *i;
    i++;
    // pierwszy node musi zawierac wierzcholek znajdujacy sie w zbiorze
    // acyklicznym, bo takie byly warunki szukania sciezki
    Vertex lastVertex = prevNode->FindAcyclicVertexNotIn(borderVerts);
    // std::cout<<"vertex "<<lastVertex<<" is acyclic"<<std::endl;
    assert(lastVertex != -1);
    for (; i != path.end(); i++)
    {
        Vertex vertex = GetVertexFromIntersection(prevNode->simplex, (*i)->simplex);
        if ((*i)->GetAcyclicIntersectionFlags() & (1 << (*i)->NormalizeVertex(vertex)))
        {
            // std::cout<<"adding acyclic edge "<<lastVertex<<" " <<vertex<<" -> finishing 1"<<std::endl;
            prevNode->UpdateAcyclicIntersectionWithEdge(lastVertex, vertex);
            prevNode = 0;
            break;
        }
        else
        {
            // std::cout<<"adding acyclic edge "<<lastVertex<<" " <<vertex<<std::endl;
            prevNode->UpdateAcyclicIntersectionWithEdge(lastVertex, vertex);
            prevNode = (*i);
            lastVertex = vertex;
        }
    }
    // jezeli prevNode == 0 to znaczy ze trafilismy na podzbior acykliczny
    // i konczymy szukanie sciezki, wpp. musimy jeszcze wierzcholek z przeciecia
    // dwoch osatnich node'ow polaczyc z wierzcholkiem na brzegu
    if (prevNode != 0)
    {
        // moze sie zdazyc, ze w ostatnie dwa sympleksy beda sasiadowaly
        // wlasnie na wierzcholku w brzegu, dlatego sprawdzamy, czy nie
        // dodajemy zdegenerowanej krawedzi
        if (lastVertex != borderVertex)
        {
            // std::cout<<"adding acyclic edge "<<lastVertex<<" " <<borderVertex<<" -> finishing 2"<<std::endl;
            prevNode->UpdateAcyclicIntersectionWithEdge(lastVertex, borderVertex);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
// ta funkcja bierze laczy wszystkie punkty na brzegu znajdujace sie w zbiorze
// acyklicznym w drzewo. korzeniem drzewa jest pierwszy na liscie wierzcholek
// jezeli w trakcie dodawania sciezki od nowego liscia do drzewa trafimy ma
// zbior acykliczny to konczymy dodawanie sciezki => "podlaczylismy" nowy
// pozdbior acykliczny do juz utworzonego
void ParallelGraph::SpanningTreeNode::UpdateBorderVerts()
{
    // jezeli sa mniej niz dwa wierzcholki, to znaczy, ze ta czesc kompleksu
    // nie laczy zadnych zbiorow acyklicznych wiec mozna ja zignorowac
    if (singleBorderVerts.size() < 2)
    {
        // std::cout<<"less than 2 -> finishing"<<std::endl;
        return;
    }

    std::cout<<"!!!!!!!!!!!!"<<std::endl;

    std::vector<Vertex>::iterator vertex = singleBorderVerts.begin();
    Vertex firstVertex = *vertex;
    IncidenceGraph::Node *firstNode = FindNode(connectedComponent, FindNodeWithVertex(firstVertex));
    std::cout<<"first vertex: "<<firstVertex<<std::endl;
    vertex++;
    for (; vertex != singleBorderVerts.end(); vertex++)
    {
        // szukanie sciezki pomiedzy wierzcholkami
        IncidenceGraph::Path path = FindPath(firstNode, FindPathToVertex(*vertex));
        assert(path.size() > 0);
        IncidenceGraph::Node *prevNode = path.back();
        Vertex lastVertex = *vertex;
        std::cout<<"vertex: "<<lastVertex<<std::endl;
        IncidenceGraph::Path::reverse_iterator i = path.rbegin();
        i++;
        for (; i != path.rend(); i++)
        {
            Vertex vertex = GetVertexFromIntersection(prevNode->simplex, (*i)->simplex);
            if ((*i)->GetAcyclicIntersectionFlags() & (1 << (*i)->NormalizeVertex(vertex)))
            {
                std::cout<<"adding acyclic edge "<<lastVertex<<" " <<vertex<<" -> finishing"<<std::endl;
                prevNode->UpdateAcyclicIntersectionWithEdge(lastVertex, vertex);
                prevNode = 0;
                break;
            }
            else
            {
                std::cout<<"adding acyclic edge "<<lastVertex<<" " <<vertex<<std::endl;
                prevNode->UpdateAcyclicIntersectionWithEdge(lastVertex, vertex);
                prevNode = (*i);
                lastVertex = vertex;
            }
        }
        // jezeli doszlismy tutaj, to znaczy ze nie trafilismy wczesniej
        // na podzbior acykliczny i musimy polaczyc sciezke z pierwszym
        // wierzcholkiem
        if (prevNode != 0)
        {
            std::cout<<"adding acyclic edge "<<lastVertex<<" " <<firstVertex<<" -> finishing"<<std::endl;
            prevNode->UpdateAcyclicIntersectionWithEdge(lastVertex, firstVertex);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

void ParallelGraph::SpanningTreeEdge::FindAcyclicConnections()
{
    nodeA->FindAcyclicSubsetToBorderConnection(intersectionVertex, pathToA);
    nodeB->FindAcyclicSubsetToBorderConnection(intersectionVertex, pathToB);
}

void ParallelGraph::SpanningTreeEdge::UpdateAcyclicConnections()
{
    // najpierw uaktualnbiamy sciezke zbioru, ktory juz jest
    // dolaczony do drzewa
    // mamy wtedy pewnosc, ze jezeli pierwszy wierzcholek jest zaznaczony
    // jako acykliczny, to znaczy, ze zostal dodany do zbioru acyklicznego
    // juz wczesniej i mozna pod niego podpiac nowo dodawana czesc zbioru ac.

    // std::cout<<"intersection vertex: "<<intersectionVertex<<std::endl;
    if (nodeA->isConnectedToAcyclicSubset)
    {
        // Debug::Print(std::cout, pathToA);
        nodeA->UpdateAcyclicSubsetToBorderConnection(intersectionVertex, pathToA);
        // Debug::Print(std::cout, pathToB);
        nodeB->UpdateAcyclicSubsetToBorderConnection(intersectionVertex, pathToB);
    }
    else
    {
        // Debug::Print(std::cout, pathToB);
        nodeB->UpdateAcyclicSubsetToBorderConnection(intersectionVertex, pathToB);
        // Debug::Print(std::cout, pathToA);
        nodeA->UpdateAcyclicSubsetToBorderConnection(intersectionVertex, pathToA);
    }
}

////////////////////////////////////////////////////////////////////////////////

ParallelGraph::ParallelGraph(IncidenceGraph *ig, SimplexList &simplexList, IncidenceGraph::Params params, IncidenceGraph::ParallelParams parallelParams, AcyclicTest<IncidenceGraph::IntersectionFlags> *test, bool local)
{
    incidenceGraph = ig;
#ifdef USE_MPI
    this->local = local;
#else
    this->local = true;
#endif
    int packSize = parallelParams.packSize;
    if (parallelParams.packsCount != -1)
    {
        packSize = (int)ceil(float(simplexList.size()) / parallelParams.packsCount);
    }
    std::cout<<"pack size: "<<packSize<<std::endl;
    if (parallelParams.prepareData)
    {
        PrepareData(simplexList, packSize);
        Timer::Update("preparing data");
    }
    DivideData(simplexList, packSize);
    Timer::Update("dividing data");
    CreateDataEdges();
    Timer::Update("creating data connections");
    CalculateIncidenceGraphs(dataNodes, params, test);
    Timer::Update("creating incidence graphs");
    CreateSpanningTree();
    CalculateIncidenceGraphs(secondPhaseDataNodes, params, test);
    Timer::Update("creating second phase incidence graphs");
    CombineGraphs();
}

////////////////////////////////////////////////////////////////////////////////

ParallelGraph::~ParallelGraph()
{
    for (DataNodes::iterator i = dataNodes.begin(); i != dataNodes.end(); i++)
    {
        delete (*i);
    }
    for (DataEdges::iterator i = dataEdges.begin(); i != dataEdges.end(); i++)
    {
        delete (*i);
    }
    for (DataNodes::iterator i = secondPhaseDataNodes.begin(); i != secondPhaseDataNodes.end(); i++)
    {
        delete (*i);
    }
    for (SpanningTreeNodes::iterator i = spanningTreeNodes.begin(); i != spanningTreeNodes.end(); i++)
    {
        delete (*i);
    }
    for (SpanningTreeEdges::iterator i = spanningTreeEdges.begin(); i != spanningTreeEdges.end(); i++)
    {
        delete (*i);
    }
}

////////////////////////////////////////////////////////////////////////////////

struct SimplexDescriptor
{
    Simplex simplex;
    bool added;

    SimplexDescriptor()
    {
        added = false;
    }
};

void ParallelGraph::PrepareData(SimplexList &simplexList, int packSize)
{
    int count = simplexList.size();
    SimplexDescriptor *descriptors = new SimplexDescriptor[count];
    int index = 0;
    for (SimplexList::iterator i = simplexList.begin(); i != simplexList.end(); i++)
    {
        descriptors[index++].simplex = *i;
    }
    simplexList.clear();

    std::map<Vertex, std::vector<SimplexDescriptor *> > H;
    for (int i = 0; i < count; i++)
    {
        Simplex &s = descriptors[i].simplex;
        for (Simplex::iterator v = s.begin(); v != s.end(); v++)
        {
            H[*v].push_back(&descriptors[i]);
        }
    }

    index = 0;
    for (int i = 0; i < count; i++)
    {
        if (descriptors[i].added)
        {
            continue;
        }
        std::queue<SimplexDescriptor *> L;
        L.push(&descriptors[i]);
        descriptors[i].added = true;
        while (!L.empty())
        {
            SimplexDescriptor *sd = L.front();
            L.pop();
            simplexList.push_back(sd->simplex);
            for (Simplex::iterator v = sd->simplex.begin(); v != sd->simplex.end(); v++)
            {
                std::vector<SimplexDescriptor *> neighbours = H[*v];
//                if (neighbours.size() > 20)
//                {
//                    continue;
//                }
                for (std::vector<SimplexDescriptor *>::iterator n = neighbours.begin(); n != neighbours.end(); n++)
                {
                    if (!(*n)->added)
                    {
                        L.push(*n);
                        (*n)->added = true;
                    }                    
                }
            }
        }
    }
    delete [] descriptors;
}

void ParallelGraph::DivideData(SimplexList& simplexList, int packSize)
{
    // test!!!
    // SimplexList tempSimplexList;

    DataNode *currentNode =  new DataNode();
    int simplicesLeft = packSize;
    SimplexList::iterator it = simplexList.begin();

    while (it != simplexList.end())
    {
        // tempSimplexList.push_back(*it);

        currentNode->simplexPtrList.push_back(&(*it));
        for (Simplex::const_iterator i = it->begin(); i != it->end(); i++)
        {
            currentNode->verts.insert(*i);
        }
        simplicesLeft--;
        if (simplicesLeft == 0)
        {
            dataNodes.push_back(currentNode);
            currentNode = new DataNode();
            simplicesLeft = packSize;
        }
        it++;

        // test!!!
        // if (tempSimplexList.size() > 7500) break;
    }
    if (currentNode->simplexPtrList.size() > 0)
    {
        dataNodes.push_back(currentNode);
    }

     // std::cout<<"temp simplex list ("<<tempSimplexList.size()<<") homology:"<<std::endl;
     // Tests::Test(tempSimplexList, RT_Coreduction);
}

////////////////////////////////////////////////////////////////////////////////

void ParallelGraph::CreateDataEdges()
{
    for (DataNodes::iterator i = dataNodes.begin(); i != dataNodes.end(); i++)
    {
        for (DataNodes::iterator j = i + 1; j != dataNodes.end(); j++)
        {
            Simplex intersection;
            GetSortedIntersectionOfUnsortedSets(intersection, (*i)->verts, (*j)->verts);
            if (intersection.size() > 0)
            {
                DataEdge *edge = new DataEdge(*i, *j);
                dataEdges.push_back(edge);
                (*i)->AddEdge(edge, intersection);
                (*j)->AddEdge(edge, intersection);
            }
        }
    }

    for (DataNodes::iterator i = dataNodes.begin(); i != dataNodes.end(); i++)
    {
        DataNode *node = *i;
        std::cout<<"border verts: "<<node->borderVerts.size()<<" verts size: "<<node->verts.size()<<std::endl;
    }
}

////////////////////////////////////////////////////////////////////////////////

ParallelGraph::DataNode *ParallelGraph::GetNodeWithProcessRank(DataNodes &sourceNodes, int processRank)
{
    for (DataNodes::iterator i = sourceNodes.begin(); i != sourceNodes.end(); i++)
    {
        if ((*i)->processRank == processRank)
        {
            return (*i);
        }
    }
    return 0;
}

void ParallelGraph::CalculateIncidenceGraphs(DataNodes &sourceNodes, const IncidenceGraph::Params &params, AcyclicTest<IncidenceGraph::IntersectionFlags> *test)
{
    if (local)
    {
        for (DataNodes::iterator i = sourceNodes.begin(); i != sourceNodes.end(); i++)
        {
            (*i)->CreateIncidenceGraphLocally(params, test);
        }
    }
#ifdef USE_MPI
    else
    {
        int nodesCount = sourceNodes.size();
        int currentNode = 0;
        int tasksCount;
        int dataSize;
        MPI_Status status;

        MPI_Comm_size(MPI_COMM_WORLD, &tasksCount);

        int size = (tasksCount < nodesCount) ? tasksCount : nodesCount;

        // dopoki starczy nam node'ow wysylamy paczki
        for (int rank = 1; rank < size; rank++)
        {
            std::cout<<"sending node "<<currentNode<<" to process: "<<rank<<std::endl;
            sourceNodes[currentNode++]->SendMPIData(params, rank);
        }

        // potem czekamy na dane i wysylamy kolejne
        while (currentNode < nodesCount)
        {
            // najpierw pobieramy rozmiar danych
            MPI_Recv(&dataSize, 1, MPI_INT, MPI_ANY_SOURCE, MPI_MY_DATASIZE_TAG, MPI_COMM_WORLD, &status);
            int *buffer = new int[dataSize];
#ifdef DEBUG_MEMORY
            MemoryInfo::Alloc(dataSize);
#endif
            // teraz dane od node'a od ktorego dostalismy info o rozmiarze danych
            MPI_Recv(buffer, dataSize, MPI_INT, status.MPI_SOURCE, MPI_MY_DATA_TAG, MPI_COMM_WORLD, &status);
#ifdef DEBUG_MPI
        std::cout<<"process 0 received data at "<<MPI_Wtime()<<std::endl;
#endif
            GetNodeWithProcessRank(sourceNodes, status.MPI_SOURCE)->SetMPIIncidenceGraphData(buffer, size);
            std::cout<<"sending node "<<currentNode<<" to process: "<<status.MPI_SOURCE<<std::endl;
            sourceNodes[currentNode++]->SendMPIData(params, status.MPI_SOURCE);
        }

        // na koncu odbieramy to co jeszcze jest liczone
        for (int rank = 1; rank < size; ++rank)
        {
            // najpierw pobieramy rozmiar danych
            MPI_Recv(&dataSize, 1, MPI_INT, MPI_ANY_SOURCE, MPI_MY_DATASIZE_TAG, MPI_COMM_WORLD, &status);
            int *buffer = new int[dataSize];
#ifdef DEBUG_MEMORY
            MemoryInfo::Alloc(dataSize);
#endif
            // teraz dane od node'a od ktorego dostalismy info o rozmiarze danych
            MPI_Recv(buffer, dataSize, MPI_INT, status.MPI_SOURCE, MPI_MY_DATA_TAG, MPI_COMM_WORLD, &status);
#ifdef DEBUG_MPI
            std::cout<<"process 0 received data at "<<MPI_Wtime()<<std::endl;
#endif
            GetNodeWithProcessRank(sourceNodes, status.MPI_SOURCE)->SetMPIIncidenceGraphData(buffer, size);
        }

        // konczymy prace procesow, tylko jezeli pracowalismy na secondPhaseDataNodes
        if (sourceNodes == secondPhaseDataNodes)
        {
            std::cout<<"terminating jobs"<<std::endl;

            // wysylamy informacje do node'ow o zakonczeniu pracy
            for (int rank = 1; rank < tasksCount; ++rank)
            {
                MPI_Send(0, 0, MPI_INT, rank, MPI_MY_DIE_TAG, MPI_COMM_WORLD);
            }

            std::cout<<"parallel computing done"<<std::endl;
        }
    }
#endif
}

////////////////////////////////////////////////////////////////////////////////

void ParallelGraph::CreateSpanningTree()
{
    // budujemy graf, w ktorym wierzcholkami beda podzbiory acykliczne
    // (czyli de facto skladowe spojne poszczegolnych paczek), ktore potem
    // polaczymy drzewem rozpinajacym
    int currentID = 1;
    for (DataNodes::iterator i = dataNodes.begin(); i != dataNodes.end(); i++)
    {
        IncidenceGraph *ig = (*i)->ig;
        std::vector<std::set<Vertex> >::iterator ccb = ig->connectedComponentsBorders.begin();
        std::vector<int>::iterator ccass = ig->connectedComponentsAcyclicSubsetSize.begin();
        for (IncidenceGraph::ConnectedComponents::iterator cc = ig->connectedComponents.begin(); cc != ig->connectedComponents.end(); cc++)
        {
            SpanningTreeNode *newNode = new SpanningTreeNode(*i, currentID++, *cc, *ccb, *ccass);
            (*i)->spanningTreeNodes.push_back(newNode);
            spanningTreeNodes.push_back(newNode);
            ccb++;
            ccass++;
        }
    }

    Timer::Update("creating acyclic tree nodes");

    // tworzymy krawedzie w grafie
    for (SpanningTreeNodes::iterator node = spanningTreeNodes.begin(); node != spanningTreeNodes.end(); node++)
    {
        DataNode *parent = (*node)->parent;
        std::vector<SpanningTreeNode *> potentialNeighbours;
        for (DataEdges::iterator edge = parent->edges.begin(); edge != parent->edges.end(); edge++)
        {
            DataNode *neighbour = ((*edge)->nodeA == parent) ? (*edge)->nodeB : (*edge)->nodeA;
            potentialNeighbours.insert(potentialNeighbours.end(), neighbour->spanningTreeNodes.begin(), neighbour->spanningTreeNodes.end());
        }
        for (SpanningTreeNodes::iterator neighbour = potentialNeighbours.begin(); neighbour != potentialNeighbours.end(); neighbour++)
        {
            if ((*neighbour)->subtreeID <= (*node)->subtreeID)
            {
                continue;
            }
            Simplex intersection;
            GetSortedIntersectionOfUnsortedSets(intersection, (*node)->borderVerts, (*neighbour)->borderVerts);
            if (intersection.size() > 0)
            {
                SpanningTreeEdge *edge = new SpanningTreeEdge(*node, *neighbour, intersection.front());
                spanningTreeEdges.push_back(edge);
                (*node)->AddEdge(edge);
                (*neighbour)->AddEdge(edge);
            }
        }
    }

    Timer::Update("creating acyclic tree edges");

    // tworzymy hasha, w ktorym dla kazdego ID poddrzewa bedziemy przechowywali
    // rozmiar podzbioru acyklicznego w nim zawartego (w poddrzewie)
    std::map<int, int> spanningTreeAcyclicSubsetSize;
    for (SpanningTreeNodes::iterator node = spanningTreeNodes.begin(); node != spanningTreeNodes.end(); node++)
    {
        spanningTreeAcyclicSubsetSize[(*node)->subtreeID] = (*node)->acyclicSubsetSize;
    }

    // i na koncu drzewo rozpinajace
    for (SpanningTreeEdges::iterator edge = spanningTreeEdges.begin(); edge != spanningTreeEdges.end(); edge++)
    {
        if ((*edge)->nodeA->subtreeID == (*edge)->nodeB->subtreeID)
        {
            continue;
        }
        (*edge)->isInSpanningTree = true;
        int newID = (*edge)->nodeA->subtreeID;
        int oldID = (*edge)->nodeB->subtreeID;
        // przy laczeniu poddrzew sumujemy rozmiary podzbiorow acyklicznych
        // w nich zawartych
        spanningTreeAcyclicSubsetSize[newID] = spanningTreeAcyclicSubsetSize[newID] + spanningTreeAcyclicSubsetSize[oldID];
        for (SpanningTreeNodes::iterator node = spanningTreeNodes.begin(); node != spanningTreeNodes.end(); node++)
        {
            if ((*node)->subtreeID == oldID)
            {
                (*node)->subtreeID = newID;
            }
        }
    }

    Timer::Update("creating spanning tree");

    // jezeli zostalo jakies poddrzewo, ktore nie zawiera podzbioru acyklicznego
    // to usuwamy wszystkie sympleksy z utworzonego grafu, dodajemy do wspolnej
    // listy, a nastepnie traktujemy jako jedna spojna skladowa i jeszcze raz
    // wysylamy do obliczen, ale tym razem nie laczymy juz brzegow, tylko po
    // prostu "przenosimy" nowo utworzony graf jako osobna spojna skladowa
    // duzego grafu
    std::map<int, SimplexPtrList> simplexPtrLists;
    for (SpanningTreeNodes::iterator node = spanningTreeNodes.begin(); node != spanningTreeNodes.end(); node++)
    {
        if (spanningTreeAcyclicSubsetSize[(*node)->subtreeID] == 0)
        {
            (*node)->parent->RemoveChildAndCopySimplexPtrList(*node, simplexPtrLists[(*node)->subtreeID]);
        }
    }

    // dla kazdej listy sympleksow, ktora otrzymalismy w poprzednim kroku
    // tworzymy osobny data node (nie interesuje nas zupelnie jego brzeg,
    // poniewaz wiemy, ze jest to oddzielna spojna skladowa i nie sasiaduje
    // z innymi sympleksami)
    for (std::map<int, SimplexPtrList>::iterator i = simplexPtrLists.begin(); i != simplexPtrLists.end(); i++)
    {
        std::cout<<"creating second phase data node with "<<i->second.size()<<" simplices"<<std::endl;
        DataNode *newNode = new DataNode();
        newNode->simplexPtrList = i->second;
        secondPhaseDataNodes.push_back(newNode);
    }

    Timer::Update("finding second phase data nodes");
}

////////////////////////////////////////////////////////////////////////////////

void ParallelGraph::CombineGraphs()
{
    // na drzewie rozpinajacym
    // szukamny polaczen acyklicznych zbiorow, ale na razie ich nie laczymy
    // bedziemy robic to po polaczeniu wszystkich sympleksow, zeby dobrze
    // zaktualizowac podzbior acykliczny
    for (SpanningTreeEdges::iterator i = spanningTreeEdges.begin(); i != spanningTreeEdges.end(); i++)
    {
        if ((*i)->isInSpanningTree) (*i)->FindAcyclicConnections();
    }

    Timer::Update("searching paths from acyclic subsets to border");
    Timer::Time timeStart = Timer::Now();

    float creatingMapsTime = 0;
    float connectingSimplicesTime = 0;
    int neighboursCount = 0;
    int nodesCount = 0;

    // laczymy sympleksy
    for (DataEdges::iterator edge = dataEdges.begin(); edge != dataEdges.end(); edge++)
    {
        IncidenceGraph::Nodes nodesA = (*edge)->nodeA->ig->nodes;
        (*edge)->nodeB->CreateIntNodesMapWithBorderNodes();
        IncidenceGraph::VertexNodesMap HB = (*edge)->nodeB->H;

        creatingMapsTime += Timer::Update();

        for (IncidenceGraph::Nodes::iterator node = nodesA.begin(); node != nodesA.end(); node++)
        {
            if (!(*node)->IsOnBorder())
            {
                continue;
            }
            for (Simplex::iterator v = (*node)->simplex->begin(); v != (*node)->simplex->end(); v++)
            {
                IncidenceGraph::Nodes neighbours = HB[*v];
                if (neighbours.size() == 0)
                {
                    continue;
                }
                neighboursCount += neighbours.size();
                nodesCount++;
                for (IncidenceGraph::Nodes::iterator neighbour = neighbours.begin(); neighbour != neighbours.end(); neighbour++)
                {                    
                    if (!(*node)->HasNeighbour(*neighbour))
                    {
                        (*node)->AddNeighbour(*neighbour);
                        (*neighbour)->AddNeighbour(*node);
                    }
                }
            }
        }
        connectingSimplicesTime += Timer::Update();
    }

    Timer::TimeFrom(timeStart, "connecting simplices on border");
    std::cout<<"creating maps: "<<creatingMapsTime<<std::endl;
    std::cout<<"connecting simplices: "<<connectingSimplicesTime<<std::endl;
    if (nodesCount > 0)
    {
        std::cout<<"avg neighbours count: "<<neighboursCount / nodesCount<<std::endl;
    }
    Timer::Update();

    // przenosimy wszystkie sympleksy do jednego grafu
    for (DataNodes::iterator i = dataNodes.begin(); i != dataNodes.end(); i++)
    {
         incidenceGraph->nodes.insert(incidenceGraph->nodes.end(), (*i)->ig->nodes.begin(), (*i)->ig->nodes.end());
        (*i)->ig->nodes.clear();
    }

    // to samo ze spojnymi skladowymi z drugiej fazy obliczen
    for (DataNodes::iterator i = secondPhaseDataNodes.begin(); i != secondPhaseDataNodes.end(); i++)
    {
         incidenceGraph->nodes.insert(incidenceGraph->nodes.end(), (*i)->ig->nodes.begin(), (*i)->ig->nodes.end());
        (*i)->ig->nodes.clear();
    }

    std::cout<<"total simplices after connecting graphs: "<<incidenceGraph->nodes.size()<<std::endl;

    Timer::Update("moving simplices to single incidence graph");

    // aktualizujemy zbior acykliczny o polaczenia pomiedzy podgrafami
    for (SpanningTreeEdges::iterator i = spanningTreeEdges.begin(); i != spanningTreeEdges.end(); i++)
    {
        if ((*i)->isInSpanningTree) (*i)->UpdateAcyclicConnections();
    }

    Timer::Update("adding paths to acyclic subset");
}

////////////////////////////////////////////////////////////////////////////////

void ParallelGraph::MPISlave(int processRank)
{
#ifdef USE_MPI

    MPI_Status status;
    int dataSize;

    while (1)
    {

        // pobieramy pierwszy komunikat
        MPI_Recv(&dataSize, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        // jezeli polecenie zakonczenia pracy to konczymy
        if (status.MPI_TAG == MPI_MY_DIE_TAG)
        {
            return;
        }

        // wpp. musi to byc MPI_MY_DATASIZE_TAG
        assert(status.MPI_TAG == MPI_MY_DATASIZE_TAG);

        // pobieramy bufor z danymi
        int *buffer = new int[dataSize];
#ifdef DEBUG_MEMORY
        MemoryInfo::Alloc(dataSize);
#endif
        MPI_Recv(buffer, dataSize, MPI_INT, 0, MPI_MY_WORK_TAG, MPI_COMM_WORLD, &status);
#ifdef DEBUG_MPI
        std::cout<<"process "<<processRank<<" received data at "<<MPI_Wtime()<<std::endl;
#endif

        // z pobranego bufora budujemy dane wejsciowe
        MPIData::SimplexData *data = new MPIData::SimplexData(buffer, dataSize);
        SimplexList simplexList;
        std::set<Vertex> borderVerts;
        IncidenceGraph::Params params;
        data->GetSimplexData(simplexList, borderVerts, params.dim, params.acyclicTestNumber);
#ifdef DEBUG_MPI
        std::cout<<"process "<<processRank<<" upacked data at "<<MPI_Wtime()<<std::endl;
#endif
        AcyclicTest<IncidenceGraph::IntersectionFlags> *test = AcyclicTest<IncidenceGraph::IntersectionFlags>::Create(params.acyclicTestNumber, params.dim);

        // tworzymy graf incydencji z policzonym podzbiorem acyklicznym
        IncidenceGraph *ig = IncidenceGraph::CreateWithBorderVerts(simplexList, borderVerts, params);
        ig->CalculateAcyclicSubsetWithSpanningTree(test);
        ig->RemoveAcyclicSubset();

        delete data;
        delete test;

        // zamieniamy na bufor danych
        MPIData::IncidenceGraphData *igData = new MPIData::IncidenceGraphData(ig);
#ifdef DEBUG_MPI
        std::cout<<"process "<<processRank<<" packing data at "<<MPI_Wtime()<<std::endl;
#endif
        dataSize = igData->GetSize();

#ifdef DEBUG_MPI
        std::cout<<"process "<<processRank<<" sending data at "<<MPI_Wtime()<<std::endl;
#endif
        // i odsylamy do mastera
        MPI_Send(&dataSize, 1, MPI_INT, 0, MPI_MY_DATASIZE_TAG, MPI_COMM_WORLD);
        MPI_Send(igData->GetBuffer(), dataSize, MPI_INT,0, MPI_MY_DATA_TAG, MPI_COMM_WORLD);

        delete igData;
        delete ig;
    }

#endif
}

////////////////////////////////////////////////////////////////////////////////
// eof
