#include "LWWElementGraph.h"
#include <iostream>

#define ASSERT_TEST(condition) \
do { \
    if (! (condition)) \
        std::cerr << "Test `" #condition "` failed in " << __FILE__ \
                    << " line " << __LINE__ << std::endl; \
} while (false)

/// <summary>
/// Testing the independence of operations on the graph
/// </summary>
void TestOperations()
{
    {   // Idempotent operations
        LWWElementGraph<int, int> graph;

        ASSERT_TEST(!graph.ContainsVertex(0));

        graph.AddVertex(0, 0);
        graph.AddVertex(0, 0);

        ASSERT_TEST(graph.ContainsVertex(0));

        graph.AddVertex(1, 0);

        graph.AddEdge(0, 1, 1);
        graph.AddEdge(0, 1, 1);

        ASSERT_TEST(graph.ContainsEdge(0, 1));

        graph.RemoveEdge(0, 1, 2);

        ASSERT_TEST(!graph.ContainsEdge(0, 1));

        graph.RemoveEdge(0, 1, 2);

        ASSERT_TEST(!graph.ContainsEdge(0, 1));
    }

    {   // Commutative operations
        LWWElementGraph<int, int> a;
        LWWElementGraph<int, int> b;

        a.AddVertex(0, 0);
        a.AddVertex(1, 1);

        b.AddVertex(1, 1);
        b.AddVertex(0, 0);

        ASSERT_TEST(a == b);

        a.AddEdge(1, 0, 2);
        a.RemoveEdge(1, 0, 3);

        b.RemoveEdge(1, 0, 3);
        b.AddEdge(1, 0, 2);

        ASSERT_TEST(a == b);

        a.Merge(b);

        ASSERT_TEST(a == b);

        b.Merge(a);

        ASSERT_TEST(a == b);
    }

    {   // Associative operations
        LWWElementGraph<int, int> a;
        LWWElementGraph<int, int> b;
        LWWElementGraph<int, int> c;

        a.AddVertex(0, 0);
        b.AddVertex(1, 1);
        c.AddVertex(2, 2);

        LWWElementGraph<int, int> x;
        LWWElementGraph<int, int> y;
        LWWElementGraph<int, int> z;

        x.AddVertex(0, 0);
        y.AddVertex(1, 1);
        z.AddVertex(2, 2);

        a.Merge(b);
        a.Merge(c);

        y.Merge(z);
        x.Merge(y);

        ASSERT_TEST(a == x);
    }
}

/// <summary>
/// Testing the precedence of operations
/// </summary>
void TestPrecedence()
{
    {   // Adding and removing at the same time
        LWWElementGraph<int, int> graph;

        graph.AddVertex(0, 0);
        graph.RemoveVertex(0, 0);

        ASSERT_TEST(!graph.ContainsVertex(0));
    }

    {   // Adding edges with vertices at the same time
        LWWElementGraph<int, int> graph;

        graph.AddEdge(0, 1, 0);
        graph.AddVertex(0, 0);
        graph.AddVertex(1, 0);

        ASSERT_TEST(graph.ContainsEdge(0, 1));
        ASSERT_TEST(!graph.ContainsEdge(1, 0));
        ASSERT_TEST(graph.ContainsVertex(0));
        ASSERT_TEST(graph.ContainsVertex(1));
    }

    {   // Removing vertex at the same time with adding edge
        LWWElementGraph<int, int> graph;

        graph.AddVertex(0, 0);
        graph.AddVertex(1, 0);

        graph.AddEdge(0, 1, 1);
        graph.RemoveVertex(1, 1);

        ASSERT_TEST(!graph.ContainsEdge(0, 1));
        ASSERT_TEST(graph.ContainsVertex(0));
        ASSERT_TEST(!graph.ContainsVertex(1));
    }

    {   // Adding edge before adding vertices
        LWWElementGraph<int, int> graph;

        graph.AddVertex(0, 0);
        graph.AddVertex(1, 0);

        graph.AddEdge(0, 1, -1);

        ASSERT_TEST(!graph.ContainsEdge(0, 1));
        ASSERT_TEST(graph.ContainsVertex(0));
        ASSERT_TEST(graph.ContainsVertex(1));
    }
}

/// <summary>
/// Tests LWWElementGraph::AllConnectedVertices functionality
/// </summary>
void TestConnections()
{
    {   // Incoming and outgoing vertex connections
        LWWElementGraph<int, int> graph;

        graph.AddVertex(0, 0);

        const int ConnectedVertices = 20;

        for (int i = 1; i <= ConnectedVertices; i++)
        {
            graph.AddVertex(i, 0);
            graph.AddEdge(0, i, 0);
        }

        ASSERT_TEST(graph.AllConnectedVertices(0).size() == ConnectedVertices);

        for (int i = 1; i <= ConnectedVertices; i++)
        {
            graph.AddVertex(ConnectedVertices + i, 1);
            graph.AddEdge(i + ConnectedVertices, 0, 1);
        }

        ASSERT_TEST(graph.AllConnectedVertices(0).size() == ConnectedVertices * 2);
    }
}

