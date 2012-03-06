/*
 * File:   AccSpanningTree.hpp
 * Author: Piotr Brendel
 *         piotr.brendel@ii.uj.edu.pl
 *
 *         AccSub - constructing and removing acyclic subset
 *                  for simplicial complexes
 *         This code is a part of RedHom library
 *         http://redhom.ii.uj.edu.pl
 */

#ifndef ACCSPANNINGTREE_HPP
#define ACCSPANNINGTREE_HPP

#include "IncidenceGraphAlgorithms.hpp"
#include <vector>
#include <set>
#include <map>

#ifdef ACCSUB_TRACE
#include "../Helpers/Utils.hpp"
#endif

template <typename PartitionGraph>
class AccSpanningTreeT
{
    typedef typename PartitionGraph::IncidenceGraph IncidenceGraph;
    typedef typename IncidenceGraph::Vertex Vertex;
    typedef typename IncidenceGraph::VertsSet VertsSet;
    typedef typename IncidenceGraph::Simplex Simplex;
    typedef typename IncidenceGraph::SimplexPtrList SimplexPtrList;
    typedef typename IncidenceGraph::ConnectedComponent ConnectedComponent;
    typedef typename IncidenceGraph::ConnectedComponents ConnectedComponents;
    typedef typename IncidenceGraph::Path Path;

    typedef typename IncidenceGraph::Node IncidenceGraphNode;
    typedef typename PartitionGraph::Node PartitionGraphNode;

public:
    
    struct Edge;
    
    struct Node
    {
        PartitionGraphNode      *parent;
        int                     subtreeID;
        std::set<Vertex>        borderVerts;
        int                     accSubSize;
        std::vector<Edge *>     edges;
        ConnectedComponent      connectedComponent;
        std::vector<Vertex>     boundaryVertsToConnect;
        std::vector<Path>       boundaryVertsConnectingPaths;
        bool                    isConnectedToAccSub;

        Node(PartitionGraphNode *parent, int id, ConnectedComponent connectedComponent, std::set<Vertex> &borderVerts, int accSubSize)
        {
            this->parent = parent;
            this->subtreeID = id;
            this->connectedComponent = connectedComponent;
            this->borderVerts = borderVerts;
            this->accSubSize = accSubSize;
            this->isConnectedToAccSub = false;
        }

        void AddEdge(Edge *edge)
        {
            edges.push_back(edge);
        }

        void FindAccSubToBorderConnection(Vertex borderVertex, Path &path)
        {
            if (accSubSize > 0)
            {
                // finding path from boundary vertex to nearest acyclic subset
                path = FindPath(FindNode(parent->ig->nodes, FindNodeWithVertex<IncidenceGraph>(borderVertex)), FindPathToNodeWithAccIntersection<IncidenceGraph>());
                assert(path.size() > 0);
            }
            else
            {
                boundaryVertsToConnect.push_back(borderVertex);
            }
        }

        void UpdateAccSubToBorderConnection(Vertex borderVertex, Path &path)
        {
            if (accSubSize > 0)
            {
                if (isConnectedToAccSub)
                {
                    UpdatePathFromBorderToAccSub(borderVertex, path);
                }
                else
                {
                    UpdatePathFromAccSubToBorder(borderVertex, path);
                    isConnectedToAccSub = true;
                }
            }
            else
            {
                UpdateBoundaryVertsConnectingPaths();
                boundaryVertsConnectingPaths.clear();
                isConnectedToAccSub = true;
            }
        }

