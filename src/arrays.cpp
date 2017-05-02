/* © Copyright Piotr Nikiel, CERN, 2015.  All rights not expressly granted are reserved.
 *
 *  Created on: 1 May, 2017
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

#include <arrays.h>
#include <stdexcept>

UaVariantArray::UaVariantArray():
	m_data(0),
	m_size(0)
{

}

UaVariantArray::UaVariantArray (const UaVariantArray& other)
{
	m_size = other.m_size;
	m_data = new UaVariant[ m_size ];
	std::copy( other.m_data, other.m_data+other.m_size, m_data );
}

void UaVariantArray::create(unsigned int sz)
{
	if (m_data != 0)
	{
		delete [] m_data;
		m_data = 0;
		m_size = 0;
	}

	m_data = new UaVariant [sz];
	if (m_data == 0)
		throw std::runtime_error("alloc");
	m_size = sz;
}

const UaVariant& UaVariantArray::operator[](unsigned int index) const
{
	if (index >= m_size)
		throw std::runtime_error("out-of-bounds");
	return m_data[index];
}

UaVariant& UaVariantArray::operator[](unsigned int index)
{
	if (index >= m_size)
		throw std::runtime_error("out-of-bounds");
	return m_data[index];
}
