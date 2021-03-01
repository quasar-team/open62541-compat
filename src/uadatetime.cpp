/* © Copyright CERN, 2017.  All rights not expressly granted are reserved.
 *  opcua_types.h
 *
 *    Moved on: 30 Nov 2017  
 *      Author: Ben Farnham 
 *              Piotr Nikiel <piotr@nikiel.info>
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


#include <boost/format.hpp>
#include <boost/date_time.hpp>

#include <uadatetime.h>
#include <open62541_compat.h>

UaDateTime::UaDateTime()
:m_dateTime{0}
{}

UaDateTime::UaDateTime(const UA_DateTime& dateTime)
:m_dateTime(dateTime)
{}

UaDateTime UaDateTime::now()
{
    return UaDateTime(UA_DateTime_now());
}

void UaDateTime::addSecs(int secs)
{
        m_dateTime += (secs * UA_DATETIME_SEC);
}

void UaDateTime::addMilliSecs(int msecs)
{
        m_dateTime += (msecs * UA_DATETIME_MSEC);
}

/**
 *  * Accepts format
 *   * "%Y-%m-%dT%H:%M:%S%ZP"
 *    * e.g. unix epoch: "1970-01-01T00:00:00Z"
 *     * e.g. open62541 epoch "1601-01-01T00:00:00Z" (i.e. windows epoch)
 *      */
UaDateTime UaDateTime::fromString(const UaString& dateTimeString)
{
        const std::string stdDateTimeString(dateTimeString.toUtf8());
        std::istringstream ss(stdDateTimeString);

        const static std::string timeFormatString("%Y-%m-%dT%H:%M:%S%ZP");
        static std::locale timeFormatLocale(ss.getloc(), new boost::posix_time::time_input_facet(timeFormatString)); // Not a leak: std::locale deletes fa    cet
        ss.imbue(timeFormatLocale);

        try
        {
                static const boost::posix_time::ptime unixEpoch(boost::gregorian::date(1970, 1, 1));

                if(unixEpoch.is_not_a_date_time())
                {
                        OPEN62541_COMPAT_LOG_AND_THROW(std::runtime_error, "Failed to calculate unix epoch, cannot parse any dates from strings.");
                }

                boost::posix_time::ptime dateTime;
                ss >> dateTime;

                if(dateTime.is_not_a_date_time())
                {
                        std::ostringstream err;
                        err << "Failed to convert string ["<<stdDateTimeString<<"] to a date, valid format ["<<timeFormatString<<"]";
                        OPEN62541_COMPAT_LOG_AND_THROW(std::runtime_error, err.str());
                }

                const UA_DateTime open62541DateTime = UA_DATETIME_UNIX_EPOCH + ((dateTime - unixEpoch).total_seconds() * UA_DATETIME_SEC);
                return UaDateTime(open62541DateTime);
        }
        catch(const std::runtime_error& e)
        {
                std::ostringstream err;
                err << "Failed to convert string ["<<stdDateTimeString<<"] to a date, valid format ["<<timeFormatString<<"], error: "<<e.what();
                OPEN62541_COMPAT_LOG_AND_THROW(std::runtime_error, err.str());
        }
        catch(...)
        {
                std::ostringstream err;
                err << "Failed to convert string ["<<stdDateTimeString<<"] to a date, valid format ["<<timeFormatString<<"], unknown error";
                OPEN62541_COMPAT_LOG_AND_THROW(std::runtime_error, err.str());
        }
}

/**
 *  * Returns format
 *   * "%Y-%m-%dT%H:%M:%S%ZP"
 *    * e.g. unix epoch: "1970-01-01T00:00:00Z"
 *     * e.g. open62541 epoch "1601-01-01T00:00:00Z" (i.e. windows epoch)
 *      */
UaString UaDateTime::toString() const
{
        const UA_DateTimeStruct dateTime = UA_DateTime_toStruct(m_dateTime);
        std::ostringstream result;

        const double totalNanoSeconds = (dateTime.milliSec * std::pow(10,6)) + (dateTime.microSec * std::pow(10,3)) + (dateTime.nanoSec);
        const double fractionalSeconds = dateTime.sec + (totalNanoSeconds * std::pow(10,-9));
        result << (boost::format("%04d-%02d-%02d:%02d:%02d:%02.09f") % dateTime.year % dateTime.month % dateTime.day %dateTime.hour % dateTime.min % fractionalSeconds);

        return UaString(result.str().c_str());
}

OpcUa_Int32 UaDateTime::secsTo(const UaDateTime& dt) const
{
	return (dt.m_dateTime - m_dateTime) / 1E7;
}
