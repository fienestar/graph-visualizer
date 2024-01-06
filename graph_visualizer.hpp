#ifndef FIENE_GRAPH_VISUALIZER
#define FIENE_GRAPH_VISUALIZER

#include <vector>
#include <string>
#include <functional>
#include <sstream>
#include <set>
#include <map>

template <class Index>
class graph_visualizer
{
public:
	graph_visualizer()
	 :m_show_index(true),
	 m_directed(false),
	 m_get_description_cb([](Index){ return ""; }),
	 m_get_adj_nodes_cb([](Index){ return std::vector<std::pair<Index, std::string>>{}; }),
	 m_get_color_cb([](Index){return "";}) {}

	graph_visualizer& description(std::function<void(Index, std::ostream&)> get_description_cb) {
		m_get_description_cb = [get_description_cb](Index index) {
			std::ostringstream os;
			get_description_cb(index, os);

			return escape(os.str(), "\"<>#&");
		};
		return *this;
	}

	graph_visualizer& html_description(std::function<void(Index, std::ostream&)> get_description_cb) {
		m_get_description_cb = [get_description_cb](Index index) {
			std::ostringstream os;
			get_description_cb(index, os);

			return escape(os.str(), "\"");
		};
		return *this;
	}

	graph_visualizer& adj_nodes(std::function<std::vector<Index>(Index)> get_adj_nodes_cb) {
		m_get_adj_nodes_cb = [get_adj_nodes_cb](Index index) {
			std::vector<Index> adj_nodes = get_adj_nodes_cb(index);
			std::vector<std::pair<Index, std::string>> adj_nodes_with_description;
			adj_nodes_with_description.reserve(adj_nodes.size());
			for(auto u:adj_nodes)
				adj_nodes_with_description.push_back({u, ""});
			return adj_nodes_with_description;
		};
		return *this;
	}

	graph_visualizer& adj_nodes(std::function<std::vector<std::pair<Index, std::string>>(Index)> get_adj_nodes_cb) {
		m_get_adj_nodes_cb = get_adj_nodes_cb;
		return *this;
	}

	graph_visualizer& color(std::function<std::string(Index)> get_color_cb) {
		m_get_color_cb = get_color_cb;
		return *this;
	}

	graph_visualizer& hide_index(bool value = false) {
		m_show_index = value;
		return *this;
	}

	graph_visualizer& directed(bool value = true) {
		m_directed = value;
		return *this;
	}

	std::string get_diagram(std::vector<Index> nodes) {
#ifndef ONLINE_JUDGE
		std::ostringstream os;
		os << "```mermaid\ngraph\n";
		std::set<Index> visited;
		std::function<void(Index)> printEdges = [&](Index u){
			if(visited.count(u)) return;
			visited.insert(u);

			for(auto [v, edge_name]:m_get_adj_nodes_cb(u)) {
				if(m_directed || !visited.count(v)) {
					os << "    " << get_node_identifier(u);
					if(m_directed)
						os << " --> ";
					else
						os << " --- ";
					os << get_node_identifier(v);
					if(!edge_name.empty()) os << ": " << escape(edge_name, "\"<>#& -\n\t");
					os << "\n";
				}
				printEdges(v);
			}
		};

		for (auto u:nodes)
			if(!visited.count(u))
				printEdges(u);

		std::map<std::string, size_t> color_index;
		size_t color_index_end = 0;
		for (auto u:visited) {
			auto color = m_get_color_cb(u);
			if(color.empty()) continue;

			if(!color_index.count(color)) {
				os << "    classDef c" << color_index_end << " stroke:" << color << "\n";
				color_index[color] = color_index_end++;
			}
			os << "    class " << u << " c" << color_index[color] << "\n";
		}

		for (auto u:visited)
			os << "    " << get_description_statement(u);
		os << "```";

		return os.str();
#else
		return "";
#endif
	}
private:
	std::function<std::string(Index)> m_get_description_cb;
	std::function<std::vector<std::pair<Index, std::string>>(Index)> m_get_adj_nodes_cb;
	std::function<std::string(Index)> m_get_color_cb;
	bool m_show_index;
	bool m_directed;
	static std::string escape(const std::string& str, const std::string& chars)
	{
		std::ostringstream os;
		for(auto ch:str)
			if(strchr(chars.c_str(), ch))
				os << "#" << +ch << ";";
			else
				os << ch;
		return os.str();
	}

	std::string get_description_statement(Index index)
	{
		std::ostringstream os;

		auto description = m_get_description_cb(index);
		if(!description.empty()) {
			os << get_node_identifier(index) << "(\"";
			if (m_show_index)
				os << get_node_identifier(index) << "<br>";
			os << description << "\")\n";
		}

		return os.str();
	}

	std::string get_node_identifier(Index index)
	{
		return escape(std::to_string(index), "()<>#& -\n\t");
	}
};

#endif
