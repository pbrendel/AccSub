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

#ifdef USE_MPI
#include <mpi.h>
#include "MPIData.h"
#endif

////////////////////////////////////////////////////////////////////////////////

void ParallelGraph::DataNode::CreateIncidenceGraphLocally(const IncidenceGraph::Params &params, AcyclicTest<IncidenceGraph::IntersectionFlags> *test)
{
    for (SimplexPtrList::iterator i = simplexPtrList.begin(); i != simplexPtrList.end(); i++)
    {
        localSimplexList.push_back(*(*i));
    }
    ig = IncidenceGraph::CreateWithBorderVerts(localSimplexList, borderVerts, params);
    ig->CalculateAcyclicSubsetWithSpanningTree(test);
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
    MPIData::SimplexData *data = new MPIData::SimplexData(simplexPtrList, borderVerts, params.dim, 0, GetConstantSimplexSize());
    int dataSize = data->GetSize();
    MPI_Send(&dataSize, 1, MPI_INT, processRank, MPI_MY_DATASIZE_TAG, MPI_COMM_WORLD);
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

void ParallelGraph::AcyclicTreeNode::FindAcyclicSubsetToBorderConnection(Vertex borderVertex, IncidenceGraph::Path &path)
{
    if (acyclicSubsetSize > 0)
    {
        // szukanie sciezki od wierzcholka w brzegu do najbliszego acyklicznego sempleksu
        path = FindPath(FindNode(connectedComponent, FindNodeWithVertex(borderVertex)), connectedComponent, FindPathToAcyclicNode());
        assert(path.size() > 0);
    }
    else
    {
        singleBorderVerts.push_back(borderVertex);
    }
}

////////////////////////////////////////////////////////////////////////////////

void ParallelGraph::AcyclicTreeNode::UpdateAcyclicSubsetToBorderConnection(Vertex borderVertex, IncidenceGraph::Path &path)
{
    if (acyclicSubsetSize > 0)
    {
        if (isConnectedToAcyclicSubset)
        {
            UpdatePathFromBorderToAcyclicSubset(borderVertex, path);
        }
        else
        {
            UpdatePathFromAcyclicSubsetToBorder(borderVertex, path);
            isConnectedToAcyclicSubset = true;
        }
    }
    else
    {
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
void ParallelGraph::AcyclicTreeNode::UpdatePathFromBorderToAcyclicSubset(Vertex borderVertex, IncidenceGraph::Path &path)
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
        return;
    }
    Vertex lastVertex = borderVertex;
    for (; i != path.end(); i++)
    {
        Vertex vertex = GetVertexFromIntersection(prevNode->simplex, (*i)->simplex);
        if ((*i)->GetAcyclicIntersectionFlags() & (1 << (*i)->NormalizeVertex(vertex)))
        {
            prevNode->UpdateAcyclicIntersectionWithEdge(lastVertex, vertex);
            break;
        }
        else
        {
            prevNode->UpdateAcyclicIntersectionWithEdge(lastVertex, vertex);
            prevNode = (*i);
            lastVertex = vertex;
        }
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
void ParallelGraph::AcyclicTreeNode::UpdatePathFromAcyclicSubsetToBorder(Vertex borderVertex, IncidenceGraph::Path &path)
{
    path.reverse();
    // pierwszy node jest ze zbioru acyklicznego
    IncidenceGraph::Node *prevNode = path.front();
    IncidenceGraph::Path::iterator i = path.begin();
    i++;
    // ten wierzcholek MUSI byc w zbiorze acyklicznym, skoro otrzymalismy
    // go z przeciecia acyklicznego wierzcholka z innym
    Vertex lastVertex = GetVertexFromIntersection(prevNode->simplex, (*i)->simplex);
    assert((*i)->GetAcyclicIntersectionFlags() & (1 << (*i)->NormalizeVertex(lastVertex)));
    prevNode = *i;
    i++;
    for (; i != path.end(); i++)
    {
        Vertex vertex = GetVertexFromIntersection(prevNode->simplex, (*i)->simplex);
        if ((*i)->GetAcyclicIntersectionFlags() & (1 << (*i)->NormalizeVertex(vertex)))
        {
            prevNode->UpdateAcyclicIntersectionWithEdge(lastVertex, vertex);
            prevNode = 0;
            break;
        }
        else
        {
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
        prevNode->UpdateAcyclicIntersectionWithEdge(lastVertex, borderVertex);
    }
}

////////////////////////////////////////////////////////////////////////////////
// ta funkcja bierze laczy wszystkie punkty na brzegu znajdujace sie w zbiorze
// acyklicznym w drzewo. korzeniem drzewa jest pierwszy na liscie wierzcholek
// jezeli w trakcie dodawania sciezki od nowego liscia do drzewa trafimy ma
// zbior acykliczny to konczymy dodawanie sciezki => "podlaczylismy" nowy
// pozdbior acykliczny do juz utworzonego
void ParallelGraph::AcyclicTreeNode::UpdateBorderVerts()
{
    // jezeli sa mniej niz dwa wierzcholki, to znaczy, ze ta czesc kompleksu
    // nie laczy zadnych zbiorow acyklicznych wiec mozna ja zignorowac
    if (singleBorderVerts.size() < 2)
    {
        return;
    }
    std::vector<Vertex>::iterator vertex = singleBorderVerts.begin();
    Vertex firstVertex = *vertex;
    vertex++;
    for (; vertex != singleBorderVerts.end(); vertex++)
    {
        // szukanie sciezki pomiedzy wierzcholkami
        IncidenceGraph::Path path = FindPath(FindNode(connectedComponent, FindNodeWithVertex(firstVertex)), connectedComponent, FindPathToNode(FindNode(connectedComponent, FindNodeWithVertex(*vertex))));
        assert(path.size() > 0);
        IncidenceGraph::Node *prevNode = path.back();
        Vertex lastVertex = *vertex;
        IncidenceGraph::Path::reverse_iterator i = path.rbegin();
        i++;
        for (; i != path.rend(); i++)
        {
            Vertex vertex = GetVertexFromIntersection(prevNode->simplex, (*i)->simplex);
            if ((*i)->GetAcyclicIntersectionFlags() & (1 << (*i)->NormalizeVertex(vertex)))
            {
                prevNode->UpdateAcyclicIntersectionWithEdge(lastVertex, vertex);
                prevNode = 0;
                break;
            }
            else
            {
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
            prevNode->UpdateAcyclicIntersectionWithEdge(lastVertex, firstVertex);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

void ParallelGraph::AcyclicTreeEdge::FindAcyclicConnections()
{
    nodeA->FindAcyclicSubsetToBorderConnection(intersectionVertex, pathToA);
    nodeB->FindAcyclicSubsetToBorderConnection(intersectionVertex, pathToB);
}

void ParallelGraph::AcyclicTreeEdge::UpdateAcyclicConnections()
{
    nodeA->UpdateAcyclicSubsetToBorderConnection(intersectionVertex, pathToA);
    nodeB->UpdateAcyclicSubsetToBorderConnection(intersectionVertex, pathToB);
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
    DivideData(simplexList, parallelParams.packSize);
    CreateDataEdges();
    CalculateIncidenceGraphs(params, test);
    CreateAcyclicTree();
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
    for (AcyclicTreeNodes::iterator i = acyclicTreeNodes.begin(); i != acyclicTreeNodes.end(); i++)
    {
        delete (*i);
    }
    for (AcyclicTreeEdges::iterator i = acyclicTreeEdges.begin(); i != acyclicTreeEdges.end(); i++)
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
                if (neighbours.size() > 20)
                {
                    continue;
                }
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
    // PrepareData(simplexList, packSize);
    DataNode *currentNode =  new DataNode();
    int simplicesLeft = packSize;
    SimplexList::iterator it = simplexList.begin();
    while (it != simplexList.end())
    {
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
    }
    if (currentNode->simplexPtrList.size() > 0)
    {
        dataNodes.push_back(currentNode);
    }
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
//
//    for (DataNodes::iterator i = dataNodes.begin(); i != dataNodes.end(); i++)
//    {
//        DataNode *node = *i;
//        std::cout<<"node size "<<node->simplexList.size()<<" border verts: "<<node->borderVerts.size()<<" verts size: "<<node->verts.size()<<std::endl;
//    }
}

////////////////////////////////////////////////////////////////////////////////

ParallelGraph::DataNode *ParallelGraph::GetNodeWithProcessRank(int processRank)
{
    for (DataNodes::iterator i = dataNodes.begin(); i != dataNodes.end(); i++)
    {
        if ((*i)->processRank == processRank)
        {
            return (*i);
        }
    }
    return 0;
}

void ParallelGraph::CalculateIncidenceGraphs(const IncidenceGraph::Params &params, AcyclicTest<IncidenceGraph::IntersectionFlags> *test)
{
    if (local)
    {
        for (DataNodes::iterator i = dataNodes.begin(); i != dataNodes.end(); i++)
        {
            (*i)->CreateIncidenceGraphLocally(params, test);
        }
    }
#ifdef USE_MPI
    else
    {
        int nodesCount = dataNodes.size();
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
            dataNodes[currentNode++]->SendMPIData(params, rank);
        }

        // potem czekamy na dane i wysylamy kolejne
        while (currentNode < nodesCount)
        {
            // najpierw pobieramy rozmiar danych
            MPI_Recv(&dataSize, 1, MPI_INT, MPI_ANY_SOURCE, MPI_MY_DATASIZE_TAG, MPI_COMM_WORLD, &status);
            int *buffer = new int[dataSize];
            // teraz dane od node'a od ktorego dostalismy info o rozmiarze danych
            MPI_Recv(buffer, dataSize, MPI_INT, status.MPI_SOURCE, MPI_MY_DATA_TAG, MPI_COMM_WORLD, &status);
            GetNodeWithProcessRank(status.MPI_SOURCE)->SetMPIIncidenceGraphData(buffer, size);
            std::cout<<"sending node "<<currentNode<<" to process: "<<status.MPI_SOURCE<<std::endl;
            dataNodes[currentNode++]->SendMPIData(params, status.MPI_SOURCE);
        }

        // na koncu odbieramy to co jeszcze jest liczone
        for (int rank = 1; rank < size; ++rank)
        {
            // najpierw pobieramy rozmiar danych
            MPI_Recv(&dataSize, 1, MPI_INT, MPI_ANY_SOURCE, MPI_MY_DATASIZE_TAG, MPI_COMM_WORLD, &status);
            int *buffer = new int[dataSize];
            // teraz dane od node'a od ktorego dostalismy info o rozmiarze danych
            MPI_Recv(buffer, dataSize, MPI_INT, status.MPI_SOURCE, MPI_MY_DATA_TAG, MPI_COMM_WORLD, &status);
            GetNodeWithProcessRank(status.MPI_SOURCE)->SetMPIIncidenceGraphData(buffer, size);
        }

        std::cout<<"terminating jobs"<<std::endl;

        // na koncu wysylamy informacje do node'ow o zakonczeniu pracy
        for (int rank = 1; rank < tasksCount; ++rank)
        {
            MPI_Send(0, 0, MPI_INT, rank, MPI_MY_DIE_TAG, MPI_COMM_WORLD);
        }

        std::cout<<"parallel computing done"<<std::endl;
    }
#endif
}

////////////////////////////////////////////////////////////////////////////////

void ParallelGraph::CreateAcyclicTree()
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
            acyclicTreeNodes.push_back(new AcyclicTreeNode(currentID++, *cc, *ccb, *ccass));
            ccb++;
            ccass++;
        }
    }

    // tworzymy krawedzie w grafie
    for (AcyclicTreeNodes::iterator i = acyclicTreeNodes.begin(); i != acyclicTreeNodes.end(); i++)
    {
        for (AcyclicTreeNodes::iterator j = i + 1; j != acyclicTreeNodes.end(); j++)
        {
            Simplex intersection;
            GetSortedIntersectionOfUnsortedSets(intersection, (*i)->borderVerts, (*j)->borderVerts);
            if (intersection.size() > 0)
            {
                AcyclicTreeEdge *edge = new AcyclicTreeEdge(*i, *j, intersection.front());
                acyclicTreeEdges.push_back(edge);
                (*i)->AddEdge(edge);
                (*j)->AddEdge(edge);
            }
        }
    }

    // i na koncu drzewo rozpinajace
    for (AcyclicTreeEdges::iterator edge = acyclicTreeEdges.begin(); edge != acyclicTreeEdges.end(); edge++)
    {
        if ((*edge)->nodeA->acyclicID == (*edge)->nodeB->acyclicID)
        {
            continue;
        }
        (*edge)->isAcyclic = true;
        int newID = (*edge)->nodeA->acyclicID;
        int oldID = (*edge)->nodeB->acyclicID;
        for (AcyclicTreeNodes::iterator node = acyclicTreeNodes.begin(); node != acyclicTreeNodes.end(); node++)
        {
            if ((*node)->acyclicID == oldID)
            {
                (*node)->acyclicID = newID;
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

void ParallelGraph::CombineGraphs()
{
    // na drzewie rozpinajacym
    // szukamny polaczen acyklicznych zbiorow, ale na razie ich nie laczymy
    // bedziemy robic to po polaczeniu wszystkich sympleksow, zeby dobrze
    // zaktualizowac podzbior acykliczny
    for (AcyclicTreeEdges::iterator i = acyclicTreeEdges.begin(); i != acyclicTreeEdges.end(); i++)
    {
        if ((*i)->isAcyclic) (*i)->FindAcyclicConnections();
    }

    // laczymy sympleksy
    for (DataEdges::iterator edge = dataEdges.begin(); edge != dataEdges.end(); edge++)
    {
        IncidenceGraph::Nodes nodesA = (*edge)->nodeA->ig->nodes;
        (*edge)->nodeB->CreateIntNodesMapWithBorderNodes();
        IncidenceGraph::IntNodesMap HB = (*edge)->nodeB->H;

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
    }

    // przenosimy wszystkie sympleksy do jednego grafu
    for (DataNodes::iterator i = dataNodes.begin(); i != dataNodes.end(); i++)
    {
         incidenceGraph->nodes.insert(incidenceGraph->nodes.begin(), (*i)->ig->nodes.begin(), (*i)->ig->nodes.end());
        (*i)->ig->nodes.clear();
    }

    // aktualizujemy zbior acykliczny o polaczenia pomiedzy podgrafami
    for (AcyclicTreeEdges::iterator i = acyclicTreeEdges.begin(); i != acyclicTreeEdges.end(); i++)
    {
        if ((*i)->isAcyclic) (*i)->UpdateAcyclicConnections();
    }
}

////////////////////////////////////////////////////////////////////////////////
// eof
