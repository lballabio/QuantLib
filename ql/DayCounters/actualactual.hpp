
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.org/
 *
 * QuantLib is free software and you are allowed to use, copy, modify, merge,
 * publish, distribute, and/or sell copies of it under the conditions stated
 * in the QuantLib License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
 *
 * You should have received a copy of the license along with this file;
 * if not, please email quantlib-users@lists.sourceforge.net
 * The license is also available at http://quantlib.org/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.org/group.html
*/

/*! \file actualactual.hpp
    \brief act/act day counters

    \fullpath
    ql/DayCounters/%actualactual.hpp
*/

// $Id$

#ifndef quantlib_actualactual_day_counter_h
#define quantlib_actualactual_day_counter_h

#include <ql/daycounter.hpp>

namespace QuantLib {

    namespace DayCounters {

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
            class ActActFactory : public DayCounter::factory {
              public:
                ActActFactory(Convention c) : convention_(c) {}
                DayCounter create() const {
                    return ActualActual(convention_);
                }
              private:
                Convention convention_;
            };
            class ActActISMAImpl : public DayCounter::DayCounterImpl {
              public:
                std::string name() const { return std::string("act/act(b)");}
                int dayCount(const Date& d1, const Date& d2) const {
                    return (d2-d1); }
                Time yearFraction(const Date& d1, const Date& d2,
                    const Date&, const Date&) const;
            };
            class ActActISDAImpl : public DayCounter::DayCounterImpl {
              public:
                std::string name() const { return std::string("act/act(h)");}
                int dayCount(const Date& d1, const Date& d2) const {
                    return (d2-d1); }
                Time yearFraction(const Date& d1, const Date& d2,
                    const Date&, const Date&) const;
            };
            class ActActAFBImpl : public DayCounter::DayCounterImpl {
              public:
                std::string name() const { return std::string("act/act(e)");}
                int dayCount(const Date& d1, const Date& d2) const {
                    return (d2-d1); }
                Time yearFraction(const Date& d1, const Date& d2,
                    const Date&, const Date&) const;
            };
            static Handle<DayCounterImpl> implementation(Convention c);
          public:
            ActualActual(Convention c = ActualActual::ISMA)
            : DayCounter(implementation(c)) {}
            //! returns a factory of actual/actual day counters
            Handle<factory> getFactory(Convention c) const {
                return Handle<factory>(new ActActFactory(c));
            }
        };

    }

}


#endif
