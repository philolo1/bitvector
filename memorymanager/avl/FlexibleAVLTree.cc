// based on
// http://ezekiel.vancouver.wsu.edu/~cs223/lectures/avl-trees/c++/AVLTree.cpp

#ifndef LALA_DYNAMIC_BLOCK_AVL_TREE
#define LALA_DYNAMIC_BLOCK_AVL_TREE

#include <cstdio>
#include "../Helper.cc"
#include "../MemoryPool.cc"
#include "../MemoryManager.cc"
#include "FlexibleAVLNode.cc"
#include <tuple>
#include "../datastructures/FlexibleCache.cc"
#include "../defines.h"

// template <class Compressor, class MemManager, class MemPointer>
template <class Compressor, class MemManager, class MemPointer>
class FlexibleAVLTree {

  typedef FlexibleCache<CACHE_SIZE, Compressor, MemManager, MemPointer>
      FlexCache;

private:
  FlexibleAVLNode *root;
  FlexCache *cache;

public:
  FlexibleAVLTree(FlexCache *cache) : root(0), cache(cache) {}

  ~FlexibleAVLTree() { delete root; }

  void print() { root->print(); }


  int getMemorySize() {
      return sizeof(this)  + root->getMemorySize();
  }

  bool empty() const { return root == 0; }
  unsigned size() const { return (root == 0) ? 0 : root->mysize; }
  int height() const { return (root == 0) ? -1 : root->height; }

  // we already assume that updatePath was caleld before
  void deleteElem(int elem) {
    list<FlexibleAVLNode *> path; // path from inserted node back to root

    auto tree = root;
    int add = 0;
    while (tree) {
      if (elem < add + tree->getStartIndex()) {
        path.push_front(tree);
        add += tree->addPos;
        tree = tree->left;
      } else if (elem < add + tree->getStartIndex() + tree->len * CHAR_BIT) {

        FlexibleAVLNode *n = tree; // find node n to delete

        // get the rest of the value added

        tree->propagateAddPos();

        if (n->left == 0 || n->right == 0) { // leaf or parent of leaf

          FlexibleAVLNode *t = (n->left == 0) ? n->right : n->left;
          if (path.empty()) {
            // we delete the root that only has one element,
            // so we set the root to the right or left child
            // TODO DOES SOMETHING NEED TO CHANGE ?????
            root = t;
          } else if (path.front()->left == n) {
            // it is the left child of something
            path.front()->left = t;
          } else {
            // it is the right child child of something
            path.front()->right = t;
          }

          // delete the node then, we don't need it anymore
          n->left = n->right = 0;
          cache->deletePointer(n);
          delete n;
        } else { // internal node with 2 children

          list<FlexibleAVLNode *> path2;

          FlexibleAVLNode *node2 = tree->right;

          int add2 = 0;

          while (node2->left) {
            add2 += node2->addPos;
            path2.push_front(node2);
            node2 = node2->left;
          }

          if (tree->right == node2) {

            //  tree
            //    |
            //    --> node2 (no left child)
            //         |
            //    X  <----> node2->right (might be null)
            //
            tree->right = node2->right;

            // make sure we dont have the addPos Value here
            node2->propagateAddPos();

            // TODO check this again in the morning
            // set all the values but mySize und height
            //
            cache->magic(tree, node2, add + add2 + node2->getElem(), this);
            tree->setToNode(node2, add2);

            // mysize stays the same
            tree->height = 1 + max(height(tree->left), height(tree->right));

          } else {

            // tree
            //  |
            //  -----> path2.back()
            //           |
            //       .. <--
            //       |
            //       path2.front()
            //        |
            //   node2 <-
            //     |
            //  X <--  node2->right()
            //

            node2->propagateAddPos();
            path2.front()->left = node2->right;
            path2.front()->height = 1 + max(height(path2.front()->left),
                                            height(path2.front()->right));

            cache->magic(tree, node2, add + add2 + node2->getElem(), this);
            tree->setToNode(node2, add2);
          }

          // subtract the number of ones we go on the left
          // for( FlexibleAVLNode* treeNode: path2) {
          //   treeNode->mysize -= node2->numOnes;
          // }

          node2->right = nullptr;
          cache->deletePointer(node2);

          delete node2;

          // cout <<"BEF BALANCE" <<endl;
          // print();

          tree->right = rebalance(tree->right, path2);

          // cout<<"BBBBBBBBBBBBBBBBBBBB "<<endl;
          // tree->print(add);

          // cout <<endl <<"BEF AFTER" <<endl;
          // print();
        }

        // balance the tree at the end
        root = rebalance(root, path);

        // we are done now, finally
        return;

      } else if (elem > add + tree->getStartIndex()) {
        path.push_front(tree);
        add += tree->addPos;
        tree = tree->right;
      } else {
        break;
      }
    }
  }

