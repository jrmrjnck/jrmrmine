/**
 * This is free and unencumbered software released into the public domain.
**/
#ifndef MERKLE_TREE_H
#define MERKLE_TREE_H

#include "Util.h"
#include "Sha256.h"

#include <memory>

class MerkleTree
{
private:
   struct Node;
   typedef std::shared_ptr<Node> NodePtr;
   struct Node
   {
      bool append( NodePtr node, int depth );
      bool isLeaf() const;
      void update();

      ByteArray   hash;
      NodePtr     leftChild;
      NodePtr     rightChild;
   };


public:
   MerkleTree();

   void append( const ByteArray& hash );
   void update( int index, const ByteArray& newHash );
   ByteArray rootHash();

private:
   void _reshape();

private:
   int                  _depth;
   NodePtr              _rootNode;
   std::vector<NodePtr> _leafNodes;
};

#endif // !MERKLE_TREE_H