/// <summary>
/// Tests LWWElementGraph::Merge functionality
/// </summary>
void TestMerging()
{
    {   // Simple two vertices merge
        LWWElementGraph<int, int> graph, other;

        graph.AddVertex(0, 0);
        other.AddVertex(1, 1);

        graph.Merge(other);

        ASSERT_TEST(graph.ContainsVertex(0));
        ASSERT_TEST(graph.ContainsVertex(1));
    }

    {   // Vertex removal merge
        LWWElementGraph<int, int> graph, other;

        graph.AddVertex(0, 1);
        other.AddVertex(0, 0);
        other.RemoveVertex(0, 2);

        graph.Merge(other);

        ASSERT_TEST(!graph.ContainsVertex(0));
    }

    {   // Cross removal merge
        LWWElementGraph<int, int> graph, other;

        graph.AddVertex(0, 1);
        graph.RemoveVertex(0, 3);
        other.AddVertex(0, 0);
        other.RemoveVertex(0, 2);

        graph.Merge(other);

        ASSERT_TEST(!graph.ContainsVertex(0));
    }

    {   // One lifetime inside other
        LWWElementGraph<int, int> graph, other;

        graph.AddVertex(0, 1);
        graph.RemoveVertex(0, 2);
        other.AddVertex(0, 0);
        other.RemoveVertex(0, 3);

        graph.Merge(other);

        ASSERT_TEST(!graph.ContainsVertex(0));
    }

    {   // Merging edges where one of the edges disappears because of LWW
        LWWElementGraph<int, int> graph, other;

        graph.AddVertex(0, 0);
        graph.AddVertex(1, 1);
        graph.AddEdge(1, 0, 2);

        other.AddVertex(0, 2);
        other.AddVertex(1, 3);
        other.AddEdge(0, 1, 4);

        graph.Merge(other);

        ASSERT_TEST(graph.ContainsEdge(0, 1));
        ASSERT_TEST(!graph.ContainsEdge(1, 0));

        // Merging remove edge operation from other graph
        other.RemoveEdge(0, 1, 5);

        graph.Merge(other);

        ASSERT_TEST(!graph.ContainsEdge(0, 1));
    }
}

/// <summary>
/// Tests LWWElementGraph::AnyPath functionality
/// </summary>
void TestAnyPath()
{
    {   // Some general pathfinding tests
        LWWElementGraph<int, int> graph;

        graph.AddVertex(0, 0);
        graph.AddVertex(1, 1);
        graph.AddVertex(2, 2);
        graph.AddVertex(3, 3);

        graph.AddEdge(0, 1, 4);
        graph.AddEdge(0, 2, 5);
        graph.AddEdge(1, 3, 6);
        graph.AddEdge(2, 3, 7);

        // There are two possible paths
        auto path = graph.AnyPath(0, 3);

        ASSERT_TEST(path[0] == 0);
        ASSERT_TEST(path[1] == 1 || path[1] == 2);
        ASSERT_TEST(path[2] == 3);

        graph.RemoveEdge(0, 1, 8);

        // Only one path is possible
        path = graph.AnyPath(0, 3);

        ASSERT_TEST(path[0] == 0);
        ASSERT_TEST(path[1] == 2);
        ASSERT_TEST(path[2] == 3);

        graph.RemoveEdge(2, 3, 9);

        // There is no path
        path = graph.AnyPath(0, 3);

        ASSERT_TEST(path.empty());
    }

    {   // Missing target vertex test
        LWWElementGraph<int, int> graph;

        graph.AddVertex(0, 0);
        graph.AddVertex(1, 1);
        graph.AddVertex(2, 2);
        graph.AddVertex(3, 3);

        graph.AddEdge(0, 1, 4);
        graph.AddEdge(0, 2, 5);
        graph.AddEdge(1, 3, 6);
        graph.AddEdge(2, 3, 7);

        graph.RemoveVertex(2, 8);

        ASSERT_TEST(!graph.ContainsEdge(0, 2));
        ASSERT_TEST(!graph.ContainsEdge(2, 3));

        // One of the target vertices is not in the graph
        auto path = graph.AnyPath(0, 2);

        ASSERT_TEST(path.empty());

        graph.AddVertex(2, 9);

        ASSERT_TEST(!graph.ContainsEdge(0, 2));
        ASSERT_TEST(!graph.ContainsEdge(2, 3));

        // Path to the same vertex
        path = graph.AnyPath(0, 0);

        ASSERT_TEST(path.size() == 1);
    }

    {   // Large path test
        int time = 0;
        LWWElementGraph<int, int> graph;

        const int NumVertices = 1'000'000;

        for (int i = 0; i < NumVertices; i++)
            graph.AddVertex(i, time++);
        for (int i = 0; i < NumVertices - 1; i++)
            graph.AddEdge(i, i + 1, time++);

        auto path = graph.AnyPath(0, NumVertices - 1);

        int i = 0;
        for (auto& e : path)
            ASSERT_TEST(e == i++);
    }
}

int main()
{
    TestOperations();
    TestPrecedence();
    TestConnections();
    TestMerging();
    TestAnyPath();
}
