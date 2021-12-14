#pragma once
#include "ppCommon.h"
#include <mutex>
#include <condition_variable>


//===============================================================================================================================================================================================================
template <class XXX> class ppRing
{
protected:
  XXX*    m_Ring;     //ring data
  int32_t m_RingSize; //ring size
  int32_t m_DataCnt;  //number of units in ring
  int32_t m_WriteIdx; //writting address 
  int32_t m_ReadIdx;  //reading address

  //threading utils
  std::mutex              m_Mutex;
  std::condition_variable m_ConditionVariable;

public:
          ppRing     ();

  void    create     (const int32_t RingSize);
  void    destroy    ();

  void    EnqueueWait(const XXX& NodeData);
  XXX     DequeueWait();
  bool    isEmpty    ();
  bool    isFull     ();
  int32_t getLoad    ();
};  

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

template <class XXX> ppRing<XXX>::ppRing()
{
  m_Ring     = nullptr;
  m_RingSize = 0;
  m_DataCnt  = 0;
  m_WriteIdx = 0;
  m_ReadIdx  = 0;
}
template <class XXX> void ppRing<XXX>::create(const int32_t RingSize)
{
  m_Ring     = (XXX*)calloc(1, RingSize*sizeof(XXX));
  m_RingSize = RingSize;
  m_DataCnt  = 0;
  m_WriteIdx = 0;
  m_ReadIdx  = 0;
}
template <class XXX> void ppRing<XXX>::destroy()
{
  free(m_Ring); 
  m_Ring     = nullptr;
  m_RingSize = 0;
  m_DataCnt  = 0;
  m_WriteIdx = 0;
  m_ReadIdx  = 0;
}
template <class XXX> void ppRing<XXX>::EnqueueWait(const XXX& NodeData)
{
  std::unique_lock<std::mutex> LockManager(m_Mutex);
  while(m_DataCnt>=m_RingSize) { m_ConditionVariable.wait(LockManager, [&]{ return m_DataCnt<m_RingSize;} ); }
  m_Ring[m_WriteIdx] = NodeData;
  m_WriteIdx         = (m_WriteIdx+1)%m_RingSize;
  m_DataCnt++;
  m_ConditionVariable.notify_all();
  //release lock - std::unique_lock destructor... 
}
template <class XXX> XXX ppRing<XXX>::DequeueWait()
{
  XXX NodeData;
  std::unique_lock<std::mutex> LockManager(m_Mutex);
  while(m_DataCnt<=0) { m_ConditionVariable.wait(LockManager, [&]{ return m_DataCnt>0;} ); }
  NodeData = m_Ring[m_ReadIdx];
  m_ReadIdx = (m_ReadIdx+1)%m_RingSize;
  m_DataCnt--;
  m_ConditionVariable.notify_all();
  //release lock - std::unique_lock destructor... 
  return NodeData;
}
template <class XXX> bool ppRing<XXX>::isEmpty()
{
  bool isEmpty;
  m_Mutex.lock();
  isEmpty = m_DataCnt==0;
  m_Mutex.unlock();
  return isEmpty;
}
template <class XXX> bool ppRing<XXX>::isFull()
{
  bool isFull;
  m_Mutex.lock();
  isFull = m_DataCnt==m_RingSize;
  m_Mutex.unlock();
  return isFull;
}
template <class XXX> int32_t ppRing<XXX>::getLoad()
{
  int32_t NumElements;
  m_Mutex.lock();
  NumElements = m_DataCnt;
  m_Mutex.unlock();
  return NumElements;
}

//===============================================================================================================================================================================================================

