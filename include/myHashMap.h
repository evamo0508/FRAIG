/****************************************************************************
  FileName     [ myHashMap.h ]
  PackageName  [ util ]
  Synopsis     [ Define HashMap and Cache ADT ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2009-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef MY_HASH_MAP_H
#define MY_HASH_MAP_H

#include <vector>
#include <algorithm>

using namespace std;

// TODO: (Optionally) Implement your own HashMap and Cache classes.

//-----------------------
// Define HashMap classes
//-----------------------
// To use HashMap ADT, you should define your own HashKey class.
// It should at least overload the "()" and "==" operators.
//
class strashKey
{
 public:
   strashKey() {}
   strashKey(unsigned _in0=0,unsigned _in1=0) {in0=_in0;in1=_in1;}
 
   size_t operator() () const 
   { 
      if(in0>in1) return ((in1<<32)+in0);
      else return ((in0<<32)+in1); 
   }
 
   bool operator == (const strashKey& k) const 
   { 
      if(in0>in1) return ((in1<<32)+in0 == k());
      else return ((in0<<32)+in1 == k()); 
   }
 
 private:
   size_t in0,in1;
};

class CirGateV
{
 public:
   CirGateV(){}
   CirGateV(unsigned _id){id=_id;}
   ~CirGateV(){}
   unsigned getID() {return id;}
 private:
   unsigned id;//or cirgate* ??
};

class SimValueKey
{
 public:
   SimValueKey():flag(0){}
   SimValueKey(unsigned _SimValue):flag(0){SimValue=_SimValue;}
   SimValueKey(unsigned _SimValue,unsigned _flag){SimValue=_SimValue;flag=_flag;}
   ~SimValueKey(){}
   unsigned operator() () const { return SimValue; }
   bool operator == (const SimValueKey& k) const { return (k()==SimValue || k()==(~SimValue)); }
   unsigned getSimValue() const{return SimValue;}
   void setFlag (unsigned i) {flag=i;}
   unsigned getFlag() const {return flag;}
 private:
   unsigned SimValue;
   unsigned flag;
};

class FECGroup
{
 public:
   FECGroup(){}
   FECGroup(vector<unsigned>& _id){id=_id;}
   ~FECGroup(){}
   unsigned& operator [] (size_t i) { return id[i]; }
   const unsigned& operator [](size_t i) const { return id[i]; }
   void pushID(unsigned _id) {id.push_back(_id);}
   size_t size() const {return id.size();}
   unsigned getID() {return 0;}
   //bool operator == ( FECGroup& k) const { return k.getID()==id; }
 private:
   vector<unsigned> id;   
};

template <class HashKey, class HashData>
class HashMap
{
typedef pair<HashKey, HashData> HashNode;

public:
   HashMap(size_t b=0) : _numBuckets(0), _buckets(0) { if (b != 0) init(b); }
   ~HashMap() { reset(); }

   // [Optional] TODO: implement the HashMap<HashKey, HashData>::iterator
   // o An iterator should be able to go through all the valid HashNodes
   //   in the HashMap
   // o Functions to be implemented:
   //   - constructor(s), destructor
   //   - operator '*': return the HashNode
   //   - ++/--iterator, iterator++/--
   //   - operators '=', '==', !="
   //
   class iterator
   {
      friend class HashMap<HashKey, HashData>;

    public:
      iterator(HashNode* n,vector<HashNode>* b, size_t i): _node(n),buckets(b),idx(i) {}
      iterator(const iterator& i): _node(i._node),buckets(i.buckets),idx(i.idx) {}
      iterator(){}
      ~iterator() {} 
      
      const HashNode& operator * () const { return *_node; }
      HashNode& operator * () { return *_node; }
      iterator& operator ++ () 
      { 
         if(_node==&buckets[idx].back())
         { 
            do{idx++;}while(buckets[idx].empty());
            _node=&(buckets[idx].front());
         }
         else _node++;
         return *this; 
      }
         
      iterator operator ++ (int) 
      { 
         HashNode* tmp=_node;
         size_t i=idx;
         ++(*this);
         return iterator(tmp,buckets,i);
      }
      iterator& operator -- () 
      { 
         if(_node==&buckets[idx].front())
         { 
            do{idx--;}while(buckets[idx].empty());
            _node=&(buckets[idx].back());
         }
         else _node--;
         return *this; 
      }
      iterator operator -- (int) 
      { 
         HashNode* tmp=_node;
         size_t i=idx;
         --(*this);
         return iterator(tmp,buckets,i);
      }
      iterator& operator = (const iterator& i) 
      { 
         _node=i._node; 
         buckets=i.buckets; 
         idx=i.idx;
         return *(this); 
      }

      bool operator != (const iterator& i) const { return(_node!=i._node); }
      bool operator == (const iterator& i) const { return(_node==i._node); }

      //size_t getIdx const (){return idx;}

    private:
      HashNode* _node;
      vector<HashNode>* buckets;
      size_t idx;
   };

   void init(size_t b) {
      reset(); _numBuckets = b; _buckets = new vector<HashNode>[b+1]; _buckets[b].push_back(HashNode(HashKey(0,1),HashData())); }
   void reset() {
      _numBuckets = 0;
      if (_buckets) { delete [] _buckets; _buckets = 0; }
   }
   void clear() {
      for (size_t i = 0; i < _numBuckets; ++i) _buckets[i].clear();
   }
   size_t numBuckets() const { return _numBuckets; }

   vector<HashNode>& operator [] (size_t i) { return _buckets[i]; }
   const vector<HashNode>& operator [](size_t i) const { return _buckets[i]; }

   // TODO: implement these functions
   //
   // Point to the first valid data
   iterator begin() const { 
     size_t n=0;
     while(_buckets[n].empty())++n;
     return iterator(&_buckets[n][0],_buckets,n);
   }
   // Pass the end
   iterator end() const { return iterator(&_buckets[_numBuckets][0],_buckets,_numBuckets); }
   // return true if no valid data
   bool empty() const { 
     iterator li=begin();
     return li==end();
   }
   // number of valid data
   size_t size() const { 
     size_t s = 0; 
     iterator li=begin();
     while(li!=end()){++li;++s;}
     return s;
   }

   // check if k is in the hash...
   // if yes, return true;
   // else return false;
   bool check(const HashKey& k, unsigned& mergeGate) const { 
      size_t t=bucketNum(k);
      for(size_t i=0;i<_buckets[t].size();++i)
      {
         if(k==_buckets[t][i].first) 
         {
            mergeGate=_buckets[t][i].second.getID();
            return true;
         }
      }
      return false;
   }

   bool check(const HashKey& k, unsigned& row,unsigned& col) const { 
      size_t t=bucketNum(k);
      for(size_t i=0;i<_buckets[t].size();++i)
      {
         if(k==_buckets[t][i].first) 
         {
            //mergeGate=_buckets[t][i].second.getID();
            row=t;col=i;
            return true;
         }
      }
      return false;
   }   

   // query if k is in the hash...
   // if yes, replace d with the data in the hash and return true;
   // else return false;
   bool query(const HashKey& k, HashData& d) const { 
      size_t t=bucketNum(k);
      for(size_t i=0;i<_buckets[t].size();++i)
      {
         if(k==_buckets[t][i].first) 
         {
            d=_buckets[t][i].second; 
            return true;
         }
      }
      return false;
   }

   // update the entry in hash that is equal to k (i.e. == return true)
   // if found, update that entry with d and return true;
   // else insert d into hash as a new entry and return false;
   bool update(const HashKey& k, HashData& d) {  
      size_t t=bucketNum(k);
      for(size_t i=0;i<_buckets[t].size();++i)
      {
         if(k==_buckets[t][i].first) 
         {
            _buckets[t][i].second=d; 
            return true;
         }
      }
      insert(k,d);
      return false;
   }

   // return true if inserted d successfully (i.e. k is not in the hash)
   // return false is k is already in the hash ==> will not insert
   bool insert(const HashKey& k, const HashData& d) { 
      unsigned TOT;
      if(check(k,TOT)) return false;
      _buckets[bucketNum(k)].push_back(HashNode(k,d));
      return true;
   }

   bool forceInsert(const HashKey& k, const HashData& d) {
      _buckets[bucketNum(k)].push_back(HashNode(k,d));
      return true;      
   }

   // return true if removed successfully (i.e. k is in the hash)
   // return fasle otherwise (i.e. nothing is removed)
   bool remove(const HashKey& k) { 
      size_t t=bucketNum(k);
      for(size_t i=0;i<_buckets[t].size();++i)
      {
         if(k==_buckets[t][i].first)
         {
            _buckets[t].erase(_buckets[t].begin()+i);
            return true;
         }
      }
      return false;
   }

private:
   // Do not add any extra data member
   size_t                   _numBuckets;
   vector<HashNode>*        _buckets;

   size_t bucketNum(const HashKey& k) const {
      return (k() % _numBuckets); }

};


//---------------------
// Define Cache classes
//---------------------
// To use Cache ADT, you should define your own HashKey class.
// It should at least overload the "()" and "==" operators.
//
// class CacheKey
// {
// public:
//    CacheKey() {}
//    
//    size_t operator() () const { return 0; }
//   
//    bool operator == (const CacheKey&) const { return true; }
//       
// private:
// }; 
// 
template <class CacheKey, class CacheData>
class Cache
{
typedef pair<CacheKey, CacheData> CacheNode;

public:
   Cache() : _size(0), _cache(0) {}
   Cache(size_t s) : _size(0), _cache(0) { init(s); }
   ~Cache() { reset(); }

   // NO NEED to implement Cache::iterator class

   // TODO: implement these functions
   //
   // Initialize _cache with size s
   void init(size_t s) { reset(); _size = s; _cache = new CacheNode[s]; }
   void reset() {  _size = 0; if (_cache) { delete [] _cache; _cache = 0; } }

   size_t size() const { return _size; }

   CacheNode& operator [] (size_t i) { return _cache[i]; }
   const CacheNode& operator [](size_t i) const { return _cache[i]; }

   // return false if cache miss
   bool read(const CacheKey& k, CacheData& d) const {
      size_t i = k() % _size;
      if (k == _cache[i].first) {
         d = _cache[i].second;
         return true;
      }
      return false;
   }
   // If k is already in the Cache, overwrite the CacheData
   void write(const CacheKey& k, const CacheData& d) {
      size_t i = k() % _size;
      _cache[i].first = k;
      _cache[i].second = d;
   }

private:
   // Do not add any extra data member
   size_t         _size;
   CacheNode*     _cache;
};


#endif // MY_HASH_H
