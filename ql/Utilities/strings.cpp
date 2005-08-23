/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/Utilities/strings.hpp>
#include <cctype>
#if defined(BOOST_NO_STDC_NAMESPACE)
    namespace std { using ::tolower; using ::toupper; }
#endif

namespace QuantLib {

    std::string lowercase(const std::string& s) {
        std::string output = s;
        for (std::string::iterator i=output.begin(); i!=output.end(); i++)
            *i = std::tolower(*i);
        return output;
    }

    std::string uppercase(const std::string& s) {
        std::string output = s;
        for (std::string::iterator i=output.begin(); i!=output.end(); i++)
            *i = std::toupper(*i);
        return output;
    }

}
