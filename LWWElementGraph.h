#pragma once

#include <unordered_map>
#include <set>
#include <queue>

/// <summary>
/// Last-Writer-Wins Set.
/// </summary>
/// <typeparam name="E">Data type</typeparam>
/// <typeparam name="T">Timestamp type, must be comparable</typeparam>
template<class E, class T>
class LWWElementSet
{
	std::unordered_map<E, T> add;
	std::unordered_map<E, T> remove;

public:
	/// <summary>
	/// Adds the element to the set.
	/// 
	/// If adding and removing occur at the same time, removing has priority.
	/// </summary>
	/// <param name="e">Element</param>
	/// <param name="t">Timestamp</param>
	void Add(E e, T t)
	{
		if (AddExist(e))
			add[e] = std::max(add[e], t);
		else
			add[e] = t;
	}

	/// <summary>
	/// Removes the element from the set.
	/// 
	/// If adding and removing occur at the same time, removing has priority.
	/// </summary>
	/// <param name="e">Element</param>
	/// <param name="t">Timestamp</param>
	void Remove(E e, T t)
	{
		if (RemoveExist(e))
			remove[e] = std::max(remove[e], t);
		else
			remove[e] = t;
	}

	/// <summary>
	/// Was the element ever added?
	/// </summary>
	/// <param name="e">Element</param>
	/// <returns>true if it was, false if it was not</returns>
	bool AddExist(E e) const
	{
		return add.find(e) != add.end();
	}

	/// <summary>
	/// Timestamp of adding the element
	/// </summary>
	/// <param name="e">Element</param>
	/// <returns>Timestamp</returns>
	T AddTimestamp(E e) const
	{
		return add.at(e);
	}

	/// <summary>
	/// Add map getter
	/// </summary>
	/// <returns>Add map</returns>
	const std::unordered_map<E, T>& GetAdd() const
	{
		return add;
	}

	/// <summary>
	/// Was the element ever removed?
	/// </summary>
	/// <param name="e">Element</param>
	/// <returns>true if it was, false if it was not</returns>
	bool RemoveExist(E e) const
	{
		return remove.find(e) != remove.end();
	}

	/// <summary>
	/// Timestamp of removing the element
	/// </summary>
	/// <param name="e">Element</param>
	/// <returns>Timestamp</returns>
	T RemoveTimestamp(E e) const
	{
		return remove.at(e);
	}

	/// <summary>
	/// Does the set contain this element?
	/// </summary>
	/// <param name="e">Element</param>
	/// <returns>true if it contains, false if it does not</returns>
	bool Contains(E e) const
	{
		if (!AddExist(e))
			return false;
		if (!RemoveExist(e))
			return true;
		return AddTimestamp(e) > RemoveTimestamp(e);
	}

	/// <summary>
	/// Merges with concurrent changes in other set.
	/// </summary>
	/// <param name="g">Other set</param>
	void Merge(const LWWElementSet& s)
	{
		for (const auto& [e, t] : s.add)
			Add(e, t);
		for (const auto& [e, t] : s.remove)
			Remove(e, t);
	}

	/// <summary>
	/// Is equal to operator
	/// </summary>
	/// <param name="s">Set to compare with</param>
	/// <returns>true if equal, false otherwise</returns>
	bool operator==(const LWWElementSet& s) const
	{
		return add == s.add && remove == s.remove;
	}
};

/// <summary>
/// Last-Writer-Wins Directed Graph.
/// 
/// Stores vertices and edges, does not store data.
/// </summary>
/// <typeparam name="E">Vertex key type</typeparam>
/// <typeparam name="T">Timestamp type, must be comparable</typeparam>
template<class E, class T>
class LWWElementGraph
{
	LWWElementSet<E, T> vertices;
	std::unordered_map<E, LWWElementSet<E, T>> edges;

public:
	/// <summary>
	/// Adds the vertex to the graph.
	/// </summary>
	/// <param name="e">Vertex key</param>
	/// <param name="t">Timestamp</param>
	void AddVertex(E e, T t)
	{
		vertices.Add(e, t);
	}

	/// <summary>
	/// Removes the vertex from the graph.
	/// 
	/// Removes all edges connected with this vertex.
	/// </summary>
	/// <param name="e">Vertex key</param>
	/// <param name="t">Timestamp</param>
	void RemoveVertex(E e, T t)
	{
		vertices.Remove(e, t);
	}

	/// <summary>
	/// Does the graph contain this vertex?
	/// </summary>
	/// <param name="e">Vertex key</param>
	/// <returns>
	/// true if it contains, false if it does not
	/// </returns>
	bool ContainsVertex(E e) const
	{
		return vertices.Contains(e);
	}

