/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

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

/*! \file actualactual.hpp
    \brief act/act day counters
*/

#ifndef quantlib_actualactual_day_counter_h
#define quantlib_actualactual_day_counter_h

#include <ql/time/daycounter.hpp>
#include <ql/time/schedule.hpp>
#include <utility>

namespace QuantLib {

    //! Actual/Actual day count
    /*! The day count can be calculated according to:

        - the ISDA convention, also known as "Actual/Actual (Historical)",
          "Actual/Actual", "Act/Act", and according to ISDA also "Actual/365",
          "Act/365", and "A/365";
        - the ISMA and US Treasury convention, also known as
          "Actual/Actual (Bond)";
        - the AFB convention, also known as "Actual/Actual (Euro)".

        For more details, refer to
        https://www.isda.org/a/pIJEE/The-Actual-Actual-Day-Count-Fraction-1999.pdf

        \ingroup daycounters

        \test the correctness of the results is checked against known
              good values.
    */
    class ActualActual : public DayCounter {
      public:
        enum Convention { ISMA, Bond,
                          ISDA, Historical, Actual365,
                          AFB, Euro };
      private:
        class ISMA_Impl : public DayCounter::Impl {
          public:
            explicit ISMA_Impl(Schedule schedule) : schedule_(std::move(schedule)) {}

            std::string name() const override { return std::string("Actual/Actual (ISMA)"); }
            Time yearFraction(const Date& d1,
                              const Date& d2,
                              const Date& refPeriodStart,
                              const Date& refPeriodEnd) const override;

          private:
            Schedule schedule_;
        };
        class Old_ISMA_Impl : public DayCounter::Impl {
          public:
            std::string name() const override { return std::string("Actual/Actual (ISMA)"); }
            Time yearFraction(const Date& d1,
                              const Date& d2,
                              const Date& refPeriodStart,
                              const Date& refPeriodEnd) const override;
        };
        class ISDA_Impl : public DayCounter::Impl {
          public:
            std::string name() const override { return std::string("Actual/Actual (ISDA)"); }
            Time
            yearFraction(const Date& d1, const Date& d2, const Date&, const Date&) const override;
        };
        class AFB_Impl : public DayCounter::Impl {
          public:
            std::string name() const override { return std::string("Actual/Actual (AFB)"); }
            Time
            yearFraction(const Date& d1, const Date& d2, const Date&, const Date&) const override;
        };
        static ext::shared_ptr<DayCounter::Impl> implementation(
                                                               Convention c, 
                                                               const Schedule& schedule);
      public:
        /*! \deprecated Use the other constructor.
                        Deprecated in version 1.23.
        */
        QL_DEPRECATED
        ActualActual()
        : DayCounter(implementation(ActualActual::ISDA, Schedule())) {}

        explicit ActualActual(Convention c,
                              const Schedule& schedule = Schedule())
        : DayCounter(implementation(c, schedule)) {}
    };

}

#endif


#ifndef id_5cb326d1f742b3390f4ddd0aeab7cb81
#define id_5cb326d1f742b3390f4ddd0aeab7cb81
inline bool test_5cb326d1f742b3390f4ddd0aeab7cb81(const int* i) {
    return i != nullptr;
}
#endif
