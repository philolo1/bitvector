// based on http://ezekiel.vancouver.wsu.edu/~cs223/lectures/avl-trees/c++/AVLTree.cpp

#ifndef BLOCK_AVL_TREE
#define BLOCK_AVL_TREE

#include <cstdio>
#include "../Helper.cc"
#include "../MemoryPool.cc"
#include "../MemoryManager.cc"
#include "BlockAVLNode.cc"


class BlockAVLTree {
private:
  BlockAVLNode *root;


public:

  BlockAVLTree() : root(0) {
  }

  ~BlockAVLTree() {
    delete root;
  }

  bool empty() const {
    return root == 0;
  }
  unsigned size() const {
    return (root == 0) ? 0 : root->mysize;
  }
  int height() const {
    return (root == 0) ? -1 : root->height;
  }

  BlockAVLNode* find(int elem) const {
    auto tree = root;
// std::cout<<"p find "<<elem<<" "<<tree->elem<< " "<<(tree->len * CHAR_BIT)<<std::endl;
    while (tree) {
      if (elem < tree->elem) {
//std::cout<<"p left"<<std::endl;
        tree = tree->left;
      }
      else if (elem < tree->elem + tree->len * CHAR_BIT )  {
// std::cout<<"p stop"<<std::endl;
          return tree;
        // TODO delete after making sure it works
        // int pos = elem - tree->elem;
        // char *arr = manager->getMemoryLocation(tree->ptr);
        // return (arr[pos/INT_BIT] >> (pos % INT_BIT)) & 1;
      }
      else if (elem > tree->elem) {
// std::cout<<"p right"<<std::endl;
        tree = tree->right;
      } else {
        break;
      }
    }

// std::cout<<"p finish"<<std::endl;
    return NULL;
   }
 
  // add to all the elements along path to elem val p
  void updatePath(int elem, int add) const {
    auto tree = root;
    while (tree) {
      tree->mysize += add;
      if (elem < tree->elem) {
        tree = tree->left;
      }
      else if (elem < tree->elem + tree->len * CHAR_BIT )  {
        return;
      }
      else if (elem > tree->elem) {
        tree = tree->right;
      } else {
        break;
      }
    }
   }


  BlockAVLNode *insert(int elem, MemoryPointer *ptr,  int numOnes, int len) {


    BlockAVLNode *tree = root;
    if (tree == 0) {
      root = new BlockAVLNode(elem, ptr, numOnes, len, numOnes);
      return root;
    }

    list<BlockAVLNode*> path;  // path from inserted node back to root

    //
    // Phase 1: Insert new element in the usual manner, but //   save the path of visited nodes.  //
    BlockAVLNode *dad = tree;
    while (true) {
      path.push_front(dad);
      if (elem < dad->elem) {
// std::cout<<"p left"<<std::endl;
        if (dad->left == 0) {
          dad->left = new BlockAVLNode(elem, ptr, numOnes, len, numOnes);
          break;
        } 
        dad = dad->left;
      } else if (elem > dad->elem) {
// std::cout<<"p right"<<std::endl;
        if (dad->right == 0) {
          dad->right = new BlockAVLNode(elem, ptr, numOnes, len, numOnes);
          break;
        } 
        dad = dad->right;
      } else {
// std::cout<<"p duplicate"<<std::endl;
        return tree;  // duplicate, tree not modified
      }
    }

    //
    // Phase 2: Starting with the last node visited in the
    //   path back to the root do the following:
    //      A. check for an AVL imbalance and perform
    //         a rotation as necessary (only done at most once);
    //      B. Adjust the stored height of each node.
    //
    root = rebalance(tree, path);

    return tree;
  }


