
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

/*! \file dataformatters.hpp
    \brief output manipulators
*/

#ifndef quantlib_data_formatters_hpp
#define quantlib_data_formatters_hpp

#include <ql/null.hpp>
#include <ostream>

namespace QuantLib {

    namespace detail {

        template <typename T> struct null_checker { T value; };
        template <typename T>
        std::ostream& operator<<(std::ostream&, const null_checker<T>&);

        struct ordinal_holder { Size n; };
        std::ostream& operator<<(std::ostream&, const ordinal_holder&);

        template <typename T> struct power_of_two_holder { T n; };
        template <typename T>
        std::ostream& operator<<(std::ostream&,
                                 const power_of_two_holder<T>&);

        struct percent_holder { Real value; };
        std::ostream& operator<<(std::ostream&, const percent_holder&);

    }


    namespace io {

        /*! \defgroup manips Output manipulators

            Helper functions for creating formatted output.

            @{
        */

        //! check for nulls before output
        template <typename T>
        detail::null_checker<T> checknull(T);

        //! outputs naturals as 1st, 2nd, 3rd...
        detail::ordinal_holder ordinal(Size);

        //! output integers as powers of two
        template <typename T>
        detail::power_of_two_holder<T> power_of_two(T);

        //! output reals as percentages
        detail::percent_holder percent(Real);

        //! output rates and spreads as percentages
        detail::percent_holder rate(Rate);

        //! output volatilities as percentages
        detail::percent_holder volatility(Volatility);

        /*! @}  */


        // inline definitions


        template <typename T>
        inline detail::null_checker<T> checknull(T x) {
            detail::null_checker<T> checker = { x };
            return checker;
        }

        inline detail::ordinal_holder ordinal(Size n) {
            detail::ordinal_holder holder = { n };
            return holder;
        }

        template <typename T>
        inline detail::power_of_two_holder<T> power_of_two(T n) {
            detail::power_of_two_holder<T> holder = { n };
            return holder;
        }

        inline detail::percent_holder percent(Real x) {
            detail::percent_holder holder = { x };
            return holder;
        }

        inline detail::percent_holder rate(Rate r) {
            detail::percent_holder holder = { r };
            return holder;
        }

        inline detail::percent_holder volatility(Volatility v) {
            detail::percent_holder holder = { v };
            return holder;
        }

    }

    namespace detail {

        template <typename T>
        inline std::ostream& operator<<(std::ostream& out,
                                        const null_checker<T>& checker) {
            if (checker.value == Null<T>())
                return out << "null";
            else
                return out << checker.value;
        }

        template <typename T>
        inline std::ostream& operator<<(std::ostream& out,
                                        const power_of_two_holder<T>& holder) {
            if (holder.n == Null<T>())
                return out << "null";

            T n = holder.n;
            Integer power = 0;
            if (n != 0) {
                while (!(n & 1UL)) {
                    power++;
                    n >>= 1;
                }
            }
            return out << n << "*2^" << power;
        }

    }

}


#endif