        // we assume we're in the part of acyclic subset that is already
        // "connected" to "big" acyclic subset.
        // this function's task is to connect boundary vertex to a part
        // of acyclic subset. we move along path starting from vertex
        // towards acyclic subset. once we find simplex that intersects
        // with acyclic subset (which can be part of other path) we stop.
        void UpdatePathFromBorderToAccSub(Vertex borderVertex, Path &path)
        {
            IncidenceGraphNode *prevNode = path.front();
            typename Path::iterator i = path.begin();
            i++;
            // we have to guarantee that boundary vertex is connected to
            // acyclic subset. if we start with vertex that is already
            // added to acyclic subset then we don't need to do anything more
            if (prevNode->GetAccInfo().IsVertexInAccIntersection(borderVertex))
            {
                return;
            }
            Vertex lastVertex = borderVertex;
            VertsSet vertsOnPath;
            vertsOnPath.insert(lastVertex);
            for (; i != path.end(); i++)
            {
                Vertex vertex = Simplex::GetVertexFromIntersection(prevNode->simplex, (*i)->simplex);
                if (vertex == lastVertex)
                {
                    prevNode = *i;
                }
                else if ((*i)->GetAccInfo().IsVertexInAccIntersection(vertex))
                {
                    prevNode->GetAccInfo().UpdateAccIntersectionWithEdge(lastVertex, vertex);
                    prevNode = 0;
                    break;
                }
                else
                {
                    vertsOnPath.insert(vertex);
                    prevNode->GetAccInfo().UpdateAccIntersectionWithEdge(lastVertex, vertex);
                    prevNode = (*i);
                    lastVertex = vertex;
                    vertex = prevNode->GetAccInfo().FindAccVertexNotIn(vertsOnPath);
                    if (vertex != Vertex(-1))
                    {
                        prevNode->GetAccInfo().UpdateAccIntersectionWithEdge(lastVertex, vertex);
                        prevNode = 0;
                        break;
                    }
                }
            }
            // if there were only one on on the path
            if (prevNode != 0)
            {
                Vertex vertex = prevNode->GetAccInfo().FindAccVertexNotEqual(lastVertex);
                assert(vertex != Vertex(-1));
                prevNode->GetAccInfo().UpdateAccIntersectionWithEdge(lastVertex, vertex);

            }
        }

        // no we assume that we're in a part of acyclic subset that needs
        // to be "connected" to "big" acyclic subset.
        // this function's task is to connect acyclic subset to a part
        // of already constructed "big" acyclic subset. we start with
        // a vertex from part of acyclic subset that we want to connect
        // and move along path towards boundary vertex (of which we are
        // sure that has been already connected to "big" acyclic subset
        // - see previous function). once we reach boundary vertex or 
        // another part of acyclic subset we stop.
        void UpdatePathFromAccSubToBorder(Vertex borderVertex, Path &path)
        {
            path.reverse();
            typename Path::iterator i = path.begin();
            IncidenceGraphNode *prevNode = *i;
            i++;
            // first node has to intersects acyclic subset
            Vertex lastVertex = prevNode->GetAccInfo().FindAccVertexNotIn(borderVerts);
            assert(lastVertex != Vertex(-1));
            for (; i != path.end(); i++)
            {
                Vertex vertex = Simplex::GetVertexFromIntersection(prevNode->simplex, (*i)->simplex);
                if (vertex == lastVertex)
                {
                    prevNode = *i;
                }
                else if ((*i)->GetAccInfo().IsVertexInAccIntersection(vertex))
                {
                    prevNode->GetAccInfo().UpdateAccIntersectionWithEdge(lastVertex, vertex);
                    prevNode = 0;
                    break;
                }
                else
                {
                    prevNode->GetAccInfo().UpdateAccIntersectionWithEdge(lastVertex, vertex);
                    prevNode = (*i);
                    lastVertex = vertex;
                }
            }
            // if prevNode == 0 this means we have reached a part of acyclic
            // subset and we can stop whole procedure
            // in other case we have to connect last vertex to boundary vertex
            if (prevNode != 0)
            {
                // it may happen that last two simplices intersection is
                // just our boundary vertex. because of that we have to
                // check that we don't add degenerated edge
                if (lastVertex != borderVertex)
                {
                    prevNode->GetAccInfo().UpdateAccIntersectionWithEdge(lastVertex, borderVertex);
                }
            }
        }
        