	/// <summary>
	/// Adds the edge to the graph.
	/// 
	/// The vertices of this edge must be in the graph already with the same or lesser timestamp.
	/// </summary>
	/// <param name="from">Source vertex key</param>
	/// <param name="to">Destination vertex key</param>
	/// <param name="t">Timestamp</param>
	void AddEdge(E from, E to, T t)
	{
		edges[from].Add(to, t);
	}

	/// <summary>
	/// Removes the edge from the graph.
	/// </summary>
	/// <param name="from">Source vertex key</param>
	/// <param name="to">Destination vertex key</param>
	/// <param name="t">Timestamp</param>
	void RemoveEdge(E from, E to, T t)
	{
		edges[from].Remove(to, t);
	}

	/// <summary>
	/// Does the graph contain this edge?
	/// </summary>
	/// <param name="from">Source vertex key</param>
	/// <param name="to">Destination vertex key</param>
	/// <returns>
	/// true if it contains, false if it does not
	/// </returns>
	bool ContainsEdge(E from, E to) const
	{
		// If there is no edge with the given vertices
		if (edges.find(from) == edges.end() || !edges.at(from).Contains(to))
			return false;

		// Or one of the vertices is not alive
		if (!ContainsVertex(from) || !ContainsVertex(to))
			return false;

		const T& addTimestamp = edges.at(from).AddTimestamp(to);

		// If some vertex was deleted - delete all edges connected to it
		if (vertices.RemoveExist(from) && addTimestamp <= vertices.RemoveTimestamp(from) ||
			vertices.RemoveExist(to) && addTimestamp <= vertices.RemoveTimestamp(to))
			return false;

		// The edges must be added after or at the same time as the vertices
		if (addTimestamp < vertices.AddTimestamp(from) ||
			addTimestamp < vertices.AddTimestamp(to))
			return false;

		return true;
	}

	/// <summary>
	/// Merges with concurrent changes in other graph.
	/// </summary>
	/// <param name="g">Other graph</param>
	void Merge(const LWWElementGraph& g)
	{
		vertices.Merge(g.vertices);

		for (const auto& [from, edge] : g.edges)
			edges[from].Merge(edge);
	}

	/// <summary>
	/// Query for all vertices connected to the vertex.
	/// 
	/// Complexity O(edges).
	/// </summary>
	/// <param name="e">Vertex key</param>
	/// <returns>The set of vertices connected to a given one. Includes both incoming and outgoing edges.</returns>
	std::set<E> AllConnectedVertices(E e) const
	{
		std::set<E> res;

		for (const auto& [from, edge] : edges)
		{
			if (from == e)
			{
				for (const auto& [to, t] : edge.GetAdd())
					if (ContainsEdge(from, to))
						res.insert(to);
			}
			else
			{
				for (const auto& [to, t] : edge.GetAdd())
					if (to == e && ContainsEdge(from, to))
						res.insert(from);
			}
		}

		return res;
	}

	/// <summary>
	/// Searches the path from one vertex to another.
	/// 
	/// BFS iterative algorithm.
	/// </summary>
	/// <param name="from">Source vertex key</param>
	/// <param name="to">Destination vertex key</param>
	/// <returns>Path, if it exists, or empty vector, if there is no path or the specified vertices are not contained in the graph.</returns>
	std::vector<E> AnyPath(E from, E to) const
	{
		if (!ContainsVertex(from) || !ContainsVertex(to))
			return std::vector<E>();

		std::queue<E> queue;
		std::unordered_map<E, E> previous;

		queue.push(from);
		previous[from] = from;

		while (!queue.empty())
		{
			E e = queue.front();
			queue.pop();

			if (e == to)
			{
				std::vector<E> path;

				for(; e != from; e = previous[e])
					path.push_back(e);
				path.push_back(e);

				std::reverse(path.begin(), path.end());

				return path;
			}

			for (const auto& [next, t] : edges.at(e).GetAdd())
				if (previous.find(next) == previous.end() && ContainsEdge(e, next))
				{
					previous[next] = e;
					queue.push(next);
				}
		}

		return std::vector<E>();
	}

	/// <summary>
	/// Is equal to operator
	/// </summary>
	/// <param name="g">Graph to compare with</param>
	/// <returns>true if equal, false otherwise</returns>
	bool operator==(const LWWElementGraph& g) const
	{
		return vertices == g.vertices && edges == g.edges;
	}
};
