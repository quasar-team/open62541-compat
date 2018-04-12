/* © Copyright Piotr Nikiel, CERN, 2015.  All rights not expressly granted are reserved.
 *
 *  Created on: 15 Nov,  2015
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


#ifndef __UASTRING_H_
#define __UASTRING_H_

//TODO: amalgamate?
#include <open62541.h>

#include <string>

#include <uabytestring.h>

class UaString
{
    //! NOTE: All constructors allocate memory for new string's data. There is no memory reuse of any kind.
public:
    //! Constructs null string
    UaString ();
    //! From zero-terminated string; length taken by strlen
    UaString( const char* s);
    //! From another UaString
    UaString( const UaString& other );
    //! From UA_String (open6xxxx)
    UaString( const UA_String* other );

    UaString( const UA_String& other ): UaString(&other) {}

    //! Assignment operator
    const UaString& operator=(const UaString& other);    

    const UaString& operator=(const UA_String& other);

    ~UaString ();
 
    UaString operator+(const UaString& other);
    bool operator==(const UaString& other);

    std::string toUtf8() const;

    const UA_String * impl() const{ return m_impl; }

    UA_String* toOpcUaString() const { return m_impl; }

    void detach(UaString* out);

    size_t length() const { return m_impl->length; /*FIXME: open62541 seems not UTF-8 aware so handle this we caution!*/ }

private:
    UA_String * m_impl;
};

#endif // __UASTRING_H_