        // finding paths connect boundary vertices
        void FindBoundaryVertsConnectingPaths()
        {
            // if there is less than 2 vertices it means that this part
            // does not connect any two acyclic subsets so we can ignore it
            if (boundaryVertsToConnect.size() < 2)
            {
                return;
            }

            typename std::vector<Vertex>::iterator vertex = boundaryVertsToConnect.begin();
            Vertex firstVertex = *vertex;
            IncidenceGraphNode *firstNode = FindNode(connectedComponent, FindNodeWithVertex<IncidenceGraph>(firstVertex));
            vertex++;
            for (; vertex != boundaryVertsToConnect.end(); vertex++)
            {
                Path path = FindPath(firstNode, FindPathToVertex<IncidenceGraph>(*vertex));
                assert(path.size() > 0);
                boundaryVertsConnectingPaths.push_back(path);
            }
        }

        // paths found in previous function are formed into tree
        // the root if the tree is first vertex on the list
        // we move from currently added vertex towards root at stop once
        // we intersect a part of acyclic subset (in the worst case -> root)
        void UpdateBoundaryVertsConnectingPaths()
        {
            // if there is less than one path it means that part
            // does not connect any two acyclic subsets so we can ignore it
            if (boundaryVertsConnectingPaths.size() < 1)
            {
                return;
            }

            typename std::vector<Vertex>::iterator vertex = boundaryVertsToConnect.begin();
            Vertex firstVertex = *vertex;
            IncidenceGraphNode *firstNode = FindNode(connectedComponent, FindNodeWithVertex<IncidenceGraph>(firstVertex));
            firstNode->GetAccInfo().UpdateAccIntersectionWithVertex(firstVertex);
            vertex++;
            for (typename std::vector<Path>::iterator path = boundaryVertsConnectingPaths.begin(); path != boundaryVertsConnectingPaths.end(); path++, vertex++)
            {
                assert(vertex != boundaryVertsToConnect.end());
                IncidenceGraphNode *prevNode = path->back();
                // if vertex is already in the acyclic subset, then we
                // stop adding path for this vertex. it means we have reached
                // a part of acyclic subset (path added before).
                if (prevNode->GetAccInfo().IsVertexInAccIntersection(*vertex))
                {
                    continue;
                }
                Vertex lastVertex = *vertex;
                typename Path::reverse_iterator i = path->rbegin();
                i++;
                VertsSet vertsOnPath;
                vertsOnPath.insert(lastVertex);
                for (; i != path->rend(); i++)
                {
                    Vertex vertex = Simplex::GetVertexFromIntersection(prevNode->simplex, (*i)->simplex);
                    if (vertex == lastVertex)
                    {
                        prevNode = *i;
                    }
                    else if ((*i)->GetAccInfo().IsVertexInAccIntersection(vertex))
                    {
                        prevNode->GetAccInfo().UpdateAccIntersectionWithEdge(lastVertex, vertex);
                        prevNode = 0;
                        break;
                    }
                    else
                    {
                        vertsOnPath.insert(vertex);
                        prevNode->GetAccInfo().UpdateAccIntersectionWithEdge(lastVertex, vertex);
                        prevNode = (*i);
                        lastVertex = vertex;
                        vertex = prevNode->GetAccInfo().FindAccVertexNotIn(vertsOnPath);
                        if (vertex != Vertex(-1))
                        {
                            prevNode->GetAccInfo().UpdateAccIntersectionWithEdge(lastVertex, vertex);
                            prevNode = 0;
                            break;
                        }
                    }
                }
                // if we are here it means we didn't intersect acyclic subset
                // so have to connect path to root (first vertex on the list)
                if (prevNode != 0)
                {
                    // it may happen that last two simplices intersection is
                    // just our boundary vertex. because of that we have to
                    // check that we don't add degenerated edge
                    if (lastVertex != firstVertex)
                    {
                        prevNode->GetAccInfo().UpdateAccIntersectionWithEdge(lastVertex, firstVertex);
                    }
                }
            }
        }
    };
    
    struct Edge
    {
        Node    *nodeA;
        Node    *nodeB;
        Path    pathToA;
        Path    pathToB;
        Vertex  intersectionVertex;
        bool    isInSpanningTree;