  pair<FlexibleAVLNode *, int> find(int elem, int addTreeSize = 0,
                                    int addPlus = 0) const {
    auto tree = root;
    int add = 0;
    while (tree) {
      tree->mysize += addTreeSize;
      if (elem < add + tree->getStartIndex()) {
        if (addPlus != 0) {
          tree->addIndexRight(addPlus);
        }

        add += tree->addPos;
        tree = tree->left;
      } else if (elem < add + tree->getStartIndex() + tree->len * CHAR_BIT) {
        if (tree->right && addPlus != 0) {
          tree->right->addPos += addPlus;
        }

        tree->numOnes += addTreeSize;
        return make_pair(tree, tree->getStartIndex() + add);
      } else if (elem > add + tree->getStartIndex()) {
        add += tree->addPos;
        tree = tree->right;
      } else {
        break;
      }
    }
    return make_pair(nullptr, 0);
  }

  // add to all the elements along path to elem val p
  void updatePath(int elem, int add) const { find(elem, add); }

  // add to all the elements along path to elem val p
  void updatePath2(int elem, int add, int addPlus) const {
    find(elem, add, addPlus);
  }

  // TODO change
  FlexibleAVLNode *insert(int elem, MemoryPointer *ptr, int numOnes, int len) {

    FlexibleAVLNode *tree = root;
    if (tree == 0) {
      root = new FlexibleAVLNode(elem, ptr, numOnes, len, numOnes);
      return root;
    }

    list<FlexibleAVLNode *> path; // path from inserted node back to root

    FlexibleAVLNode *ret;

    // Phase 1: Insert new element in the usual manner, but //   save the path
    // of visited nodes.  //
    FlexibleAVLNode *dad = tree;
    while (true) {
      path.push_front(dad);
      if (elem < dad->getStartIndex()) {
        elem -= dad->addPos;
        if (dad->left == 0) {
          dad->left = new FlexibleAVLNode(elem, ptr, numOnes, len, numOnes);
          ret = dad->left;
          break;
        }
        dad = dad->left;
      } else if (elem > dad->getStartIndex()) {
        elem -= dad->addPos;
        if (dad->right == 0) {
          dad->right = new FlexibleAVLNode(elem, ptr, numOnes, len, numOnes);
          ret = dad->right;
          break;
        }
        dad = dad->right;
      } else {
        return tree; // duplicate, tree not modified
      }
    }

    // Phase 2: Starting with the last node visited in the
    //   path back to the root do the following:
    //      A. check for an AVL imbalance and perform
    //         a rotation as necessary (only done at most once);
    //      B. Adjust the stored height of each node.
    root = rebalance(tree, path);

    return ret;
  }

  tuple<int, FlexibleAVLNode *, int> rank(int elem) {
    FlexibleAVLNode *tree = root;
    int elements = 0;

    while (tree) {
      if (elem < tree->getStartIndex()) {
        elem -= tree->addPos;
        tree = tree->left;
      } else {
        elements += mysize(tree->left);

        int num = elem - tree->getStartIndex();

        if (num >= tree->len * CHAR_BIT - 1) {

          elements += mysize(tree) - mysize(tree->left) - mysize(tree->right);
          elem -= tree->addPos;
          tree = tree->right;
        } else {

          // tree->elem is not the right value, becasue we ignore the add Plus
          // of the tree here, so be careful
          return make_tuple(elements, tree, num);
        }
      }
    }

    // third value does not matter :P
    return make_tuple(elements, nullptr, 0);
  }

