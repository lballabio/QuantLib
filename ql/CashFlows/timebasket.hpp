
/*
 Copyright (C) 2003 Andre Louw
 Copyright (C) 2003 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file timebasket.hpp
    Distribution over a number of dates
*/

#ifndef quantlib_time_basket_hpp
#define quantlib_time_basket_hpp

#include <ql/date.hpp>
#include <ql/null.hpp>
#include <vector>
#include <map>

namespace QuantLib {

    namespace CashFlows {

        //! Distribution over a number of dates
        class TimeBasket : private std::map<Date,double> {
            // this is needed for Visual C++ 6
            typedef std::map<Date,double> super;
          public:
            TimeBasket() {}
            TimeBasket(const std::vector<Date>& dates,
                       const std::vector<double>& values);
            //! \name Map interface
            //@{
            //! returns the number of entries
            using super::size;
            //! element access
            using super::operator[];
            using super::begin;
            using super::end;
            using super::rbegin;
            using super::rend;
            //@}
            //! \name Algebra
            //@{
            TimeBasket& operator+=(const TimeBasket& other);
            TimeBasket& operator-=(const TimeBasket& other);
            //@}
            //! \name Other methods
            //@{
            //! redistribute the entries over the given dates
            TimeBasket rebin(const std::vector<Date>& buckets) const;
            //@}
        };


        // inline definitions

        inline TimeBasket& TimeBasket::operator+=(const TimeBasket& other) {
            for (const_iterator j = other.begin(); j != other.end(); j++)
                (*this)[j->first] += j->second;
            return *this;
        }

        inline TimeBasket& TimeBasket::operator-=(const TimeBasket& other) {
            for (const_iterator j = other.begin(); j != other.end(); j++)
                (*this)[j->first] -= j->second;
            return *this;
        }

    }

}


#endif
