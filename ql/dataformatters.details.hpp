
/*
 Copyright (C) 2005 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#ifndef quantlib_data_formatters_details_hpp
#define quantlib_data_formatters_details_hpp

#include <ql/qldefines.hpp>
#include <ql/null.hpp>
#include <ostream>

namespace QuantLib {

    namespace detail {

        template <typename T>
        struct nullchecker {
            nullchecker(T value) : value(value) {}
            T value;
        };

        template <typename T>
        std::ostream& operator<<(std::ostream& out,
                                 const nullchecker<T>& checker) {
            if (checker.value == Null<T>())
                return out << "null";
            else
                return out << checker.value;
        }

    }

}


#endif
