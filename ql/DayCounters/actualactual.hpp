
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
    /*! The day count can be calculated according to:
    
        - the ISDA convention, also known as "Actual/Actual (Historical)",
        "Actual/Actual", "Act/Act", "Actual/365", "Act/365", and "A/365";
        - ISMA and US Treasury convention, also known as "Actual/Actual (Bond)";
        - to AFB, also known as "Actual/Actual (Euro)".

        For more details, refer to
        http://www.isda.org/c_and_a/pdf/mktc1198.pdf

        \warning this is the same as "Actual/365", while "Actual/365 (Fixed)"
                 is a different daycount

        \ingroup daycounters

        \test the correctness of the results is checked against known
              good values.
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
            Time yearFraction(const Date& d1, const Date& d2,
                              const Date&, const Date&) const;
        };
        class ISDA_Impl : public DayCounter::Impl {
          public:
            std::string name() const {
                return std::string("act/act (ISDA)");
            }
            Time yearFraction(const Date& d1, const Date& d2,
                              const Date&, const Date&) const;
        };
        class AFB_Impl : public DayCounter::Impl {
          public:
            std::string name() const {
                return std::string("act/act (Euro)");
            }
            Time yearFraction(const Date& d1, const Date& d2,
                              const Date&, const Date&) const;
        };
        static boost::shared_ptr<DayCounter::Impl> implementation(
                                                                Convention c);
      public:
        ActualActual(Convention c = ActualActual::ISDA)
        : DayCounter(implementation(c)) {}
    };

}


#endif
