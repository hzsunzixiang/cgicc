#ifndef SINGLETON_T_H
#define SINGLETON_T_H

#include "sync_lock.h"

template <class TYPE, class LOCK = CDummyLock>
class CSingletonT
{
public:
    static TYPE* Instance();

private:
    CSingletonT();
    CSingletonT(const CSingletonT&);
    ~CSingletonT(); 

private:
	TYPE s_aGuestInstance;
	static CSingletonT<TYPE, LOCK> s_aHostInstance;
};

///////////////////////////////////////
template <class TYPE, class LOCK>
CSingletonT<TYPE, LOCK> CSingletonT<TYPE, LOCK>::s_aHostInstance;

template <class TYPE, class LOCK>
CSingletonT<TYPE, LOCK>::CSingletonT()
{
}

template <class TYPE, class LOCK>
CSingletonT<TYPE, LOCK>::~CSingletonT()
{
}

template <class TYPE, class LOCK>
TYPE* 
CSingletonT<TYPE, LOCK>::Instance()
{
    //Lock Guard
    //....

    return &(s_aHostInstance.s_aGuestInstance);
}

#endif /* SINGLETON_T_H */
 
