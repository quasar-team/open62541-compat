/*
 * array_templates.h
 *
 *  Created on: 27 Nov 2017
 *      Author: pnikiel
 */

#ifndef OPEN62541_COMPAT_INCLUDE_ARRAY_TEMPLATES_H_
#define OPEN62541_COMPAT_INCLUDE_ARRAY_TEMPLATES_H_

#include <vector>

template<typename T>
class UaCompatArray
{
public:

    void create(size_t n)
    {
        m_data.clear();
        m_data.insert( m_data.begin(), n, T());
    }

    T& operator[](size_t i) { return m_data[i]; }
    const T& operator[](size_t i) const { return m_data[i]; }

    size_t size() const { return m_data.size(); }

private:
    std::vector<T> m_data;
};



#endif /* OPEN62541_COMPAT_INCLUDE_ARRAY_TEMPLATES_H_ */
