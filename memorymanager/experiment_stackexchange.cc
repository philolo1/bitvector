#include <iostream>
#include <vector>
#include <list>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdlib>
#include <stdint.h>
#include <time.h>
#include <sys/time.h>

#include "datastructures/SetBitVector.cc"
#include "datastructures/AdvancedSetBitVector.cc"
#include "datastructures/BlockBitVector.cc"
#include "compression/LZ4Compressor.cc"
#include "compression/LZ4CompressorHC.cc"
#include "compression/SimdCompCompressor.cc"
#include "compression/NoCompressor.cc"
#include "compression/SnappyCompressor.cc"
#include <gtest/gtest.h>
#include "MyTimer.cc"
#include "datastructures/DynamicBitVector.cc"
#include "datastructures/FlexibleBitVector.cc"

#include "MemoryManager.cc"
#include "SimpleMemoryManager.cc"
#include "functions/SimpleFunction.cc"
#include "functions/GeometricFunction.cc"

#include <limits.h>

#include <random>
#include <iostream>
#include <functional>

#define BAG_SIZE 16


typedef uint32_t node_id_t;
typedef uint32_t timestamp_t;

enum EdgeType {
  REPLY_TO, POSTED, VOTED_FOR, COMMENT_ON, MADE_COMMENT
};

enum VertexType {
  POST, REPLY, COMMENT, USER
};


/**
 *
 */
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


/**
 *
 */
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


/**
 *
 */
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


/**
 *
 */
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


/**
 *
 */
class Node {
public:
  VertexType vert;
  timestamp_t timestamp;
  node_id_t id;

  /**
   *
   */
  Node (VertexType v, timestamp_t ts) {
    vert = v;
    timestamp = ts;
  }

  /**
   *
   */
  VertexType get_type() {
    return vert;
  }

  /**
   *
   */
  timestamp_t get_timestamp() {
    return timestamp;
  }
};


class Edge{
public:
  node_id_t target;
  EdgeType type;
  timestamp_t timestamp;

  /**
   *
   */
  Edge(node_id_t t, EdgeType y, timestamp_t ts) {
    target = t;
    type = y;
    timestamp = ts;
  }

  /**
   *
   */
  Edge(std::vector<uint32_t>::iterator start) {
    target = (node_id_t) *(start);
    type = (EdgeType) *(start + 1);
    timestamp = (timestamp_t) *(start + 2);
  }

  /**
   *
   */
  static int serialized_length() {
    return sizeof(Edge)/sizeof(uint32_t);
  }

  /**
   *
   */
  uint32_t serialize(int pos) {
    switch(pos) {
    case 0:
      return (uint32_t) target;
    case 1:
      return (uint32_t) type;
    case 2:
      return (uint32_t) timestamp;
    default:
      std::cerr << "Invalid serialization position" << std::endl;
      exit(1);
    }
  }
};


/**
 *
 */
class BlockArrayGroup {
  //typedef std::vector<uint32_t> FlexVector;
  //FlexVector<NoCompressor, MemoryManager, MemoryPointer> bv; std::uint32<vector_t> bv;
  std::vector<uint32_t> bv;
  int numArrays;

public:
  /**
   *
   */
  BlockArrayGroup(int na) {
    numArrays = na;

    for (int i = 0; i < numArrays; i++) {
      bv.push_back(0);
    }
    for (int i = 0; i < numArrays; i++) {
      get_array_range(i);
    }
  }

  ~BlockArrayGroup() { }

  /**
   * returns uint32 positions of bag bounaries
   */
  std::pair<int, int> get_array_range(int arr_num) {
    assert (arr_num >= 0 && arr_num < numArrays);
    assert (bv.size() > arr_num);
    uint32_t left = 0;
    uint32_t right = 0;

    //std::cout << bv.size() << " vs. " << arr_num << std::endl;

    for (int i = 0; i <= arr_num; i++) {
      if (arr_num == i)
        left = right;
      right += bv.at(i) * Edge::serialized_length();
    }

    left += numArrays;
    right += numArrays;
    return std::make_pair(left, right);
  }

