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

#ifndef OPEN62541_COMPAT_INCLUDE_ARRAYS_H_
#define OPEN62541_COMPAT_INCLUDE_ARRAYS_H_

#include <uavariant.h>

class UaVariantArray
{
public:
	UaVariantArray();
	UaVariantArray (const UaVariantArray& other);

	const UaVariant& operator[](unsigned int index) const;
	UaVariant& operator[](unsigned int index);

	void create(unsigned int sz);
	size_t size() const { return m_size; }

private:
	UaVariant    *m_data;
	unsigned int m_size;

};


#endif /* OPEN62541_COMPAT_INCLUDE_ARRAYS_H_ */
