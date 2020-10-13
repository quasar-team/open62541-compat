/* © Copyright Piotr Nikiel, CERN, 2020.  All rights not expressly granted are reserved.
 *
 *  Created on: 08 Oct,  2020
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

#ifndef __UAEXPANDEDNODEID_H__
#define __UAEXPANDEDNODEID_H__

#include <uanodeid.h>

// Note from Piotr
// There are many ways how ExpandedNodeId could be conceived --
// In the style of open62541-compat it simply should be a wrapper on top of UA_ExpandedNodeId,
// But it's difficult to judge how that would affect API etc. So now we have sth simple that conforms for UA-SDK API,
// And then "we will see" if to remap the implementation.

class UaExpandedNodeId
{
public:
	UaExpandedNodeId () {};

	UaExpandedNodeId (
			const UaNodeId &id,
			const UaString &sNsUri,
			OpcUa_UInt32 serverIndex) :
				m_nodeId (id)
	{}

	UaNodeId nodeId() const { return m_nodeId; }

private:
	UaNodeId m_nodeId; // might drift towards either o6 "m_imp" style, or might be supplemented by ns URL ...

};

#endif // __UAEXPANDEDNODEID_H__