 pair<int, BlockAVLNode*> rank( int elem) {
   BlockAVLNode *tree = root;
    int elements = 0;
//cout<<"rank : "<<elem<<endl;

    while (tree) {
      if (elem < tree->elem) {
//std::cout<<"p left"<<std::endl;
        tree = tree->left;
      } else {
//std::cout<<"p right"<<std::endl;
        elements +=  mysize(tree->left);

     
        int num = elem - tree->elem;

        if (num >= tree->len * CHAR_BIT -1) {
//std::cout<<"p HERE :"<<std::endl;

          //cout<<" RES: " <<mysize(tree) << mysize(tree->left) <<" "<< mysize(tree->right) <<endl;
          elements += mysize(tree) - mysize(tree->left) - mysize(tree->right);

//std::cout<<"p HERE 2:"<<elements<<std::endl;
          tree = tree->right;
        } else {
//std::cout<<"p left_CHOICE"<<std::endl;
//cout<<"elem : "<<elements<<endl;

          return make_pair(elements, tree);
          // int * intArr = (int *)manager->getMemoryLocation(tree->ptr);
          // while (num >= 32) {
          //   elements += Helper::countInt(intArr[0]);
          //   intArr++;
          //   num -= 32;
          //
          // }
          // if (num >= 31) {
          //   return Helper::countInt(intArr[0]);
          // } else {
          //   int help = (1 <<(num+1)) -1;
          //   return Helper::countInt(intArr[0] & help);
          // }
        }
    }

    }

    return make_pair<int, BlockAVLNode*>(int(elements), NULL);
  }

  pair<int, BlockAVLNode*> select(int size) { 

    BlockAVLNode *tree = root;
    size++;

    while(tree) {
      int val = mysize(tree->left);
      int count = mysize(tree) - mysize(tree->left) - mysize(tree->right);

      //
      // for (int n=0; n< 32; n++) {
      // }

      if (size <= val) {
        tree = tree->left;
      } else if (size <= val + count) {

        
        return make_pair(size-val, tree);
        // TODO test this and put it in
        //
        // int num = size - val;
        // int* intArr = (int *)manager->getMemoryLocation(tree->ptr);
        // int res = tree->elem;
        // while(true) {
        //   int help = Helper::countInt(intArr[0]);
        //   if (help >= num) {
        //     return res + Helper::getXOnePos(intArr[0], num);
        //   } else {
        //     num-=help;
        //     res += 32;
        //     intArr++;
        //   }
        // }
      } else {
        size -= val + count;
        tree = tree->right;
      }
    }

    return make_pair<int, BlockAVLNode*>(-1, NULL);
  }

  inline int max(int a, int b) {return (a > b) ? a : b;}

  inline int height(BlockAVLNode *t) {return t ? t->height : -1;}
  inline int mysize(BlockAVLNode *t) {return t ? t->mysize : 0;}

  BlockAVLNode *leftRotate(BlockAVLNode *tree) {  // returns new tree
    BlockAVLNode *k2 = tree;
    BlockAVLNode *k1 = tree->left;

    k2->left = k1->right;
    k1->right = k2;
    k2->height = 1 + max(height(k2->left), height(k2->right));
    k1->height = 1 + max(height(k1->left), k2->height);

    k2->mysize = k2->getOnes() + mysize(k2->left) +  mysize(k2->right);
    k1->mysize = k1->getOnes() + mysize(k1->left) +  k2->mysize;

    return k1;
  }

  BlockAVLNode *rightRotate(BlockAVLNode *tree) {  // returns new tree
    BlockAVLNode *k2 = tree;
    BlockAVLNode *k1 = tree->right;
    k2->right = k1->left;
    k1->left = k2;
    k2->height = 1 + max(height(k2->left), height(k2->right));
    k1->height = 1 + max(k2->height, height(k1->right));

    k2->mysize = k2->getOnes() + mysize(k2->left) +  mysize(k2->right);
    k1->mysize = k1->getOnes() + k2->mysize + mysize(k1->right);



    return k1;
  }

  // rebalance ()
  // Given a tree and a path from a node back up to the root,
  // we do the following with each visited node:
  //    A. check for an AVL imbalance and perform
  //       a rotation as necessary;
  //    B. Recompute the stored height of each node.
  //
  BlockAVLNode *rebalance(BlockAVLNode *tree, list<BlockAVLNode*> &path) {
    while (!path.empty()) {
      BlockAVLNode *dad = path.front();
      path.pop_front();
      const int leftHeight = height(dad->left);
      const int rightHeight = height(dad->right);
      const int balance = rightHeight - leftHeight;
      if (balance < -1) { // left tree too deep
        BlockAVLNode **root;   // ptr to root of (sub)tree we are rotating
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
        BlockAVLNode **root;
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

  struct NodeCoord { // use by print routine
    const BlockAVLNode *node;   // may be null
    int i, j;        // node addres (i = level, j = child index for level i)
    NodeCoord(const BlockAVLNode *n, int i_, int j_) : node(n), i(i_), j(j_) {}
  };




};

#endif // ADVANCED_BLOCKAVLTREE_H
