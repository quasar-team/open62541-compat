/* © Copyright Ben Farnham, CERN, 2018.  All rights not expressly granted are reserved.
 * nodemanagerbase.cpp
 *
 *  Created on: reated on: Apr 11, 2018
 *      Author: Ben Farnham <ben.farnham@cern.ch>
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

#ifndef TEST_INCLUDE_UAVARIANT_TEST_H_
#define TEST_INCLUDE_UAVARIANT_TEST_H_

#include "gtest/gtest.h"
#include "uavariant.h"

class UaVariantTest : public ::testing::Test
{
public:

  UaVariant m_testee;
};


#endif /* TEST_INCLUDE_UAVARIANT_TEST_H_ */
