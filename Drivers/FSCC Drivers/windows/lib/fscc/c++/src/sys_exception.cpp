/*
    Copyright (C) 2014 Commtech, Inc.

    This file is part of cppfscc.

    cppfscc is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    cppfscc is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with cppfscc.  If not, see <http://www.gnu.org/licenses/>.

*/

#include <fscc.h>
#include <sstream>

#include "sys_exception.hpp"

#define SSTR(x) dynamic_cast< std::ostringstream & >( \
        ( std::ostringstream() << std::dec << x ) ).str()

namespace Fscc {

SystemException::SystemException(const std::string& error_msg) : std::runtime_error(error_msg)
{
}


PortNotFoundException::PortNotFoundException(unsigned port_num)
    : SystemException(SSTR("Port " << + port_num << " not found"))
{
}


InvalidAccessException::InvalidAccessException(void) : SystemException("Invalid access")
{
}


TimeoutException::TimeoutException(void) : SystemException("Command timed out (missing clock)")
{
}


BufferTooSmallException::BufferTooSmallException(void) : SystemException("Buffer too small")
{
}


IncorrectModeException::IncorrectModeException(void) : SystemException("Incorrect mode")
{
}


InvalidParameterException::InvalidParameterException(void) : SystemException("Invalid parameter")
{
}

} /* namespace Fscc */
