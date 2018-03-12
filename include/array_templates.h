/* © Copyright Piotr Nikiel, CERN, 2017.  All rights not expressly granted are reserved.
 * array_templates.h
 *
 *  Created on: 27 Nov,  2017
 *      Author: Piotr Nikiel <piotr@nikiel.info>
 *
 *  This file is part of Quasar.
 *
 *  Quasar is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public Licence as published by
 *  the Free Software Foundation, either version 3 of the Licence.
 *
 *  Quasar is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public Licence for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with Quasar.  If not, see <http://www.gnu.org/licenses/>.
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

    void resize(size_t n)
    {
        m_data.resize(n, T());
    }

    T& operator[](size_t i) { return m_data.at(i); }
    const T& operator[](size_t i) const { return m_data.at(i); }

    size_t size() const { return m_data.size(); }
    size_t length() const { return size(); } // TODO is this really necessary ...

private:
    std::vector<T> m_data;
};



#endif /* OPEN62541_COMPAT_INCLUDE_ARRAY_TEMPLATES_H_ */
