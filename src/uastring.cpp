/* © Copyright Piotr Nikiel, CERN, 2015.  All rights not expressly granted are reserved.
 * uastring.cpp
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

#include <open62541_compat.h>
#include <iostream>

#include <open62541_compat_common.h>

UaString::UaString ()
{
    m_impl = UA_String_new( );
    if (! m_impl)
	throw alloc_error();

}

UaString::UaString( const char* s)
{
    m_impl = UA_String_new( );
    if ( ! m_impl)
	throw alloc_error();

    *m_impl = UA_String_fromChars( s );
    if( m_impl->length < 0 )
    {
	UA_String_delete( m_impl );
	throw alloc_error();
    }

}

UaString::UaString( const UaString& other)
{
    m_impl = UA_String_new( );
    if ( ! m_impl)
	throw alloc_error();

    if( UA_String_copy( other.m_impl, m_impl ) != UA_STATUSCODE_GOOD)
    {
	UA_String_delete( m_impl );
	throw alloc_error();
    }
   
}

UaString::UaString( const UA_String* other )
{
    m_impl = UA_String_new( );
    if ( ! m_impl)
	throw alloc_error();
    if( UA_String_copy( other, m_impl ) != UA_STATUSCODE_GOOD)
    {
	UA_String_delete( m_impl );
	throw alloc_error();
    }
}    

UaString::~UaString ()
{
    if (m_impl -> data)
	UA_String_deleteMembers( m_impl );
    UA_String_delete( m_impl );
    m_impl = 0;
}

UaString UaString::operator+(const UaString& other)
{
    std::string concatenated = this->toUtf8() + other.toUtf8();
    return UaString( concatenated.c_str() );
}

const UaString& UaString::operator=(const UaString& other)
{
    if (m_impl -> data)
	UA_String_deleteMembers( m_impl );

    if( UA_String_copy( other.m_impl, m_impl ) != UA_STATUSCODE_GOOD)
    {
	UA_String_delete( m_impl );
	throw alloc_error();
    }
    return *this;
}

const UaString& UaString::operator=(const UA_String& other)
{
    if (m_impl -> data)
    UA_String_deleteMembers( m_impl );

    if( UA_String_copy( &other, m_impl ) != UA_STATUSCODE_GOOD)
    {
        UA_String_delete( m_impl );
        throw alloc_error();
    }
    return *this;
}

std::string UaString::toUtf8() const
{
  return std::string( reinterpret_cast<const char*>(m_impl->data), m_impl->length );
}

/** Note(Piotr): this is not real detachment.
 * It was implemented to be fit for arrays stuff but perhaps an another overloaded
 * function should be created.
 */
void UaString::detach(UaString* out)
{
    *out = *this;
}

bool UaString::operator==(const UaString& other)
{
    return UA_String_equal(this->m_impl, other.m_impl);
}