  /**
   * pos; unit is Edges
   */
  int is_valid_pos(int arr_num, int pos) {
    return pos <= get_size(arr_num);
  }

  /**
   * pos; unit is Edges
   */
  int get_data_offset(int arr_num, int pos) {
    std::pair<int,int> range = get_array_range(arr_num);
    return range.first + pos * Edge::serialized_length();
  }

  /**
   * arr_num between [0,numArrays)
   * pos between [0,get_size(arr_num)); unit is Edges
   */
  void insert_into_bag(int arr_num, int pos, Edge data) {
    assert(arr_num >= 0 && arr_num < numArrays);
    assert(is_valid_pos(arr_num, pos));

    for (int i = 0; i < Edge::serialized_length(); i++) {
      // TODO: Change
      bv.insert(bv.begin() +
                get_data_offset(arr_num, pos) + i,
                data.serialize(i));
    }

    // TODO
    bv[arr_num]++;
    //bv.modify(arr_num, bv.at(arr_num) + 1);

  }

  /**
   *
   */
  void push_back_bag(int arr_num, Edge data) {
    assert(arr_num >= 0 && arr_num < numArrays);
    std::pair<int,int> range = this->get_array_range(arr_num);
    int position = (range.second - range.first) / Edge::serialized_length();
    // std::cout << "Inserting into bag: " << arr_num << " @ position " << position << std::endl;
    insert_into_bag(arr_num, position, data);
  }

  /**
   *
   */
  Edge access_bag(int arr_num, int pos) {
    assert(arr_num >= 0 && arr_num < numArrays);
    std::pair<int, int> range = get_array_range(arr_num);

    // TODO FIX HACK HERE
    return Edge((node_id_t)bv.at(get_data_offset(arr_num, pos) + 0),
                (EdgeType)bv.at(get_data_offset(arr_num, pos) + 1),
                (timestamp_t)bv.at(get_data_offset(arr_num, pos) + 2));
  }

  /**
   *
   */
  int get_size(int arr_num) {
    std::pair<int,int> range = get_array_range(arr_num);
    return (range.second - range.first) / Edge::serialized_length();
  }


  /**
   *
   */
  void print() {
    for (int i = 0; i < bv.size(); i++) {
      std::cout << bv.at(i) << " " ;
    }
    std::cout << std::endl;
    for (int i = 0 ; i < numArrays; i++) {
      print(i);
    }
  }


  /**
   *
   */
  void print(int i) {
    std::pair<int, int> range = get_array_range(i);
    std::cout << i << ": [" << range.first << "," << range.second << "]: ";
    for (int p = range.first; p < range.second; p++) {
      std::cout << bv.at(numArrays + p) << " " ;
    }
    std::cout << std::endl;
  }

};


/**
 *
 */
class BlockedGraph {
  typedef std::vector<uint32_t> FlexVector;

  std::vector<BlockArrayGroup> in_bags;
  std::vector<BlockArrayGroup> out_bags;
  FlexVector node_types;
  FlexVector node_ts;

public:

  /**
   *
   */
  BlockedGraph () {
  }

  /**
   *
   */
  ~BlockedGraph () {
  }

  /**
   *
   */
  int number_of_vertices() {
    return node_types.size();
  }

  /**
   *
   */
  VertexType get_vertex_type(node_id_t v) {
    return (VertexType) node_types[v];
  }

  /**
   *
   */
  void insert_vertex(node_id_t vid, VertexType type, timestamp_t ts) {
    assert(vid == number_of_vertices());
    node_types.push_back((uint32_t) type);
    node_ts.push_back((uint32_t) ts);
    if (vid / BAG_SIZE >= in_bags.size()) {
      in_bags.push_back(BlockArrayGroup(BAG_SIZE));
      out_bags.push_back(BlockArrayGroup(BAG_SIZE));
    }
  }

  /**
   *
   */
  void insert_edge(node_id_t v1, node_id_t v2, EdgeType type, timestamp_t ts) {
    in_bags[get_bag_id(v2)].push_back_bag(get_bag_index(v2), Edge(v1, type, ts));
    out_bags[get_bag_id(v1)].push_back_bag(get_bag_index(v1), Edge(v2, type, ts));
  }

