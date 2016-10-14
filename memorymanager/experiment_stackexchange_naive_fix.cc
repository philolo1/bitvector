#include <iostream>
#include <vector>
#include <list>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdlib>
#include <stdint.h>

#include "MyTimer.cc"
#include "Helper.cc"


enum EdgeType {
  REPLY_TO, POSTED, VOTED_FOR, COMMENT_ON, MADE_COMMENT
};

enum VertexType {
  POST, REPLY, COMMENT, USER
};

VertexType VertexStrToEnum(std::string const &text) {
  if (text == "POST") {
    return POST;
  } else if (text == "REPLY") { 
    return REPLY;
  } else if (text == "COMMENT") { 
    return COMMENT;
  } else if (text == "USER") { 
    return USER;
  } else {
    std::cerr << "Received invalid enum type: " << text << std::endl;
    exit(1);
  }
}

std::string VertexEnumToStr(VertexType v) {
  switch(v) {
  case POST:
    return "POST";
  case REPLY:
    return "REPLY";
  case COMMENT:
    return "COMMENT";
  case USER:
    return "USER";
  default:
    std::cerr << "Received invalid enum type: " << v << std::endl;
    exit(1);
  }
}

std::string EdgeEnumToStr(EdgeType e) {
  switch(e) {
  case POSTED:
    return "POSTED";
  case REPLY_TO:
    return "REPLY_TO";
  case COMMENT_ON:
    return "COMMENT_ON";
  case MADE_COMMENT:
    return "MADE_COMMENT";
  case VOTED_FOR:
    return "VOTED_FOR";
  default:
    std::cerr << "Received invalid enum type: " << e << std::endl;
    exit(1);
  }
}


EdgeType EdgeStrToEnum(std::string const &text) {
  if (text == "POSTED") {
    return POSTED;
  } else if (text == "REPLY_TO") { 
    return REPLY_TO;
  } else if (text == "COMMENT_ON") { 
    return COMMENT_ON;
  } else if (text == "MADE_COMMENT") { 
    return MADE_COMMENT;
  } else if (text == "VOTED_FOR") { 
    return VOTED_FOR;
  } else {
    std::cerr << "Received invalid enum type: " << text << std::endl;
    exit(1);
  }
}

typedef uint32_t node_id_t;
typedef uint32_t timestamp_t;

class Edge {
private:
  node_id_t target;
  EdgeType edge_type;
  timestamp_t timestamp;
public:
  Edge (node_id_t t, EdgeType e, timestamp_t ts) {
    target = t;
    edge_type = e;
    timestamp = ts;
  }

  EdgeType get_edge_type() {
    return edge_type;
  }

  node_id_t get_target() {
    return target;
  }

  timestamp_t get_timestamp() {
    return timestamp;
  }
};


class Node {

private:
  //  node_id_t vertex_id;
  VertexType vertex_type;
  timestamp_t timestamp;
  std::vector<Edge> in_neighbours;
  std::vector<Edge> out_neighbours;

public:

  Node(VertexType t, timestamp_t ts) {
    vertex_type = t;
    timestamp = ts;
  }

  void add_in_neighbour(node_id_t target, EdgeType edge_type, timestamp_t ts) {
    in_neighbours.push_back(Edge(target, edge_type, ts));
  }

  void add_out_neighbour(node_id_t target, EdgeType edge_type, timestamp_t ts) {
    out_neighbours.push_back(Edge(target, edge_type, ts));
  }

  std::vector<Edge>::const_iterator in_neighbours_begin() {
    return in_neighbours.begin();
  }

  std::vector<Edge>::const_iterator out_neighbours_begin() {
    return out_neighbours.begin();
  }

  std::vector<Edge>::const_iterator in_neighbours_end() {
    return in_neighbours.end();
  }

  std::vector<Edge>::const_iterator out_neighbours_end() {
    return out_neighbours.end();
  }

};



class Graph {
private:
  std::vector<Node> node_list;
public:
  Graph() { }

  void insert_vertex(node_id_t vid, VertexType type, timestamp_t ts) {
    if (vid == node_list.size()) {
      node_list.push_back(Node(type, ts));
    }
  }
  
