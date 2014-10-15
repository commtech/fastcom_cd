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

#ifndef SYSEXEPTION_HPP
#define SYSEXEPTION_HPP

#include <stdexcept>

namespace Fscc {

class SystemException : public std::runtime_error
{
public:
    SystemException(const std::string& error_msg = "");

    unsigned error_code(void) const throw();

private:
    unsigned _error_code;
};

class PortNotFoundException : public SystemException
{
public:
    PortNotFoundException(unsigned port_num);
};

class InvalidAccessException : public SystemException
{
public:
    InvalidAccessException(void);
};

class TimeoutException : public SystemException
{
public:
    TimeoutException(void);
};

class BufferTooSmallException : public SystemException
{
public:
    BufferTooSmallException(void);
};

class IncorrectModeException : public SystemException
{
public:
    IncorrectModeException(void);
};

class InvalidParameterException : public SystemException
{
public:
    InvalidParameterException(void);
};

} /* namespace Fscc */

#endif
