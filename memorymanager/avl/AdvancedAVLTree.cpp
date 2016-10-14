// source
// http://ezekiel.vancouver.wsu.edu/~cs223/lectures/avl-trees/c++/AVLTree.cpp

#ifndef ADVANCED_AVL_TREE
#define ADVANCED_AVL_TREE

#include <list>
#include <iostream>

#include "AdvancedAVLTree.h"
#include "../Helper.cc"

using namespace std;
using namespace AVL2;

namespace AVLHELP { // anonymous private namespace
inline int max(int a, int b) { return (a > b) ? a : b; }

inline int height(AdvancedAVLNode *t) { return t ? t->height : -1; }
inline int mysize(AdvancedAVLNode *t) { return t ? t->mysize : 0; }

AdvancedAVLNode *leftRotate(AdvancedAVLNode *tree) { // returns new tree
  AdvancedAVLNode *k2 = tree;
  AdvancedAVLNode *k1 = tree->left;
  k2->left = k1->right;
  k1->right = k2;
  k2->height = 1 + max(height(k2->left), height(k2->right));
  k1->height = 1 + max(height(k1->left), k2->height);

  k2->mysize = k2->getOnes() + mysize(k2->left) + mysize(k2->right);
  k1->mysize = k1->getOnes() + mysize(k1->left) + k2->mysize;

  return k1;
}

AdvancedAVLNode *rightRotate(AdvancedAVLNode *tree) { // returns new tree
  AdvancedAVLNode *k2 = tree;
  AdvancedAVLNode *k1 = tree->right;
  k2->right = k1->left;
  k1->left = k2;
  k2->height = 1 + max(height(k2->left), height(k2->right));
  k1->height = 1 + max(k2->height, height(k1->right));

  k2->mysize = k2->getOnes() + mysize(k2->left) + mysize(k2->right);
  k1->mysize = k1->getOnes() + k2->mysize + mysize(k1->right);

  return k1;
}

//
// rebalance ()
// Given a tree and a path from a node back up to the root,
// we do the following with each visited node:
//    A. check for an AVL imbalance and perform
//       a rotation as necessary;
//    B. Recompute the stored height of each node.
//
AdvancedAVLNode *rebalance(AdvancedAVLNode *tree,
                           list<AdvancedAVLNode *> &path) {
  while (!path.empty()) {
    AdvancedAVLNode *dad = path.front();
    path.pop_front();
    const int leftHeight = height(dad->left);
    const int rightHeight = height(dad->right);
    const int balance = rightHeight - leftHeight;
    if (balance < -1) {       // left tree too deep
      AdvancedAVLNode **root; // ptr to root of (sub)tree we are rotating
      if (path.empty())
        root = &tree;
      else if (path.front()->right == dad)
        root = &path.front()->right;
      else
        root = &path.front()->left;
      if (height(dad->left->right) > height(dad->left->left)) // double rot
        dad->left = rightRotate(dad->left);
      dad = leftRotate(dad);
      *root = dad;
    } else if (balance > +1) { // right tree too deep
      AdvancedAVLNode **root;
      if (path.empty())
        root = &tree;
      else if (path.front()->right == dad)
        root = &path.front()->right;
      else
        root = &path.front()->left;
      if (height(dad->right->left) > height(dad->right->right)) // double rot
        dad->right = leftRotate(dad->right);
      dad = rightRotate(dad);
      *root = dad;
    }
    dad->height = 1 + max(height(dad->left), height(dad->right));
    dad->mysize = dad->getOnes() + mysize(dad->left) + mysize(dad->right);
  }
  return tree;
}

struct NodeCoord {             // use by print routine
  const AdvancedAVLNode *node; // may be null
  int i, j; // node addres (i = level, j = child index for level i)
  NodeCoord(const AdvancedAVLNode *n, int i_, int j_) : node(n), i(i_), j(j_) {}
};
}

