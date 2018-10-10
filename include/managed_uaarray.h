/*
 * managed_uaarray.h
 *
 *  Created on: 12 Jul 2018
 *      Author: pnikiel
 *
 * The purpose of this class is to provide UA_Array with:
 * - RAII (e.g. auto-destruction on going out-of-scope)
 * - without necessity to do void* casting by the programmer
 */

#ifndef INCLUDE_MANAGED_UAARRAY_H_
#define INCLUDE_MANAGED_UAARRAY_H_

#include <open62541.h>
#include <open62541_compat_common.h>

/** T is the type contained in the array */

template<typename T>
class ManagedUaArray
{
public:

    //! This ctr is for constructing an array
    ManagedUaArray(size_t sz, const UA_DataType* type):
        m_size(sz),
        m_dataType(type)
    {
        m_array = static_cast<T*> (UA_Array_new(sz, type));
        if (!m_array)
        {
            throw alloc_error();
        }
    }

    //! This ctr is for already allocated array (e.g. returned from open62541 service call)
    ManagedUaArray(size_t sz, const UA_DataType* type, T* array):
        m_size(sz),
        m_dataType(type),
        m_array(array)
    {}

    virtual ~ManagedUaArray()
    {
        UA_Array_delete(m_array, m_size, m_dataType);
    }

    operator T* ()
    {
        return m_array;
    }

private:
    const size_t        m_size;
    const UA_DataType*  m_dataType;
    T*                  m_array;



};



#endif /* INCLUDE_MANAGED_UAARRAY_H_ */
