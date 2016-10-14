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
#include "compression/Simple9Compressor.cc"
#include "compression/SimdCompCompressor.cc"
#include "compression/NoCompressor.cc"
#include "compression/SnappyCompressor.cc"
#include <gtest/gtest.h>
#include "MyTimer.cc"
#include "datastructures/DynamicBitVector.cc"
#include "datastructures/FlexibleBitVector.cc"

#include "MemoryManager.cc"
#include "SimpleMemoryManager.cc"
// #include "AdvancedMemoryManager.cc"
#include "functions/SimpleFunction.cc"
#include "functions/GeometricFunction.cc"

#include <limits.h>

#include <random>
#include <iostream>
#include <functional>
#include <algorithm>

#define BAG_SIZE 64

//////////////////////////////////////////////////////////////
///////////////Wrapper for FlexibleBitVector//////////////////
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////





/**
 * Need insert, push_back, access
 */
template <class Compressor, class MemManager, class MemPointer>
class FlexVector {

  typedef FlexibleBitVector<Compressor, MemManager, MemPointer> FBV;
  FBV *fbv;

public:
  FlexVector() {
    int chunkSize = 256;
    int S = 256 * 32;
    fbv = new FBV(0, chunkSize * sizeof(uint32_t), S * sizeof(uint32_t));
  }
  ~FlexVector() { delete fbv; }

  pair<int, int> getMemorySize() {
    auto a = fbv->getMemorySize();
    return make_pair(sizeof(this) + a.first, a.second);
  }

  // RHS value
  uint32_t at(int pos) {
    uint32_t at = fbv->accessUint32_t(pos);
    // std::cout << "at: " << pos << " -> " << at <<  std::endl;
    return at;
  }
  void push_back(uint32_t val) {
    // std::cout << "push_back: " << val << std::endl;
    insert(size(), val);
  }

  void insert(int pos, uint32_t val) {
    assert(pos <= size());
    // std::cout << "insert: " << pos << " " << val << std::endl;
    fbv->insertUint32_t(pos, val);
  }

  void insertBlock(int pos, char *data, int dataLen) {
    fbv->insertBlock(pos, data, dataLen);
  }

  void modify(int pos, uint32_t val) {
    assert(pos <= size());
    // std::cout << "modify: " << pos << " " << val << std::endl;
    fbv->modifyUint32_t(pos, val);
  }
  int size() { return fbv->getSize(); }
};

/**
 * Wrapper for vector
 */
class FlexVector2 {

  typedef std::vector<uint32_t> FBV;
  FBV *fbv;
  int length;

public:
  FlexVector2() { fbv = new FBV(); }

  ~FlexVector2() { delete fbv; }


  pair<int,int> getMemorySize() {
    return make_pair( sizeof(uint32_t) *fbv->size(), 0);
  }

  // RHS value
  uint32_t at(int pos) {
    uint32_t at = fbv->at(pos);
    // std::cout << "at: " << pos << " -> " << at <<  std::endl;
    return at;
  }
  void push_back(uint32_t val) {
    // std::cout << "push_back: " << val << std::endl;
    fbv->push_back(val);
  }

  void insert(int pos, uint32_t val) {
    assert(pos <= size());
    // std::cout << "insert: " << pos << " " << val << std::endl;
    fbv->insert(fbv->begin() + pos, val);
  }

  void insertBlock(int pos, char *data, int dataLen) {
    uint32_t *intPtr = (uint32_t *)data;

    for (int n = dataLen - 1; n >= 0; n = n - 1) {
      fbv->insert(fbv->begin() + pos, intPtr[n]);
    }
  }

  void modify(int pos, uint32_t val) {
    assert(pos <= size());
    // std::cout << "modify: " << pos << " " << val << std::endl;
    (*fbv)[pos] = val;
  }
  int size() { return fbv->size(); }
};

// Change this to use either structure
typedef FlexVector<NoCompressor, MemoryManager, MemoryPointer> FNoComp;
typedef FlexVector<LZ4Compressor, MemoryManager, MemoryPointer> FLz4;
typedef FlexVector<LZ4CompressorHC, MemoryManager, MemoryPointer> FLz4hc;
typedef FlexVector<SimdCompCompressor, MemoryManager, MemoryPointer> FSimd;
typedef FlexVector<Simple9Compressor, MemoryManager, MemoryPointer> FSimple;
typedef FlexVector<SnappyCompressor, MemoryManager, MemoryPointer> FSnappy;
typedef FlexVector2 FStl;

//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

typedef uint32_t node_id_t;
typedef uint32_t timestamp_t;

/**
 * Enum for all the graph edge types.
 */
