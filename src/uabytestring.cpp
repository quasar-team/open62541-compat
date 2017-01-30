/*
 * uabytestring.cpp
 *
 *  Created on: Jan 26, 2017
 *      Author: pnikiel
 */

#include <uabytestring.h>

#include <open62541_compat_common.h>

UaByteString::UaByteString ()
{
	m_impl = UA_ByteString_new();
	if (!m_impl)
		throw alloc_error();
	UA_ByteString_init(m_impl);
}

UaByteString::UaByteString( const int length, OpcUa_Byte* data)
{
	m_impl = UA_ByteString_new();
	if (!m_impl)
		throw alloc_error();
	UA_ByteString_init(m_impl);

	try
	{
		setByteString( length, data );
	}
	catch ( alloc_error &e)
	{
		UA_ByteString_delete(m_impl);
		throw e;
	}

}

UaByteString::~UaByteString ()
{
	if (m_impl->data)
	{
		UA_ByteString_deleteMembers( m_impl );
		m_impl->data = 0;
	}
	if (m_impl)
		UA_ByteString_delete( m_impl );
	m_impl = 0;
}

void UaByteString::setByteString (const int len, OpcUa_Byte *data)
{
	if (m_impl->data)
	{
		UA_ByteString_deleteMembers( m_impl );
		m_impl->data = 0;
	}
	m_impl->data = (UA_Byte*)malloc( len );
	if (!m_impl->data)
	{
		throw alloc_error();
	}
	memcpy( m_impl->data, data, len );
	m_impl->length = len;

}
