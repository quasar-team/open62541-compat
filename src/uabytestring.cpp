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

UaByteString::UaByteString( const UA_ByteString& other):
        UaByteString()
{
    UaStatus status = UA_ByteString_copy(&other, m_impl);
    if (!status.isGood())
        throw std::runtime_error("UA_ByteString_copy failed: " + status.toString().toUtf8());
}

UaByteString::UaByteString( const int length, const OpcUa_Byte* data):
        UaByteString()
{
	try
	{
		setByteString( length, data );
	}
	catch ( alloc_error &e)
	{
	    release();
		throw;
	}
}

UaByteString::UaByteString( const UaByteString& other ):
        UaByteString()
{
    other.copyTo(this);
}

UaByteString::~UaByteString ()
{
	release();
	if (m_impl)
		UA_ByteString_delete( m_impl );
	m_impl = nullptr;
}

UaByteString& UaByteString::operator= (const UaByteString& other)
{
    release();
    other.copyTo(this);
    return *this;
}

void UaByteString::setByteString (const int len, const OpcUa_Byte *data)
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
