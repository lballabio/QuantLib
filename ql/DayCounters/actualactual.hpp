
/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

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

/*! \file actualactual.hpp
    \brief act/act day counters
*/

#ifndef quantlib_actualactual_day_counter_h
#define quantlib_actualactual_day_counter_h

#include <ql/daycounter.hpp>

namespace QuantLib {

    //! Actual/Actual day count
    /*! The day count can be calculated according to ISMA and US Treasury
        convention, also known as "Actual/Actual (Bond)"; to ISDA, also
        known as "Actual/Actual (Historical)"; or to AFB, also known as
        "Actual/Actual (Euro)".

        For more details, refer to
        http://www.isda.org/c_and_a/pdf/mktc1198.pdf
    */
    class ActualActual : public DayCounter {
      public:
        enum Convention { ISMA, Bond, ISDA, Historical, AFB, Euro };
      private:
        class ISMA_Impl : public DayCounter::Impl {
          public:
            std::string name() const { 
                return std::string("act/act (Bond)");
            }
            int dayCount(const Date& d1, const Date& d2) const {
                return (d2-d1); 
            }
            Time yearFraction(const Date& d1, const Date& d2,
                              const Date&, const Date&) const;
        };
        class ISDA_Impl : public DayCounter::Impl {
          public:
            std::string name() const { 
                return std::string("act/act (ISDA)");
            }
            int dayCount(const Date& d1, const Date& d2) const {
                return (d2-d1); 
            }
            Time yearFraction(const Date& d1, const Date& d2,
                              const Date&, const Date&) const;
        };
        class AFB_Impl : public DayCounter::Impl {
          public:
            std::string name() const { 
                return std::string("act/act (Euro)");
            }
            int dayCount(const Date& d1, const Date& d2) const {
                return (d2-d1); 
            }
            Time yearFraction(const Date& d1, const Date& d2,
                              const Date&, const Date&) const;
        };
        static boost::shared_ptr<DayCounter::Impl> implementation(
                                                                Convention c);
      public:
        ActualActual(Convention c = ActualActual::ISMA)
        : DayCounter(implementation(c)) {}
    };

}


#endif
