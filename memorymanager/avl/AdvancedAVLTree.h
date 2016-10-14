// based on http://ezekiel.vancouver.wsu.edu/~cs223/lectures/avl-trees/c++/AdvancedAVLTree.cpp

#ifndef ADVANCED_AVLTREE_H
#define ADVANCED_AVLTREE_H

#include <cstdio>
#include "../Helper.cc"

namespace AVL2 {

  struct AdvancedAVLNode {
    int elem;         // payload
    int data;         // save the int here
    int height;       // cached height of tree
    int mysize;

    void setOne(int pos) {
      data = Helper::setOne(data, pos);
    }

    void setZero(int pos) {
      data = Helper::setZero(data, pos);
    }

    inline int getOnes() {
      return Helper::countInt(data);
    }

    inline int getOnes2(int num) {
      if (num >= 31) {
        return Helper::countInt(data);
      } else {
        int help = (1 <<(num+1)) -1;
        // cout<<"help : "<<help<<endl;
        // cout<<"data : "<<data<<endl;
        // cout<<"k : "<<(data&1)<<endl;
        return Helper::countInt(data & help);
      }
    }
    AdvancedAVLNode *left, *right;
    AdvancedAVLNode(int e, int data) : elem(e), data(data), height(0), left(0), right(0), mysize(Helper::countInt(data)) {

    }
    ~AdvancedAVLNode() {delete left; delete right;}
  };
  
  void print(const AdvancedAVLNode *tree, FILE *f);
  unsigned size(const AdvancedAVLNode *tree);
  bool find(const AdvancedAVLNode *tree, int elem);
  int rank(AdvancedAVLNode *tree, int elem);
  int select(AdvancedAVLNode *tree, int elem);
  void modify(AdvancedAVLNode *tree, int elem, int val);
  AdvancedAVLNode *insert(AdvancedAVLNode *tree, int elem, int data);
  AdvancedAVLNode *deleteMin(AdvancedAVLNode *tree, int &elem);
  AdvancedAVLNode *deleteMax(AdvancedAVLNode *tree, int &elem);
  AdvancedAVLNode *deleteElem(AdvancedAVLNode *tree, int elem, bool &found);

}

class AdvancedAVLTree {
private:
  AVL2::AdvancedAVLNode *root;
public:
  AdvancedAVLTree() : root(0) {}
  ~AdvancedAVLTree() {delete root;}
  bool empty() const {return root == 0;}
  unsigned size() const {return AVL2::size(root);}
  int height() const {return (root == 0) ? -1 : root->height;}
  void print(FILE *f = stdout) const {AVL2::print(root, f);}
  bool find(int elem) const {return AVL2::find(root, elem);}
  void insert(int elem, int data) {root = AVL2::insert(root, elem, data);}
  bool deleteMin(int &elem) {return AVL2::deleteMin(root, elem) != 0;}
  bool deleteMax(int &elem) {return AVL2::deleteMax(root, elem) != 0;}
  bool deleteElem(int elem) {
    bool found; root = AVL2::deleteElem(root, elem, found); return found;
  }

  int rank(int elem) { return AVL2::rank(root, elem); }
  int select(int elem) { return AVL2::select(root, elem); }
  void modify(int elem, int val) { AVL2::modify(root, elem, val); }


};

#endif // ADVANCED_AVL2TREE_H
