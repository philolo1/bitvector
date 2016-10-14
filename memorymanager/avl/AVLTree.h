// based on http://ezekiel.vancouver.wsu.edu/~cs223/lectures/avl-trees/c++/AVLTree.cpp

#ifndef AVLTREE_H
#define AVLTREE_H

#include <cstdio>

namespace AVL {

  struct AVLNode {
    int elem;         // payload
    int height;       // cached height of tree
    int mysize;
    AVLNode *left, *right;
    AVLNode(int e) : elem(e), height(0), left(0), right(0), mysize(1) {}
    ~AVLNode() {delete left; delete right;}
  };
  
  void print(const AVLNode *tree, FILE *f);
  unsigned size(const AVLNode *tree);
  bool find(const AVLNode *tree, int elem);
  int rank(const AVLNode *tree, int elem);
  int select(const AVLNode *tree, int elem);
  AVLNode *insert(AVLNode *tree, int elem);
  AVLNode *deleteMin(AVLNode *tree, int &elem);
  AVLNode *deleteMax(AVLNode *tree, int &elem);
  AVLNode *deleteElem(AVLNode *tree, int elem, bool &found);

}

class AVLTree {
private:
  AVL::AVLNode *root;
public:
  AVLTree() : root(0) {}
  ~AVLTree() {delete root;}
  bool empty() const {return root == 0;}
  unsigned size() const {return AVL::size(root);}
  int height() const {return (root == 0) ? -1 : root->height;}
  void print(FILE *f = stdout) const {AVL::print(root, f);}
  bool find(int elem) const {return AVL::find(root, elem);}
  void insert(int elem) {root = AVL::insert(root, elem);}
  bool deleteMin(int &elem) {return AVL::deleteMin(root, elem) != 0;}
  bool deleteMax(int &elem) {return AVL::deleteMax(root, elem) != 0;}
  bool deleteElem(int elem) {
    bool found; root = AVL::deleteElem(root, elem, found); return found;
  }

  int rank(int elem) { return AVL::rank(root, elem); }
  int select(int elem) { return AVL::select(root, elem); }

};

#endif // AVLTREE_H
