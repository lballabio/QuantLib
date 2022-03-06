/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004 Ferdinando Ametrano

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

/*! \file interestrate.hpp
    \brief Instrument rate class
*/

#ifndef quantlib_interest_rate_hpp
#define quantlib_interest_rate_hpp

#include <ql/compounding.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>

namespace QuantLib {

    //! Concrete interest rate class
    /*! This class encapsulate the interest rate compounding algebra.
        It manages day-counting conventions, compounding conventions,
        conversion between different conventions, discount/compound factor
        calculations, and implied/equivalent rate calculations.

        \test Converted rates are checked against known good results
    */
    class InterestRate {
      public:
        //! \name constructors
        //@{
        //! Default constructor returning a null interest rate.
        InterestRate();
        //! Standard constructor
        InterestRate(Rate r, DayCounter dc, Compounding comp, Frequency freq);
        //@}
        //! \name conversions
        //@{
        operator Rate() const { return r_; }
        //@}
        //! \name inspectors
        //@{
        Rate rate() const { return r_; }
        const DayCounter& dayCounter() const { return dc_; }
        Compounding compounding() const { return comp_; }
        Frequency frequency() const {
            return freqMakesSense_ ? Frequency(Integer(freq_)) : NoFrequency;
        }
        //@}

        //! \name discount/compound factor calculations
        //@{
        //! discount factor implied by the rate compounded at time t.
        /*! \warning Time must be measured using InterestRate's own
                     day counter.
        */
        DiscountFactor discountFactor(Time t) const {
            return 1.0/compoundFactor(t);
        }

        //! discount factor implied by the rate compounded between two dates
        DiscountFactor discountFactor(const Date& d1,
                                      const Date& d2,
                                      const Date& refStart = Date(),
                                      const Date& refEnd = Date()) const {
            QL_REQUIRE(d2>=d1,
                       "d1 (" << d1 << ") "
                       "later than d2 (" << d2 << ")");
            Time t = dc_.yearFraction(d1, d2, refStart, refEnd);
            return discountFactor(t);
        }

        //! compound factor implied by the rate compounded at time t.
        /*! returns the compound (a.k.a capitalization) factor
            implied by the rate compounded at time t.

            \warning Time must be measured using InterestRate's own
                     day counter.
        */
        Real compoundFactor(Time t) const;

        //! compound factor implied by the rate compounded between two dates
        /*! returns the compound (a.k.a capitalization) factor
            implied by the rate compounded between two dates.
        */
        Real compoundFactor(const Date& d1,
                            const Date& d2,
                            const Date& refStart = Date(),
                            const Date& refEnd = Date()) const {
            QL_REQUIRE(d2>=d1,
                       "d1 (" << d1 << ") "
                       "later than d2 (" << d2 << ")");
            Time t = dc_.yearFraction(d1, d2, refStart, refEnd);
            return compoundFactor(t);
        }
        //@}

        //! \name implied rate calculations
        //@{

        //! implied interest rate for a given compound factor at a given time.
        /*! The resulting InterestRate has the day-counter provided as input.

            \warning Time must be measured using the day-counter provided
                     as input.
        */
        static InterestRate impliedRate(Real compound,
                                        const DayCounter& resultDC,
                                        Compounding comp,
                                        Frequency freq,
                                        Time t);

        //! implied rate for a given compound factor between two dates.
        /*! The resulting rate is calculated taking the required
            day-counting rule into account.
        */
        static InterestRate impliedRate(Real compound,
                                        const DayCounter& resultDC,
                                        Compounding comp,
                                        Frequency freq,
                                        const Date& d1,
                                        const Date& d2,
                                        const Date& refStart = Date(),
                                        const Date& refEnd = Date()) {
            QL_REQUIRE(d2>=d1,
                       "d1 (" << d1 << ") "
                       "later than d2 (" << d2 << ")");
            Time t = resultDC.yearFraction(d1, d2, refStart, refEnd);
            return impliedRate(compound, resultDC, comp, freq, t);
        }
        //@}

        //! \name equivalent rate calculations
        //@{

        //! equivalent interest rate for a compounding period t.
        /*! The resulting InterestRate shares the same implicit
            day-counting rule of the original InterestRate instance.

            \warning Time must be measured using the InterestRate's
                     own day counter.
        */
        InterestRate equivalentRate(Compounding comp,
                                    Frequency freq,
                                    Time t) const {
            return impliedRate(compoundFactor(t), dc_, comp, freq, t);
        }

        //! equivalent rate for a compounding period between two dates
        /*! The resulting rate is calculated taking the required
            day-counting rule into account.
        */
        InterestRate equivalentRate(const DayCounter& resultDC,
                                    Compounding comp,
                                    Frequency freq,
                                    Date d1,
                                    Date d2,
                                    const Date& refStart = Date(),
                                    const Date& refEnd = Date()) const {
            QL_REQUIRE(d2>=d1,
                       "d1 (" << d1 << ") "
                       "later than d2 (" << d2 << ")");
            Time t1 = dc_.yearFraction(d1, d2, refStart, refEnd);
            Time t2 = resultDC.yearFraction(d1, d2, refStart, refEnd);
            return impliedRate(compoundFactor(t1), resultDC, comp, freq, t2);
        }
        //@}
      private:
        Rate r_;
        DayCounter dc_;
        Compounding comp_;
        bool freqMakesSense_;
        Real freq_;
    };

    /*! \relates InterestRate */
    std::ostream& operator<<(std::ostream&,
                             const InterestRate&);

}

#endif


#ifndef id_f91555a9bb53de1aad7aefecc4b935ff
#define id_f91555a9bb53de1aad7aefecc4b935ff
inline bool test_f91555a9bb53de1aad7aefecc4b935ff(const int* i) {
    return i != nullptr;
}
#endif