enum EdgeType { REPLY_TO, POSTED, VOTED_FOR, COMMENT_ON, MADE_COMMENT };

/**
 * Enum for all the graph vertex types.
 */
enum VertexType { POST, REPLY, COMMENT, USER };

/**
 * Convert a string to its appropriate vertex type.
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
 * Convert an enum type to its corresponding string.
 */
std::string VertexEnumToStr(VertexType v) {
  switch (v) {
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
 * Convert an enum type to its corresponding string.
 */
std::string EdgeEnumToStr(EdgeType e) {
  switch (e) {
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
 * Convert a string to its appropriate edge type.
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
 * Class for representing vertex type and timestamp
 */
class Node {
public:
  VertexType vert;
  timestamp_t timestamp;

  /**
   *
   */
  Node(VertexType v, timestamp_t ts) {
    vert = v;
    timestamp = ts;
  }

  /**
   *
   */
  VertexType get_type() { return vert; }

  /**
   *
   */
  timestamp_t get_timestamp() { return timestamp; }
};

/**
 *
 */
class Edge {
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
    target = (node_id_t) * (start);
    type = (EdgeType) * (start + 1);
    timestamp = (timestamp_t) * (start + 2);
  }

  /**
   *
   */
  static int serialized_length() { return sizeof(Edge) / sizeof(uint32_t); }
};

/**
 * Use one vector to represent k separate vectors of graph Edge
 * objects.  We do this for the compression algorithms since Writes
 * the lengths of each vector to the prefix
 */
template <class Vector> class BlockArrayGroup {

  Vector *bv;
  int numArrays;

public:
  /**
   *
   */
  BlockArrayGroup(int na, vector<Vector *> *vec) {
    bv = new Vector();
    vec->push_back(bv);
    numArrays = na;
    for (int i = 0; i < numArrays; i++) {
      bv->push_back(0);
    }
    for (int i = 0; i < numArrays; i++) {
      get_array_range(i);
    }
  }

  ~BlockArrayGroup() { delete bv; }

  /**
   * returns uint32 positions of bag bounaries
   */
  std::pair<int, int> get_array_range(int arr_num) {
    assert(arr_num >= 0 && arr_num < numArrays);
    assert(bv->size() > arr_num);

    int left = numArrays;
    int right = numArrays;

    // TODO: need a bulk read here to avoid walking down tree
    for (int i = 0; i <= arr_num; i++) {
      if (arr_num == i)
        left = right;
      right += (int)(((int)bv->at(i)) * ((int)Edge::serialized_length()));
    }

    // std::cout << "get_array_range: " << arr_num << ": (" << (int)left << ","
    //           << (int)right << ")" << std::endl;

    return std::make_pair(left, right);
  }

  /**
   * pos; unit is Edges
   */
  int is_valid_pos(int arr_num, int pos) { return pos <= get_size(arr_num); }

  /**
   * pos; unit is Edges
   */
  int get_data_offset(int arr_num, int pos) {
    std::pair<int, int> range = get_array_range(arr_num);
    return range.first + pos * sizeof(Edge) / sizeof(uint32_t);
  }

  /**
   * arr_num between [0,numArrays)
   * pos between [0,get_size(arr_num)); unit is Edges
   */
  void insert_into_bag(int arr_num, int pos, Edge data) {
    assert(arr_num >= 0 && arr_num < numArrays);
    assert(is_valid_pos(arr_num, pos));

    bv->insertBlock(get_data_offset(arr_num, pos), (char *)&data,
                    sizeof(Edge) / sizeof(uint32_t));

    bv->modify(arr_num, bv->at(arr_num) + (uint32_t)1);
  }

  /**
   *
   */
  void push_back_bag(int arr_num, Edge data) {
    assert(arr_num >= 0 && arr_num < numArrays);
    std::pair<int, int> range = this->get_array_range(arr_num);
    int position = (range.second - range.first) / Edge::serialized_length();
    // std::cout << "Inserting into bag: " << arr_num << " @ position " <<
    // position << std::endl;
    insert_into_bag(arr_num, position, data);
  }

  /**
   *
   */
  Edge access_bag(int arr_num, int pos) {
    assert(arr_num >= 0 && arr_num < numArrays);
    std::pair<int, int> range = get_array_range(arr_num);

    // TODO: Need some kind of iterator for bulk reading
    return Edge((node_id_t)bv->at(get_data_offset(arr_num, pos) + 0),
                (EdgeType)bv->at(get_data_offset(arr_num, pos) + 1),
                (timestamp_t)bv->at(get_data_offset(arr_num, pos) + 2));
  }

  /**
   *
   */
  int get_size(int arr_num) {
    std::pair<int, int> range = get_array_range(arr_num);
    return (range.second - range.first) / (sizeof(Edge) / sizeof(uint32_t));
  }

  /**
   *
   */
  void print() {
    for (int i = 0; i < bv->size(); i++) {
      std::cout << bv->at(i) << " ";
    }
    std::cout << std::endl;
    for (int i = 0; i < numArrays; i++) {
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
      std::cout << bv->at(numArrays + p) << " ";
    }
    std::cout << std::endl;
  }
};

template <class T> class DeleteForEach {
public:
  bool operator()(T x) const {
    delete x;
    return true;
  }
};

/**
 *
 */
template <class Vector> class BlockedGraph {

  // We should be able to use vectors here since these vectors are an
  // order of magnitude smaller than the data: if there are E edges,
  // these will store no more than 2E/BAG_SIZE pointers (I think
  // vectors just do doubling).
  std::vector<BlockArrayGroup<Vector> *> in_bags;
  std::vector<BlockArrayGroup<Vector> *> out_bags;

  // These should be FlexibleVectors since the number of nodes (V) is
  // roughly proportional to the number of edges (E).  V is about E/3.
  Vector *node_types;
  Vector *node_ts;


  vector<Vector *> *v;

public:
  /**
   *
   */
  BlockedGraph(vector<Vector *> *vec) {
    node_types = new Vector();
    node_ts = new Vector();

    v = vec;
    v->push_back(node_types);
    v->push_back(node_ts);

  }

  /**
   *
   */
  ~BlockedGraph() {
    for_each(in_bags.begin(), in_bags.end(),
             DeleteForEach<BlockArrayGroup<Vector> *>());
    for_each(out_bags.begin(), out_bags.end(),
             DeleteForEach<BlockArrayGroup<Vector> *>());
    delete node_types;
    delete node_ts;
  }

  /**
   *
   */
  int number_of_vertices() { return node_types->size(); }

  /**
   *
   */
  VertexType get_vertex_type(node_id_t v) {
    return (VertexType)node_types->at(v);
  }

  /**
   *
   */
  void insert_vertex(node_id_t vid, VertexType type, timestamp_t ts) {
    assert(vid == number_of_vertices());
    node_types->push_back((uint32_t)type);
    node_ts->push_back((uint32_t)ts);
    if (vid / BAG_SIZE >= in_bags.size()) {
      in_bags.push_back(new BlockArrayGroup<Vector>(BAG_SIZE, v));
      out_bags.push_back(new BlockArrayGroup<Vector>(BAG_SIZE, v));
    }
  }

  /**
   *
   */
  void insert_edge(node_id_t v1, node_id_t v2, EdgeType type, timestamp_t ts) {
    in_bags[get_bag_id(v2)]->push_back_bag(get_bag_index(v2),
                                           Edge(v1, type, ts));
    out_bags[get_bag_id(v1)]->push_back_bag(get_bag_index(v1),
                                            Edge(v2, type, ts));
  }

  /**
   *
   */
  int get_bag_id(node_id_t v) { return v / BAG_SIZE; }

  /**
   *
   */
  int get_bag_index(node_id_t v) { return v % BAG_SIZE; }

  /**
   *
   */
  int get_number_in_neighbours(node_id_t v) {
    return in_bags[get_bag_id(v)]->get_size(get_bag_index(v));
  }

  /**
   *
   */
  int get_number_out_neighbours(node_id_t v) {
    return out_bags[get_bag_id(v)]->get_size(get_bag_index(v));
  }

  /**
   *
   */
  Edge get_in_neighbour(node_id_t v, node_id_t idx) {
    return in_bags[get_bag_id(v)]->access_bag(get_bag_index(v), idx);
  }

  /**
   *
   */
  Edge get_out_neighbour(node_id_t v, node_id_t idx) {
    return out_bags[get_bag_id(v)]->access_bag(get_bag_index(v), idx);
  }

  /**
   * This computes the connected components of the graph and outputs
   * them in a vector of vectors.
   *
   * If we are really feeling crazy we can also replace output here
   * with a FlexibleBitVector.
   */
  void
  extract_connected_components(std::vector<std::vector<node_id_t>> &output) {

    node_id_t n = number_of_vertices();

    bool *visited = new bool[n];

    for (node_id_t i = 0; i < n; i++) {
      visited[i] = false;
    }

    std::list<node_id_t> queue;

    for (node_id_t i = 0; i < n; i++) {

      if (visited[i] == false) {

        visited[i] = true;
        queue.push_back(i);

        std::vector<node_id_t> component;

        while (!queue.empty()) {

          node_id_t current = queue.front();
          component.push_back(current);
          queue.pop_front();

          for (node_id_t i = 0; i < get_number_out_neighbours(current); i++) {

            Edge out_edge = get_out_neighbour(current, i);

            if (!visited[out_edge.target]) {

              visited[out_edge.target] = true;
              queue.push_back(out_edge.target);
            }
          }
        }
        output.push_back(component);
      }
    }
    delete[] visited;
  }

  /**
   *
   */
  void print_edges() {
    std::cout << "Loaded Graph: " << std::endl;
    for (node_id_t vert = 0; vert < number_of_vertices(); vert++) {
      for (node_id_t i = 0; i < get_number_out_neighbours(vert); i++) {
        Edge e = get_out_neighbour(vert, i);
        std::cout << e.timestamp << " E " << vert << " " << e.target << " "
                  << EdgeEnumToStr(e.type) << std::endl;
      }
    }
  }
};

/**
 * Open the file located at filename, and insert the vertices and
 * edges into the specified BlockedGraph.
 */
template <class Vector>
void load_graph(std::string filename, BlockedGraph<Vector> *g) {

  std::ifstream infile(filename);
  std::string line;
  while (infile) {
    std::getline(infile, line);
    std::stringstream parser(line);
    uint32_t ts;
    if (parser >> ts) {
      std::string type;
      if (parser >> type) {
        if (type.compare("E") == 0) {
          node_id_t v1, v2;
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
 * Write the connected components stored in vector of vectors in a
 * file.  One component per line.
 */
void write_connected_components(std::vector<std::vector<node_id_t>> &components,
                                std::string const &filename) {
  std::ofstream outfile;
  outfile.open(filename);
  for (auto next_vec_it = components.begin(); next_vec_it != components.end();
       next_vec_it++) {
    std::vector<node_id_t> next_vec = *next_vec_it;
    outfile << next_vec.size() << " ";
    for (auto it = next_vec.begin(); it != next_vec.end(); it++) {
      outfile << (*it) << " ";
    }
    outfile << std::endl;
  }

  outfile.close();
}

template <class Vector> void init(string input, string output) {
  MyTimer timer;
  vector<Vector *> vec;
  BlockedGraph<Vector> *g = new BlockedGraph<Vector>(&vec);
  load_graph(input, g);

  std::cout << "FLEXIBLE BITVECTORS : " << vec.size() << std::endl;

  pair<int, int> size = make_pair(0, 0);;
  for (int n=0; n < vec.size(); n++) {
    pair<int, int> a = vec[n]->getMemorySize();
    size.first += a.first;
    size.second+= a.second;

  }

  std::cout << "FLEXIBLE BITVECTOR SIZE : "
  << Helper::FormatWithCommas(size.first) <<" "
  <<Helper::FormatWithCommas(size.second) << std::endl;

  Helper::measureSpace();

  timer.printTime("INIT GRAPH");

  std::vector<std::vector<node_id_t>> comps;
  g->extract_connected_components(comps);
  write_connected_components(comps, output);

  Helper::measureSpace();
  MemoryProvider<NoCompressor, MemoryManager, MemoryPointer> p;
  timer.printTime("BFS");

  delete g;
}

/**
 * Read the graph in the file specified by first argument.  Output the
 * connected components to a file specified by the second argument.
 */
int main(int argc, char **argv) {

 if (argc != 4) {

    std::cout << "Usage: " << argv[0]
              << " [nocomp|lz4|lzhc|simd|simple] [input_graph] [output_file]" << std::endl;

    exit(1);
  }

   cout <<std::string(argv[1])<<endl;

  if (std::string(argv[1]) == "nocomp") {
    init<FNoComp>(std::string(argv[2]), std::string(argv[3]));
  } else if (std::string(argv[1]) == "lz4") {
    init<FLz4>(std::string(argv[2]), std::string(argv[3]));
  } else if (std::string(argv[1]) == "lz4hc") {
    init<FLz4hc>(std::string(argv[2]), std::string(argv[3]));
  } else if (std::string(argv[1]) == "simple") {
    init<FSimple>(std::string(argv[2]), std::string(argv[3]));
  } else if (std::string(argv[1]) == "stl") {
    init<FStl>(std::string(argv[2]), std::string(argv[3]));
  }
else if (std::string(argv[1]) == "simd") {
    init<FSimd>(std::string(argv[2]), std::string(argv[3]));
}
else if (std::string(argv[1]) == "snappy") {
    init<FSnappy>(std::string(argv[2]), std::string(argv[3]));
}





  else {
    init<FStl>(std::string(argv[2]), std::string(argv[3]));
  }
}
