/**
 * This is free and unencumbered software released into the public domain.
**/

#include "MerkleTree.h"

#include <cassert>

bool MerkleTree::Node::append( NodePtr node, int depth )
{
   assert( depth >= 1 );

   bool appended = false;

   if( depth == 1 )
   {
      if( leftChild == nullptr )
      {
         leftChild = node;
         appended = true;
      }
      else if( rightChild == nullptr )
      {
         rightChild = node;
         appended = true;
      }
   }
   else
   {
      if( leftChild != nullptr )
      {
         appended = leftChild->append( node, depth - 1 );
      }
      else
      {
         leftChild = std::make_shared<Node>();
         appended = leftChild->append( node, depth - 1 );
         assert( appended );
      }

      if( !appended )
      {
         if( rightChild != nullptr )
         {
            appended = rightChild->append( node, depth - 1 );
         }
         else
         {
            rightChild = std::make_shared<Node>();
            appended = rightChild->append( node, depth - 1 );
            assert( appended );
         }
      }
   }

   if( appended )
   {
      hash.clear();
   }

   return appended;
}

bool MerkleTree::Node::isLeaf() const
{
   return leftChild == nullptr && rightChild == nullptr;
}

void MerkleTree::Node::update()
{
   // Hashes get invalidated in append(), so if the hash exists (or we're on a
   // leaf), there's no need to update
   if( !hash.empty() || isLeaf() )
   {
      return;
   }

   // Recurse into the children if necessary
   if( leftChild->hash.empty() )
   {
      leftChild->update();
   }
   
   auto otherChild = leftChild;
   if( rightChild != nullptr )
   {
      otherChild = rightChild;
      if( rightChild->hash.empty() )
      {
         rightChild->update();
      }
   }

   // Concatenate the child data and hash
   auto data = leftChild->hash;
   data.insert( data.end(), otherChild->hash.begin(), otherChild->hash.end() );

   hash = Sha256::doubleHash( data );
}

MerkleTree::MerkleTree()
 : _depth(1),
   _rootNode(std::make_shared<Node>())
{
}

void MerkleTree::append( const ByteArray& hash )
{
   int oldSize = _leafNodes.size();
   if( isPowerOfTwo(oldSize) && oldSize >= 2 )
   {
      _reshape();
   }

   auto newNode = std::make_shared<Node>();
   newNode->hash = hash;

   _leafNodes.push_back( newNode );

   bool appended = _rootNode->append( newNode, _depth );
   assert( appended );
}

ByteArray MerkleTree::rootHash()
{
   _rootNode->update();
   return _rootNode->hash;
}

void MerkleTree::_reshape()
{
   auto oldRoot = _rootNode;
   _rootNode = std::make_shared<Node>();
   _rootNode->leftChild = oldRoot;
   ++_depth;
}
