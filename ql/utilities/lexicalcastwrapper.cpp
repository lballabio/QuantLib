/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Mark Joshi

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file lexicalcastwrapper.cpp
    \brief function to wrap lexical cast as it has issues under x64
*/
#ifndef x64
#include <boost/lexical_cast.hpp>
#endif
#include <ql/utilities/lexicalcastwrapper.hpp>


namespace QuantLib 
{

    Integer LexicalCastToInteger(const std::string& str)
    {
#ifndef x64
        return  boost::lexical_cast<Integer>(str.c_str());
#else
        return  atoi(str.c_str());
#endif
    }
}