  /**
   *
   */
  int get_bag_id(node_id_t v) {
    return v /BAG_SIZE;
  }

  /**
   *
   */
  int get_bag_index(node_id_t v) {
    return v % BAG_SIZE;
  }

  /**
   *
   */
  int get_number_in_neighbours(node_id_t v) {
    return in_bags[get_bag_id(v)].get_size(get_bag_index(v));
  }

  /**
   *
   */
  int get_number_out_neighbours(node_id_t v) {
    return out_bags[get_bag_id(v)].get_size(get_bag_index(v));
  }

  /**
   *
   */
  Edge get_in_neighbour(node_id_t v, node_id_t idx) {
    return in_bags[get_bag_id(v)].access_bag(get_bag_index(v), idx);
  }

  /**
   *
   */
  Edge get_out_neighbour(node_id_t v, node_id_t idx) {
    return out_bags[get_bag_id(v)].access_bag(get_bag_index(v), idx);
  }

  /**
   *
   */
  void extract_connected_components(std::vector<std::vector<node_id_t>> &output) {

    node_id_t n = number_of_vertices();

    bool *visited = new bool[n];

    for(node_id_t i = 0; i < n; i++) {
      visited[i] = false;
    }

    std::list<node_id_t> queue;

    for (node_id_t i = 0; i < n; i++) {

      if (visited[i] == false) {

        visited[i] = true;
        queue.push_back(i);

        std::vector<node_id_t> component;

        while(!queue.empty()) {

          node_id_t current = queue.front();
          component.push_back(current);
          queue.pop_front();

          for(node_id_t i = 0;
              i < get_number_out_neighbours(current);
              i++) {

            Edge out_edge = get_out_neighbour(current, i);

            if(!visited[out_edge.target]) {

              visited[out_edge.target] = true;
              queue.push_back(out_edge.target);

            }
          }
        }
        output.push_back(component);
      }
    }
    delete [] visited;
  }

  /**
   *
   */
  void print_edges() {
    std::cout << "Loaded Graph: " << std::endl;
    for (node_id_t vert = 0; vert < number_of_vertices(); vert++) {
      for (node_id_t i = 0; i < get_number_out_neighbours(vert); i++) {
        Edge e = get_out_neighbour(vert, i);
        std::cout << e.timestamp << " E " << vert << " " << e.target << " " << EdgeEnumToStr(e.type) << std::endl;
      }
    }

  }
};



/**
 *
 */
void load_graph(std::string filename, BlockedGraph *g) {

  std::ifstream infile(filename);
  std::string line;
  while (infile) {
    std::getline(infile,line);
    std::stringstream parser(line);
    uint32_t ts;
    if (parser >> ts) {
      std::string type;
      if (parser >> type) {
        if (type.compare("E") == 0) {
          node_id_t v1,v2;
          std::string type;
          if (parser >> v1 >> v2 >> type) {
            g->insert_edge(v1, v2, EdgeStrToEnum(type), ts);
          } else {
            std::cerr << "Malformed line: " << line << std::endl;
            exit(1);
          }
        } else if (type.compare("V") == 0) {
          node_id_t v;
          std::string type;
          if (parser >> v >> type) {
            g->insert_vertex(v, VertexStrToEnum(type), ts);
          } else {
            std::cerr << "Malformed line: " << line << std::endl;
            exit(1);
          }
        }
      }
    }
  }
}


/**
 *
 */
void write_connected_components(std::vector<std::vector<node_id_t>> &components, std::string const &filename) {
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

  if (argc != 3) {
    std::cout << "Usage: " << argv[0] << " [input_graph] [output_file]" << std::endl;
    exit(1);
  }

  BlockedGraph *g = new BlockedGraph();
  load_graph(std::string(argv[1]), g);

  std::vector<std::vector<node_id_t>> comps;
  g->extract_connected_components(comps);
  write_connected_components(comps, std::string(argv[2]));
  delete g;

}