        Edge(Node *na, Node *nb, Vertex iv)
        {
            nodeA = na;
            nodeB = nb;
            intersectionVertex = iv;
            isInSpanningTree = false;
        }

        void FindAccConnections()
        {
            nodeA->FindAccSubToBorderConnection(intersectionVertex, pathToA);
            nodeB->FindAccSubToBorderConnection(intersectionVertex, pathToB);
        }

        void UpdateAccConnections()
        {
            // first we add path that connects part of acyclic subset
            // already added to spanning tree with boundary vertex.
            // it gives us certainty that every time we connect another
            // path to this boundary vertex the path will be also connected
            // to spanning tree
            if (nodeA->isConnectedToAccSub)
            {
                nodeA->UpdateAccSubToBorderConnection(intersectionVertex, pathToA);
                nodeB->UpdateAccSubToBorderConnection(intersectionVertex, pathToB);
            }
            else
            {
                nodeB->UpdateAccSubToBorderConnection(intersectionVertex, pathToB);
                nodeA->UpdateAccSubToBorderConnection(intersectionVertex, pathToA);
            }
        }
    };

    typedef std::vector<Node *> Nodes;
    typedef std::vector<Edge *> Edges;
    
    Nodes nodes;
    Edges edges;    

    AccSpanningTreeT(PartitionGraph *pg)
    {
        // we build a graph in which nodes are disjoint parts of acyclic
        // subset and edges are paths connecting them
        // we then use this graph to build spanning tree connecting smaller
        // paths into one big acyclic subset
        typename std::map<PartitionGraphNode *, std::vector<Node *> > dataNodeChildren;
        int currentID = 1;
        for (typename PartitionGraph::Nodes::iterator i = pg->nodes.begin(); i != pg->nodes.end(); i++)
        {
            IncidenceGraph *ig = (*i)->ig;
            typename std::vector<std::set<Vertex> >::iterator ccb = ig->connectedComponentsBorders.begin();
            std::vector<int>::iterator ccass = ig->connectedComponentsAccSubSize.begin();
            for (typename ConnectedComponents::iterator cc = ig->connectedComponents.begin(); cc != ig->connectedComponents.end(); cc++)
            {
                Node *newNode = new Node(*i, currentID++, *cc, *ccb, *ccass);
                dataNodeChildren[*i].push_back(newNode);
                nodes.push_back(newNode);
                ccb++;
                ccass++;
            }
        }

#ifdef ACCSUB_TRACE
        Timer::Update("creating acyclic tree nodes");
#endif

        for (typename Nodes::iterator node = nodes.begin(); node != nodes.end(); node++)
        {
            PartitionGraphNode *parent = (*node)->parent;
            std::vector<Node *> potentialNeighbours;
            for (typename PartitionGraph::Edges::iterator edge = parent->edges.begin(); edge != parent->edges.end(); edge++)
            {
                PartitionGraphNode *neighbour = ((*edge)->nodeA == parent) ? (*edge)->nodeB : (*edge)->nodeA;
                potentialNeighbours.insert(potentialNeighbours.end(), dataNodeChildren[neighbour].begin(), dataNodeChildren[neighbour].end());
            }
            for (typename Nodes::iterator neighbour = potentialNeighbours.begin(); neighbour != potentialNeighbours.end(); neighbour++)
            {
                if ((*neighbour)->subtreeID <= (*node)->subtreeID)
                {
                    continue;
                }
                Simplex intersection;
                GetSortedIntersectionOfUnsortedSets(intersection, (*node)->borderVerts, (*neighbour)->borderVerts);
                if (intersection.size() > 0)
                {
                    Edge *edge = new Edge(*node, *neighbour, intersection.front());
                    edges.push_back(edge);
                    (*node)->AddEdge(edge);
                    (*neighbour)->AddEdge(edge);
                }
            }
        }

#ifdef ACCSUB_TRACE
        Timer::Update("creating acyclic tree edges");
#endif

        // we create hash in which keys are IDs of every part
        // of acyclic subset and values are sizes of this sets/
        std::map<int, int> spanningTreeAccSubSize;
        for (typename Nodes::iterator node = nodes.begin(); node != nodes.end(); node++)
        {
            spanningTreeAccSubSize[(*node)->subtreeID] = (*node)->accSubSize;
        }

        for (typename Edges::iterator edge = edges.begin(); edge != edges.end(); edge++)
        {
            if ((*edge)->nodeA->subtreeID == (*edge)->nodeB->subtreeID)
            {
                continue;
            }
            (*edge)->isInSpanningTree = true;
            int newID = (*edge)->nodeA->subtreeID;
            int oldID = (*edge)->nodeB->subtreeID;
            // while connecting two trees into one we also sum the sizes
            // of acyclic subsets in each tree
            spanningTreeAccSubSize[newID] = spanningTreeAccSubSize[newID] + spanningTreeAccSubSize[oldID];
            for (typename Nodes::iterator node = nodes.begin(); node != nodes.end(); node++)
            {
                if ((*node)->subtreeID == oldID)
                {
                    (*node)->subtreeID = newID;
                }
            }
        }

#ifdef ACCSUB_TRACE
        Timer::Update("creating spanning tree");
#endif

        // if there is a tree with acyclic subset size equal to zero this
        // mean that there is a connected component in simplicial complex
        // in which we didn't found any acyclic subset (probably beacuse
        // of boundary simplices restriction). since acyclic subset can be
        // found in ANY sonnected component (namely: single vertex), we remove
        // all simplices contained in this tree and construct acyclic subset
        // for such component without boundary simplices restriction (since
        // this is connected component it does not have neighbours in other
        // packages)
        typename std::map<int, SimplexPtrList> simplexPtrLists;
        typename Nodes::iterator node = nodes.begin();
        while (node != nodes.end())
        {
            if (spanningTreeAccSubSize[(*node)->subtreeID] == 0)
            {
                (*node)->parent->ig->RemoveConnectedComponentAndCopySimplexList((*node)->connectedComponent, simplexPtrLists[(*node)->subtreeID]);
                typename Edges::iterator edge = edges.begin();
                while (edge != edges.end())
                {
                    if ((*edge)->nodeA == *node || (*edge)->nodeB == *node)
                    {
                        edge = edges.erase(edge);
                    }
                    else
                    {
                        edge++;
                    }
                }
                node = nodes.erase(node);
            }
            else
            {
                node++;
            }
        }

        // for every connected component from previous step we create
        // new data node that will be used to computin acyclic subset
        // and incidence graph
        for (typename std::map<int, SimplexPtrList>::iterator i = simplexPtrLists.begin(); i != simplexPtrLists.end(); i++)
        {
            pg->isolatedNodes.push_back(new PartitionGraphNode(i->second));
        }

#ifdef ACCSUB_TRACE
        Timer::Update("finding isolated data nodes");
#endif

        // we find connections between acyclic subset but do not add it
        // to acyclic subset yet
        for (typename Edges::iterator i = edges.begin(); i != edges.end(); i++)
        {
            if ((*i)->isInSpanningTree) (*i)->FindAccConnections();
        }
        for (typename Nodes::iterator i = nodes.begin(); i != nodes.end(); i++)
        {
            (*i)->FindBoundaryVertsConnectingPaths();
        }

#ifdef ACCSUB_TRACE
        Timer::Update("searching paths from acyclic subsets to border");
#endif
    }

    ~AccSpanningTreeT()
    {
        for (typename Nodes::iterator i = nodes.begin(); i != nodes.end(); i++)
        {
            delete (*i);
        }
        for (typename Edges::iterator i = edges.begin(); i != edges.end(); i++)
        {
            delete (*i);
        }
    }
    
    void JoinAccSubsets()
    {
        // paths need to be found in previous steps
        for (typename Edges::iterator i = edges.begin(); i != edges.end(); i++)
        {
            if ((*i)->isInSpanningTree) (*i)->UpdateAccConnections();
        }
#ifdef ACCSUB_TRACE
        Timer::Update("adding paths to acyclic subset");
#endif
    }
};

#endif /* ACCSPANNINGTREE_HPP */
