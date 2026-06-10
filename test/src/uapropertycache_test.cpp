/* © Copyright Paris Moschovakos, CERN, 2026.  All rights not expressly granted are reserved.
 * uapropertycache_test.cpp
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

#include "gtest/gtest.h"
#include <uadatavariablecache.h>

TEST(UaPropertyCacheTest, browseNameNamespaceFollowsNodeId)
{
	UaPropertyCache property(
			UaString("address"),
			UaNodeId(UaString("elmb1.address"), 2),
			UaVariant(UaString("42")),
			0x1,
			UaString("en_US"));
	EXPECT_EQ(std::string("ns=2|address"), property.browseName().toFullString().toUtf8());

	UaPropertyCache nsZeroProperty(
			UaString("standardish"),
			UaNodeId(static_cast<OpcUa_UInt32>(12345), 0),
			UaVariant(static_cast<OpcUa_Int32>(1)),
			0x1,
			UaString("en_US"));
	EXPECT_EQ(std::string("ns=0|standardish"), nsZeroProperty.browseName().toFullString().toUtf8());
}

TEST(UaPropertyCacheTest, accessLevelIsStored)
{
	UaPropertyCache property(
			UaString("address"),
			UaNodeId(UaString("elmb1.address"), 2),
			UaVariant(UaString("42")),
			UA_ACCESSLEVELMASK_READ,
			UaString("en_US"));
	EXPECT_EQ(UA_ACCESSLEVELMASK_READ, property.accessLevel());
}

TEST(UaNodeIdFormatTest, toFullStringMatchesUasdk)
{
	EXPECT_EQ(std::string("NS2|String|elmb1.address"),
			UaNodeId(UaString("elmb1.address"), 2).toFullString().toUtf8());
	EXPECT_EQ(std::string("NS0|Numeric|85"),
			UaNodeId(static_cast<OpcUa_UInt32>(85), 0).toFullString().toUtf8());
}

TEST(UaNodeIdFormatTest, toStringIsIdentifierOnly)
{
	EXPECT_EQ(std::string("elmb1.address"),
			UaNodeId(UaString("elmb1.address"), 2).toString().toUtf8());
	EXPECT_EQ(std::string("85"),
			UaNodeId(static_cast<OpcUa_UInt32>(85), 0).toString().toUtf8());
}
