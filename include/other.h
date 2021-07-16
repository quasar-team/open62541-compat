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


#ifndef __OTHER_H__
#define __OTHER_H__

#include <open62541.h>
#include <uastring.h>
//TODO: amalgamated or not


#define OpcUa_True true
#define OpcUa_False false





enum OpcUa_AccessLevels
{
  OpcUa_AccessLevels_CurrentRead = UA_ACCESSLEVELMASK_READ,
  OpcUa_AccessLevels_CurrentReadOrWrite = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE
};


class UaLocalizedText
{
public:
    UaLocalizedText( const char* locate, const char* text);
    ~UaLocalizedText ();
    const UA_LocalizedText* impl () { return &m_impl; }
private:
    UaLocalizedText( const UaLocalizedText & other );
    void operator= ( const UaLocalizedText & other );
    UA_LocalizedText m_impl ;
};


class UaQualifiedName
{
  public:
    UaQualifiedName(int ns, const UaString& name);
    UA_QualifiedName impl() const { return m_impl; }
    UaString unqualifiedName() const { return m_unqualifiedName; }
    UaString toFullString() const;
  private:
    UaString m_unqualifiedName;
    UA_QualifiedName m_impl;
};

class UaMutexRefCounted {};

#define UA_DISABLE_COPY(T) 

class ServiceSettings
{

};

class DiagnosticInfo
{

};

#endif // __OTHER_H__
