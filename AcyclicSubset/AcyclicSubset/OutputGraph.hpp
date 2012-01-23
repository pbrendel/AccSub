/*
 * File:   OutputGraph.hpp
 * Author: Piotr Brendel
 *         piotr.brendel@ii.uj.edu.pl
 *
 *         AccSub - constructing and removing acyclic subset
 *                  for simplicial complexes
 *         This code is a part of RedHom library
 *         http://redhom.ii.uj.edu.pl
 */

#ifndef OUTPUTGRAPH_HPP
#define OUTPUTGRAPH_HPP

template <typename IncidenceGraphType>
class OutputGraphT
{
public:

    typedef IncidenceGraphType IncidenceGraph;
    typedef typename IncidenceGraph::Vertex Vertex;
    typedef typename IncidenceGraph::Simplex Simplex;
    typedef typename IncidenceGraph::SimplexList SimplexList;
    typedef typename IncidenceGraph::IntersectionFlags IntersectionFlags;

    struct Node
    {        
        int                 index;
        Simplex             simplex;
        std::vector<Node *> subnodes;    
        std::vector<int>    kappa;
    
        Node(int index, const Simplex &s)
        {
            this->index = index;
            simplex = s;
        }
 
        void AddSubnode(Node *subnode, int kappa)
        {
            subnodes.push_back(subnode);
            this->kappa.push_back(kappa);
        }

        void GetSubnodes(std::vector<Node *> &nodes)
        {
            nodes.push_back(this);
            for (typename Nodes::iterator i = subnodes.begin(); i != subnodes.end(); i++)
            {
                (*i)->GetSubnodes(nodes);
            }
        }
        
        Node *FindNodeWithSimplex(const Simplex &s)
        {
            if (simplex == s)
            {
                return this;
            }
            for (typename Nodes::iterator i = subnodes.begin(); i != subnodes.end(); i++)
            {
                Node *n = (*i)->FindNodeWithSimplex(s);
                if (n != 0)
                {
                    return n;
                }
            }
            return 0;
        }

        static Node *FindNodeWithSimplex(std::vector<Node *> nodes, const Simplex &s)
        {
            for (typename Nodes::iterator i = nodes.begin(); i != nodes.end(); i++)
            {
                if ((*i)->simplex == s)
                {
                    return (*i);
                }
            }
            return 0;
        }

        static bool Sorter(const Node *a, const Node *b)
        {
            return (a->simplex.size() > b->simplex.size());
        }
    };

    typedef std::vector<Node *> Nodes;
    Nodes nodes;

    OutputGraphT(IncidenceGraph *ig)
    {
        nodeIndex = 0;
        incidenceGraph = ig;
        std::queue<typename IncidenceGraph::Node *> Q;
        for (typename IncidenceGraph::Nodes::iterator i = ig->nodes.begin(); i != ig->nodes.end(); i++)
        {
            if ((*i)->IsInAccSub() || (*i)->IsAddedToOutput())
            {
                continue;
            }
            Q.push(*i);
            (*i)->IsAddedToQueue(true);
            while (!Q.empty())
            {
                typename IncidenceGraph::Node *currentNode = Q.front();
                Q.pop();
                // flags of already generated simplices
                IntersectionFlags subnodesFlags = 0;
                // generated faces
                Nodes subnodes;
                for (typename IncidenceGraph::Edges::iterator edge = currentNode->edges.begin(); edge != currentNode->edges.end(); edge++)
                {
                    typename IncidenceGraph::Node *neighbour = (*edge)->GetNeighbour(currentNode);
                    if (!neighbour->IsInAccSub())
                    {
                        if (neighbour->IsAddedToOutput())
                        {
                            // if intresection is not in acyclic subset
                            IntersectionFlags intersectionFlags = (*edge)->intersection.GetFlags(currentNode);
                            if (!currentNode->GetAccInfo().IsInsideAccIntersection(intersectionFlags))
                            {
                                Node *outputNode = ((Node *)neighbour->helpers.ptr)->FindNodeWithSimplex((*edge)->intersection.Get());
                                assert(outputNode != 0);
                                outputNode->GetSubnodes(subnodes);
                                subnodesFlags |= intersectionFlags;
                            }
                        }
                        else
                        {
                            if (!neighbour->IsAddedToQueue())
                            {
                                neighbour->IsAddedToQueue(true);
                                Q.push(neighbour);
                            }
                        }
                    }
                }
                currentNode->IsAddedToOutput(true);
                currentNode->helpers.ptr = GenerateNode(currentNode, *currentNode->simplex, subnodes, subnodesFlags);
            }
        }
    }

