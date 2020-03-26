/*
 * uabytestring.cpp
 *
 *  Created on: Jan 26, 2017
 *      Author: pnikiel
 */

#include <limits>

#include <uabytestring.h>
#include <statuscode.h>

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

UaByteString::UaByteString( const UaByteString& other )
{
    other.copyTo(this);
}

UaByteString::~UaByteString ()
{
	release();
	if (m_impl)
		UA_ByteString_delete( m_impl );
	m_impl = 0;
}

void UaByteString::setByteString (const int len, OpcUa_Byte *data)
{
	release();
	if (len>0)
	{
		m_impl->data = (UA_Byte*)malloc( len );
		if (!m_impl->data)
		{
			throw alloc_error();
		}
		memcpy( m_impl->data, data, len );
	}
	m_impl->length = len;

}

OpcUa_Int32 UaByteString::length() const
{
    if (m_impl->length > std::numeric_limits<OpcUa_Int32>::max() )
        throw std::runtime_error("UaByteString::length() open62541 size too big for UASDK API");
    else
        return m_impl->length;
}

void UaByteString::copyTo(UaByteString* other) const
{
    other->release();
    UaStatus status = UA_ByteString_copy(m_impl, other->m_impl);
    if (!status.isGood())
        throw std::runtime_error("UA_ByteString_copy failed with: " + status.toString().toUtf8());
}

void UaByteString::release()
{
    if (m_impl->data)
    {
        UA_ByteString_deleteMembers( m_impl );
        m_impl->data = nullptr;
        m_impl->length = 0;
    }
}