namespace AVL2 {
void print(const AdvancedAVLNode *tree, FILE *f) {
  if (tree == 0)
    return;
  fprintf(f, "height = %d\n", tree->height);
  list<AVLHELP::NodeCoord> queue; // queue used for breadth-first traversal
  queue.push_back(AVLHELP::NodeCoord(tree, 0, 0)); // insert root into queue
  for (int i = 0; i <= tree->height; i++) {        // for each tree level
    const int n = 1 << i; // number if elements at level i
    const int d = (1 << (tree->height - i)) - 1; // dist between elements
    for (int j = 0; j < n; j++) {                // for each elem on level i
      AVLHELP::NodeCoord c = queue.front(); // peek at node at head of queue
      if (c.i == i && c.j == j)             // if tree contains node at (i,j)
        queue.pop_front();                  //   remove node from queue
      else
        c.node = 0; //   else elem (i,j) is nil
      if (j == 0) { // indent for first element on row
        for (int k = d / 2; k--;)
          fprintf(f, "%2c ", ' ');
        if (i < tree->height)
          fprintf(f, "%2c", ' ');
      }
      if (c.node) {                       // if elem (i,j) not nil
        fprintf(f, "%2d ", c.node->elem); // print it
        if (c.node->left)                 // and enqueue children
          queue.push_back(AVLHELP::NodeCoord(c.node->left, i + 1, 2 * j));
        if (c.node->right)
          queue.push_back(AVLHELP::NodeCoord(c.node->right, i + 1, 2 * j + 1));
      } else
        fprintf(f, "%2c ", '.'); // else mark nil elem with .
      if (j + 1 >= n)            // if last elem on level
        fprintf(f, "\n");        //   print new line
      else {                     //   indent for elem (i,j)
        for (int k = d; k--;)
          fprintf(f, "%2c ", ' ');
      }
    }
  }
}

unsigned size(const AdvancedAVLNode *tree) {
  if (tree == 0)
    return 0;
  return tree->mysize;
}

bool find(const AdvancedAVLNode *tree, int elem) {
  while (tree)
    if (elem < tree->elem)
      tree = tree->left;
    else if (elem <= tree->elem + 31) {
      int pos = elem - tree->elem;
      return (tree->data >> pos) & 1;
    } else if (elem > tree->elem)
      tree = tree->right;
    else
      break;
  return tree != 0;
}

int rank(AdvancedAVLNode *tree, int elem) {
  int elements = 0;

  // cout<<"rank "<<elem <<endl;
  while (tree) {
    // cout<<"tree-> elem : "<<tree->elem<<endl;
    if (elem < tree->elem) {
      tree = tree->left;
    } else {
      // cout<<"case x "<<endl;
      // cout<<"elements : "<<elements<<endl;
      // cout<<"pos : "<<(elem - tree->elem)<<endl;
      // cout<<"tree data : "<<(tree->data)<<endl;
      //
      // cout<<"POS : "<<(elem - tree->elem)<<endl;

      elements +=
          tree->getOnes2(elem - tree->elem) + AVLHELP::mysize(tree->left);
      tree = tree->right;
    }
  }
  return elements;
}

int select(AdvancedAVLNode *tree, int size) {
  // we begin with index = 1
  // cout<<"select : "<<size<<endl;
  size++;

  // cout<<"start"<<endl;
  while (tree) {
    int val = AVLHELP::mysize(tree->left);
    int count = tree->getOnes();
    // cout<<val<<" "<<count<<" "<<size<<" data : "<<(tree->data)<<endl;
    //
    // for (int n=0; n< 32; n++) {
    //   cout<<"pos["<<n<<"] : "<<((tree->data >> n)&1) <<endl;
    // }

    if (size <= val) {
      tree = tree->left;
    } else if (size <= val + count) {

      // cout <<"POS2 " << size - val << " "<<endl;

      return tree->elem + Helper::getXOnePos(tree->data, size - val);
    } else {
      size -= val + count;
      tree = tree->right;
    }
  }

  return -1;
}

//
// Iterative version of AVL insert
// Duplicate elements are *not* stored.
// Two phases:
//   (1) Perform the usual binary search tree insertion, but save
//       in a list all the nodes we visited.
//   (2) Starting with the last node we visited we traverse
//       back up the tree towards the root; We check for
//       AVL imbalances and perform any necessary rotations
//       (the first rotation will rebalance the entire tree);
//       We also adjust the height of each node.
//
AdvancedAVLNode *insert(AdvancedAVLNode *tree, int elem, int data) {
  if (tree == 0)
    return new AdvancedAVLNode(elem, data);

  list<AdvancedAVLNode *> path; // path from inserted node back to root

  //
  // Phase 1: Insert new element in the usual manner, but
  //   save the path of visited nodes.
  //
  AdvancedAVLNode *dad = tree;
  while (true) {
    path.push_front(dad);
    if (elem < dad->elem) {
      if (dad->left == 0) {
        dad->left = new AdvancedAVLNode(elem, data);
        break;
      }
      dad = dad->left;
    } else if (elem > dad->elem) {
      if (dad->right == 0) {
        dad->right = new AdvancedAVLNode(elem, data);
        break;
      }
      dad = dad->right;
    } else
      return tree; // duplicate, tree not modified
  }

  //
  // Phase 2: Starting with the last node visited in the
  //   path back to the root do the following:
  //      A. check for an AVL imbalance and perform
  //         a rotation as necessary (only done at most once);
  //      B. Adjust the stored height of each node.
  //
  tree = AVLHELP::rebalance(tree, path);

  return tree;
}

AdvancedAVLNode *deleteMin(AdvancedAVLNode *tree, int &elem) {
  if (tree == 0)
    return 0;

  list<AdvancedAVLNode *> path; // path from parent of deleted node to the root

  AdvancedAVLNode *n = tree; // find node n to delete
  while (n->left) {
    path.push_front(n);
    n = n->left;
  }

  if (tree == n)
    tree = n->right;
  else
    path.front()->left = n->right;
  elem = n->elem;
  n->right = 0;
  delete n;

  tree = AVLHELP::rebalance(tree, path);
  return tree;
}

AdvancedAVLNode *deleteMax(AdvancedAVLNode *tree, int &elem) {
  if (tree == 0)
    return 0;

  list<AdvancedAVLNode *> path; // path from parent of deleted node to the root

  AdvancedAVLNode *n = tree; // find node n to delete
  while (n->right) {
    path.push_front(n);
    n = n->right;
  }

  if (tree == n)
    tree = n->left;
  else
    path.front()->right = n->left;
  elem = n->elem;
  n->left = 0;
  delete n;

  tree = AVLHELP::rebalance(tree, path);
  return tree;
}

AdvancedAVLNode *deleteElem(AdvancedAVLNode *tree, int elem, bool &found) {
  list<AdvancedAVLNode *> path; // path from parent of deleted node to the root

  AdvancedAVLNode *n = tree; // find node n to delete
  while (n)
    if (elem < n->elem) {
      path.push_front(n);
      n = n->left;
    } else if (elem > n->elem) {
      path.push_front(n);
      n = n->right;
    } else
      break; // node found!

  if (n == 0) { // node not found
    found = false;
    return tree;
  } else
    found = true;

  if (n->left == 0 || n->right == 0) { // leaf or parent of leaf
    AdvancedAVLNode *t = (n->left == 0) ? n->right : n->left;
    if (path.empty())
      tree = t;
    else if (path.front()->left == n)
      path.front()->left = t;
    else
      path.front()->right = t;
    n->left = n->right = 0;
    delete n;
  } else { // internal node with 2 children
    n->right = deleteMin(n->right, n->elem);
    n->height = 1 + max(AVLHELP::height(n->left), AVLHELP::height(n->right));
    n->mysize =
        n->getOnes() + AVLHELP::mysize(n->left) + AVLHELP::mysize(n->right);
  }

  tree = AVLHELP::rebalance(tree, path);
  return tree;
}

void modify(AdvancedAVLNode *tree, int elem, int val) {
  list<AdvancedAVLNode *> nodes;
  while (tree) {
    nodes.push_front(tree);
    if (elem < tree->elem) {
      tree = tree->left;
    } else if (elem <= tree->elem + 31) {
      int pos = elem - tree->elem;

      if (val == 1) {
        tree->setOne(pos);
      } else {
        tree->setZero(pos);
      }
      break;
    } else if (elem > tree->elem) {
      tree = tree->right;
    } else {
      return;
    }
  }

  // update mySize
  for (AdvancedAVLNode *node : nodes) {
    node->mysize = node->getOnes() + AVLHELP::mysize(node->left) +
                   AVLHELP::mysize(node->right);
  }
}
}

#endif