    OutputGraphT(const SimplexList &simplexList)
    {
        nodeIndex = 0;
        for (typename SimplexList::const_iterator i = simplexList.begin(); i != simplexList.end(); i++)
        {
            Node *node = AddNode(*i);
            if ((*i).size() > 1)
            {
                Simplex s = *i;
                int kappa = 1;
                for (typename Simplex::iterator j = s.begin(); j != s.end(); j++)
                {
                    Vertex vertex = (*j);
                    j = s.erase(j);
                    node->AddSubnode(FindOrAddNode(*i), kappa);
                    j = s.insert(j, vertex);
                    kappa = -kappa;
                }
            }
        }
    }

    ~OutputGraphT()
    {
        for (typename Nodes::iterator i = nodes.begin(); i != nodes.end(); i++)
        {
            delete (*i);
        }
        nodes.clear();
    }

private:

    int             nodeIndex;
    IncidenceGraph  *incidenceGraph;

    // using flahs we check if generated simplex is contained in acyclic subset
    // if simplex has been already generated we're looking for it in neighbours
    // baseNode - node with maximal simplex which given simplex is face of
    // baseSimplex - simplex which we're generating node for
    // generatedSubnodes - already generated faces
    // subnodesFlags - flags of already generated faces
    Node *GenerateNode(typename IncidenceGraph::Node *baseNode, Simplex &baseSimplex, Nodes &generatedSubnodes, IntersectionFlags &subnodesFlags)
    {
        IntersectionFlags flags = incidenceGraph->configurationsFlags[baseNode->Normalize(baseSimplex)];
        if (flags == 0)
        {
            baseSimplex = baseNode->Normalize(baseSimplex);
        }
        assert(flags != 0);

        Node *newNode = 0;

        // if face is disjoint with acyclic intersection but has been already generated
        // then we are looking for it in the list of already generated faces
        // if subnodesFlags == 0 then no faces have been yet generated
        if (baseNode->GetAccInfo().IsDisjointWithAccIntersection(flags) && (subnodesFlags & flags) == flags)
        {
            return Node::FindNodeWithSimplex(generatedSubnodes, baseSimplex);
        }
        // if face is disjoint with acyclic subset and has not been yet generated
        // we generate new node and add it to the list of generated faces
        else if (baseNode->GetAccInfo().IsDisjointWithAccIntersection(flags)/* && (subnodesFlags & flags) == 0 */)
        {
            assert(baseSimplex.size() != 0);
            newNode = AddNode(baseSimplex);
            generatedSubnodes.push_back(newNode);
            // we set flags only for given simplex (not included its faces)
            subnodesFlags |= flags;
        }
        // if faces is completely contained in acyclic subset
        // we do not generate nothing more
        else if (baseNode->GetAccInfo().IsInsideAccIntersection(flags))
        {
            return 0;
        }
        else /* if (!baseNode->GetAccInfo().IsDisjointWithAccIntersection(flags)) */
        {
            assert(false);
            return 0;
        }

        assert(newNode != 0);

        // if it is possible to generate lower dimensional faces
        if (baseSimplex.size() > 1)
        {
            int kappa = 1;
            for (typename Simplex::iterator i = baseSimplex.begin(); i != baseSimplex.end(); i++)
            {
                Vertex vertex = (*i);
                i = baseSimplex.erase(i);
                Node *subnode = GenerateNode(baseNode, baseSimplex, generatedSubnodes, subnodesFlags);
                if (subnode != 0)
                {
                    newNode->AddSubnode(subnode, kappa);
                }
                i = baseSimplex.insert(i, vertex);
                kappa = -kappa;
            }
        }

        return newNode;
    }

    Node *AddNode(const Simplex &s)
    {
        Node *n = new Node(nodeIndex++, s);
        nodes.push_back(n);
        return n;
    }

    Node *FindOrAddNode(const Simplex &simplex)
    {
        Node *n = Node::FindNodeWithSimplex(nodes, simplex);
        if (n != 0)
        {
            return n;
        }
        Node *node = AddNode(simplex);
        if (simplex.size() > 1)
        {
            Simplex s = simplex;
            int kappa = 1;
            for (typename Simplex::iterator j = s.begin(); j != s.end(); j++)
            {
                Vertex vertex = (*j);
                j = s.erase(j);
                node->AddSubnode(FindOrAddNode(s), kappa);
                j = s.insert(j, vertex);
                kappa = -kappa;
            }
        }
        return node;
    }
};

#endif /* OUTPUTGRAPH_HPP */
