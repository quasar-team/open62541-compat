/* © Copyright Piotr Nikiel, CERN, 2018.  All rights not expressly granted are reserved.
 * uabytearray.cpp
 *
 *  Created on: 18 Apr 2018
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

#include <simple_arrays.h>

UaByteArray::UaByteArray():
    UaCompatArray<OpcUa_Byte>()
{
}

UaByteArray::UaByteArray(const char* data, unsigned int sz)
{
    create(sz);
    std::copy(data, data+sz, m_data.begin());
}

const char* UaByteArray::data() const
{
    return this->size() > 0 ? reinterpret_cast<const char*> (&m_data[0]) : 0;
}

