// source http://ezekiel.vancouver.wsu.edu/~cs223/lectures/avl-trees/c++/AVLTree.cpp

#include <list>
#include <iostream>

#include "AVLTree.h"

using namespace std;
using namespace AVL;

#ifndef HELP_FUN
#define HELP_FUN

namespace {  // anonymous private namespace
  inline int max(int a, int b) {return (a > b) ? a : b;}

  inline int height(AVLNode *t) {return t ? t->height : -1;}
  inline int mysize(AVLNode *t) {return t ? t->mysize : 0;}

  AVLNode *leftRotate(AVLNode *tree) {  // returns new tree
    AVLNode *k2 = tree;
    AVLNode *k1 = tree->left;
    k2->left = k1->right;
    k1->right = k2;
    k2->height = 1 + max(height(k2->left), height(k2->right));
    k1->height = 1 + max(height(k1->left), k2->height);

    k2->mysize = 1 + mysize(k2->left) +  mysize(k2->right);
    k1->mysize = 1 + mysize(k1->left) +  k2->mysize;



    return k1;
  }

  AVLNode *rightRotate(AVLNode *tree) {  // returns new tree
    AVLNode *k2 = tree;
    AVLNode *k1 = tree->right;
    k2->right = k1->left;
    k1->left = k2;
    k2->height = 1 + max(height(k2->left), height(k2->right));
    k1->height = 1 + max(k2->height, height(k1->right));

    k2->mysize = 1 + mysize(k2->left) +  mysize(k2->right);
    k1->mysize = 1 + k2->mysize + mysize(k1->right);



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
  AVLNode *rebalance(AVLNode *tree, list<AVLNode*> &path) {
    while (!path.empty()) {
      AVLNode *dad = path.front();
      path.pop_front();
      const int leftHeight = height(dad->left);
      const int rightHeight = height(dad->right);
      const int balance = rightHeight - leftHeight;
      if (balance < -1) { // left tree too deep
        AVLNode **root;   // ptr to root of (sub)tree we are rotating
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
        AVLNode **root;
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
      dad->mysize = 1 + mysize(dad->left) + mysize(dad->right);


    }
    return tree;
  }

  struct NodeCoord { // use by print routine
    const AVLNode *node;   // may be null
    int i, j;        // node addres (i = level, j = child index for level i)
    NodeCoord(const AVLNode *n, int i_, int j_) : node(n), i(i_), j(j_) {}
  };

}

#endif

namespace AVL {

  void print(const AVLNode *tree, FILE *f) {
    if (tree == 0) return;
    fprintf(f, "height = %d\n", tree->height);
    list<NodeCoord> queue;   // queue used for breadth-first traversal
    queue.push_back(NodeCoord(tree, 0, 0));    // insert root into queue
    for (int i = 0; i <= tree->height; i++) {  // for each tree level
      const int n = 1 << i; // number if elements at level i
      const int d = (1 << (tree->height - i)) - 1; // dist between elements
      for (int j = 0; j < n; j++) {  // for each elem on level i
        NodeCoord c = queue.front(); // peek at node at head of queue
        if (c.i == i && c.j == j)    // if tree contains node at (i,j)
          queue.pop_front();         //   remove node from queue
        else
          c.node = 0;                //   else elem (i,j) is nil
        if (j == 0) {                // indent for first element on row
          for (int k = d/2; k--; )
            fprintf(f, "%2c ", ' ');
          if (i < tree->height)
            fprintf(f, "%2c", ' ');
        }
        if (c.node) {                // if elem (i,j) not nil
          fprintf(f, "%2d ", c.node->elem);  // print it
          if (c.node->left)              // and enqueue children
            queue.push_back(NodeCoord(c.node->left, i+1, 2*j));
          if (c.node->right)
            queue.push_back(NodeCoord(c.node->right, i+1, 2*j+1));
        } else
          fprintf(f, "%2c ", '.');       // else mark nil elem with .
        if (j+1 >= n)                // if last elem on level
          fprintf(f, "\n");              //   print new line
        else {                       //   indent for elem (i,j)
          for (int k = d; k--; )
            fprintf(f, "%2c ", ' ');
        }
      }
    }
  }

  unsigned size(const AVLNode *tree) {
    if (tree == 0)
      return 0;
    return tree->mysize;
  }

  bool find(const AVLNode *tree, int elem) {
    while (tree)
      if (elem < tree->elem)
        tree = tree->left;
      else if (elem > tree->elem)
        tree = tree->right;
      else
        break;
    return tree != 0;
  }


  int rank(const AVLNode *tree, int elem) {
    int elements = 0;
    while (tree) {
      if (elem < tree->elem) {
        tree = tree->left;
      }
      else // if (elem >= tree->elem)
      {
        elements += 1 + mysize(tree->left);
        tree = tree->right;
      }
    }

    return elements;
  }


  int select(const AVLNode *tree, int size) {
    // we begin with index = 1
    size++;

    while(tree) {
      int val = mysize(tree->left);
      if (size <= val) {
        tree = tree->left;
      } else if (size == val +1) {
        return tree->elem;
      } else {
        size -= val +1;
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
  AVLNode *insert(AVLNode *tree, int elem) {
    if (tree == 0)
      return new AVLNode(elem);
    
    list<AVLNode*> path;  // path from inserted node back to root
    
    //
    // Phase 1: Insert new element in the usual manner, but
    //   save the path of visited nodes.
    //
    AVLNode *dad = tree;
    while (true) {
      path.push_front(dad);
      if (elem < dad->elem) {
        if (dad->left == 0) {
          dad->left = new AVLNode(elem);
          break;
        } 
        dad = dad->left;
      } else if (elem > dad->elem) {
        if (dad->right == 0) {
          dad->right = new AVLNode(elem);
          break;
        } 
        dad = dad->right;
      } else
        return tree;  // duplicate, tree not modified
    }

    //
    // Phase 2: Starting with the last node visited in the
    //   path back to the root do the following:
    //      A. check for an AVL imbalance and perform
    //         a rotation as necessary (only done at most once);
    //      B. Adjust the stored height of each node.
    //
    tree = rebalance(tree, path);
    
    return tree;
  }
  
  AVLNode *deleteMin(AVLNode *tree, int &elem) {
    if (tree == 0)
      return 0;
    
    list<AVLNode*> path; // path from parent of deleted node to the root
    
    AVLNode *n = tree;   // find node n to delete
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
    
    tree = rebalance(tree, path);  
    return tree;
  }
  
  AVLNode *deleteMax(AVLNode *tree, int &elem) {
    if (tree == 0)
      return 0;
    
    list<AVLNode*> path; // path from parent of deleted node to the root
    
    AVLNode *n = tree;   // find node n to delete
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
    
    tree = rebalance(tree, path);  
    return tree;
  }
  
  AVLNode *deleteElem(AVLNode *tree, int elem, bool &found) {
    list<AVLNode*> path; // path from parent of deleted node to the root
    
    AVLNode *n = tree;   // find node n to delete
    while (n)
      if (elem < n->elem) {
        path.push_front(n);
        n = n->left;
      } else if (elem > n->elem) {
        path.push_front(n);
        n = n->right;
      } else
        break;       // node found!
    
    if (n == 0) {    // node not found
      found = false;
      return tree;
    } else
      found = true;
    
    if (n->left == 0 || n->right == 0) {  // leaf or parent of leaf
      AVLNode *t = (n->left == 0) ? n->right : n->left;
      if (path.empty())
        tree = t;
      else if (path.front()->left == n)
        path.front()->left = t;
      else
        path.front()->right = t;
      n->left = n->right = 0;
      delete n;
    } else {  // internal node with 2 children
      n->right = deleteMin(n->right, n->elem);
      n->height = 1 + max(height(n->left),height(n->right));
      n->mysize = 1 + mysize (n->left) + mysize(n->right);
    }

    tree = rebalance(tree, path);
    return tree;
  }

}
