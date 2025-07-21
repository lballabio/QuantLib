/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005, 2009 StatPro Italia srl

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

/*! \file dataformatters.hpp
    \brief output manipulators
*/

#ifndef quantlib_data_formatters_hpp
#define quantlib_data_formatters_hpp

#include <ql/utilities/null.hpp>
#include <ostream>

namespace QuantLib {

    namespace detail {

        template <typename T> struct null_checker {
            explicit null_checker(T value) : value(value) {}
            T value;
        };
        template <typename T>
        std::ostream& operator<<(std::ostream&, const null_checker<T>&);

        struct ordinal_holder {
            explicit ordinal_holder(Size n) : n(n) {}
            Size n;
        };
        std::ostream& operator<<(std::ostream&, const ordinal_holder&);

        template <typename T> struct power_of_two_holder {
            explicit power_of_two_holder(T n) : n(n) {}
            T n;
        };
        template <typename T>
        std::ostream& operator<<(std::ostream&,
                                 const power_of_two_holder<T>&);

        struct percent_holder {
            explicit percent_holder(Real value) : value(value) {}
            Real value;
        };
        std::ostream& operator<<(std::ostream&, const percent_holder&);

        template <typename InputIterator> struct sequence_holder {
            sequence_holder(InputIterator begin, InputIterator end)
            : begin(begin), end(end) {}
            InputIterator begin, end;
        };
        template <typename I>
        std::ostream& operator<<(std::ostream&, const sequence_holder<I>&);

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

        //! output STL-compliant containers as space-separated sequences
        template <class Container>
        detail::sequence_holder<typename Container::const_iterator>
        sequence(const Container& c);

        /*! @}  */


        // inline definitions

        template <typename T>
        inline detail::null_checker<T> checknull(T x) {
            return detail::null_checker<T>(x);
        }

        inline detail::ordinal_holder ordinal(Size n) {
            return detail::ordinal_holder(n);
        }

        template <typename T>
        inline detail::power_of_two_holder<T> power_of_two(T n) {
            return detail::power_of_two_holder<T>(n);
        }

        inline detail::percent_holder percent(Real x) {
            return detail::percent_holder(x);
        }

        inline detail::percent_holder rate(Rate r) {
            return detail::percent_holder(r);
        }

        inline detail::percent_holder volatility(Volatility v) {
            return detail::percent_holder(v);
        }

        template <class Container>
        inline detail::sequence_holder<typename Container::const_iterator>
        sequence(const Container& c) {
            return detail::sequence_holder<typename Container::const_iterator>(
                                                           c.begin(), c.end());
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

        template <typename I>
        inline std::ostream& operator<<(std::ostream& out,
                                        const sequence_holder<I>& holder) {
            out << "( ";
            for (I i = holder.begin; i != holder.end; ++i)
                out << *i << " ";
            out << ")";
            return out;
        }

    }

}


#endif