  void insert_edge(node_id_t v1, node_id_t v2, EdgeType type, timestamp_t ts) {
    if (node_list.size() > v1 && node_list.size() > v2) {
      node_list[v1].add_out_neighbour(v2, type, ts);
      node_list[v2].add_in_neighbour(v1, type, ts);
    } else {
      std::cerr << "Unable to insert edge: vertex does not exist" << std::endl;
      exit(1);
    }
  }

  void extract_connected_components(std::vector<std::vector<node_id_t>> &output) {
    node_id_t n = node_list.size();
    bool *visited = new bool[n];
    for(node_id_t i = 0; i < n; i++) {
      visited[i] = false;
    }

    std::list<node_id_t> queue;
    for (node_id_t i = 0; i < n; i++) {
      if (visited[i] == false) {
        visited[i] = true;
        queue.push_back(i);
        std::vector<Edge>::const_iterator iter;
        std::vector<node_id_t> component;
        while(!queue.empty()) {
          node_id_t front = queue.front();
          component.push_back(front); 
          queue.pop_front();
          Node current = node_list[front];
          for(auto i = current.out_neighbours_begin(); i != current.out_neighbours_end(); ++i) {
            Edge out_edge = *i;
            if(!visited[out_edge.get_target()]) {
              visited[out_edge.get_target()] = true;
              queue.push_back(out_edge.get_target());
            }
          }
          for(auto i = current.in_neighbours_begin(); i != current.in_neighbours_end(); ++i) {
            Edge in_edge = *i;
            if (!visited[in_edge.get_target()]) {
              visited[in_edge.get_target()] = true;
              queue.push_back(in_edge.get_target());
            }
          }
        }
        output.push_back(component);
      }
    }
  }

};


void load_graph(Graph &g, std::string filename) {
  std::ifstream infile(filename);
  std::string line;
  while (infile) {
    std::getline(infile,line);
    std::stringstream parser(line);
    uint32_t ts;
    if (parser >> ts) {
      std::string type;
      if (parser >> type) {
        //std::cout <<  ts << std::endl;
        if (type.compare("E") == 0) {
          node_id_t v1,v2;
          std::string type;
          if (parser >> v1 >> v2 >> type) {
            // std::cout << "Edge detected: " << v1 << " " << v2 << " " << type << std::endl;
            g.insert_edge(v1, v2, EdgeStrToEnum(type), ts);
          } else {
            std::cerr << "Malformed line: " << line << std::endl;
            exit(1);
          }
        } else if (type.compare("V") == 0) {
          node_id_t v;
          std::string type;
          if (parser >> v >> type) {
            // std::cout << "Vertex detected: " << v << " " << type << std::endl;
            g.insert_vertex(v, VertexStrToEnum(type), ts);
          } else {
            std::cerr << "Malformed line: " << line << std::endl;
            exit(1);
          }
        }
      }
    }
  }
}

void write_connected_components(std::vector<std::vector<node_id_t>> &components, 
				std::string const &filename) {

  std::ofstream outfile;
  outfile.open(filename);

  for (auto next_vec_it = components.begin(); next_vec_it != components.end(); next_vec_it++) {
    std::vector<node_id_t> next_vec = *next_vec_it;
    outfile << next_vec.size() << " ";
    for (auto it = next_vec.begin(); it != next_vec.end(); it++) {
      outfile << (*it) << " ";
    }
    outfile << std::endl;
  }

  outfile.close();
}


int main(int argc, char **argv) {

  std::cout <<"naive"<<endl;

  if (argc != 3) {
    std::cout << "Usage: " << argv[0] << " [input_graph] [output_file]" << std::endl;
    exit(1);
  }

  MyTimer timer;
  Graph g;
  load_graph(g, std::string(argv[1]));

  Helper::measureSpace();

  timer.printTime("INIT GRAPH");


  std::vector<std::vector<node_id_t>> components;
  g.extract_connected_components(components);
  write_connected_components(components, std::string(argv[2]));

  Helper::measureSpace();
  timer.printTime("BFS");
}