  tuple<int, FlexibleAVLNode *, int> select(int size) {

    FlexibleAVLNode *tree = root;
    size++;

    int pos = 0;

    while (tree) {
      int val = mysize(tree->left);
      int count = mysize(tree) - mysize(tree->left) - mysize(tree->right);

      if (size <= val) {
        tree = tree->left;
      } else if (size <= val + count) {
        return make_tuple(size - val, tree, tree->getStartIndex());
      } else {
        size -= val + count;
        tree = tree->right;
      }
    }

    return make_tuple(-1, nullptr, 0);
  }

  inline int max(int a, int b) { return (a > b) ? a : b; }

  inline int height(FlexibleAVLNode *t) { return t ? t->height : -1; }
  inline int mysize(FlexibleAVLNode *t) { return t ? t->mysize : 0; }

  FlexibleAVLNode *leftRotate(FlexibleAVLNode *tree) { // returns new tree

    //                 +----+                        +-----+
    //                 | k2 |                        | k1  +-------+
    //                 ++---+                        +-----+       |
    //                  |                                          |
    //      +----+      |                                          v
    //      |k1  +<-----+            +------->                 +---+----+
    //      +--+-+                   +------->         +-----+ |   k2   |
    //         |                                       |       +--------+
    //         |                                       |
    //         |         +----+                        v
    //         +-------->+ A  |                    +---+----+
    //                   +----+                    |   A    |
    //                                             +--------+
    //
    FlexibleAVLNode *k2 = tree;
    FlexibleAVLNode *k1 = tree->left;

    k2->propagateAddPos();
    k1->propagateAddPos();

    k2->left = k1->right;
    k1->right = k2;
    k2->height = 1 + max(height(k2->left), height(k2->right));
    k1->height = 1 + max(height(k1->left), k2->height);

    k2->mysize = k2->getOnes() + mysize(k2->left) + mysize(k2->right);
    k1->mysize = k1->getOnes() + mysize(k1->left) + k2->mysize;

    // update addPos
    return k1;
  }

  FlexibleAVLNode *rightRotate(FlexibleAVLNode *tree) { // returns new tree

    //     +-----+                                    +------+
    //     | k2  |                              +-----+  k1  |
    //     +--+--+                              |     +------+
    //        |                                 |
    //        |       +------+                  v
    //        +------>+  k1  |     +-->      +--+-----+
    //                +--+---+     +-->      |   k2   |
    //                   |                   +--+-----+
    //     +-----+       |                      |
    //     | A   | <-----+                      |     +--------+
    //     +-----+                              +---> |  A     |
    //                                                +--------+

    FlexibleAVLNode *k2 = tree;
    FlexibleAVLNode *k1 = tree->right;

    k2->propagateAddPos();
    k1->propagateAddPos();

    k2->right = k1->left;
    k1->left = k2;
    k2->height = 1 + max(height(k2->left), height(k2->right));
    k1->height = 1 + max(k2->height, height(k1->right));

    k2->mysize = k2->getOnes() + mysize(k2->left) + mysize(k2->right);
    k1->mysize = k1->getOnes() + k2->mysize + mysize(k1->right);

    return k1;
  }

  // rebalance ()
  // Given a tree and a path from a node back up to the root,
  // we do the following with each visited node:
  //    A. check for an AVL imbalance and perform
  //       a rotation as necessary;
  //    B. Recompute the stored height of each node.
  //    C. Recompute the summed up number of ones (mySize)
  //
  FlexibleAVLNode *rebalance(FlexibleAVLNode *tree,
                             list<FlexibleAVLNode *> &path) {
    while (!path.empty()) {
      FlexibleAVLNode *dad = path.front();

      path.pop_front();
      const int leftHeight = height(dad->left);
      const int rightHeight = height(dad->right);
      // cout<<"LeftH : "<<leftHeight<<" rightH: "<<rightHeight<<endl;
      const int balance = rightHeight - leftHeight;
      if (balance < -1) {       // left tree too deep
        FlexibleAVLNode **root; // ptr to root of (sub)tree we are rotating

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
        FlexibleAVLNode **root;
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
    const FlexibleAVLNode *node; // may be null
    int i, j; // node addres (i = level, j = child index for level i)
    NodeCoord(const FlexibleAVLNode *n, int i_, int j_)
        : node(n), i(i_), j(j_) {}
  };
};

#endif // ADVANCED_BLOCKAVLTREE_H
