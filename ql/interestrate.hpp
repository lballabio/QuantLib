
/*
 Copyright (C) 2004 Ferdinando Ametrano

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

/*! \file interestrate.hpp
    \brief Instrument rate class
*/

#ifndef quantlib_interest_rate_hpp
#define quantlib_interest_rate_hpp

#include <ql/types.hpp>
#include <ql/daycounter.hpp>

namespace QuantLib {

    //! Interest rate coumpounding rule
    enum Compounding { Simple = 0,          //!< \f$ 1+rt \f$
                       Compounded = 1,      //!< \f$ (1+r)^t \f$
                       Continuous = 2,      //!< \f$ e^{rt} \f$
                       SimpleThenCompounded //!< Simple then Compounded
    };

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
        InterestRate(Rate r,
                     const DayCounter& dc,
                     Compounding comp,
                     Frequency freq = Annual);
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
        DiscountFactor discountFactor(const Date& d1, const Date& d2) const {
            Time t = dc_.yearFraction(d1, d2);
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
        Real compoundFactor(const Date& d1, const Date& d2) const {
            Time t = dc_.yearFraction(d1, d2);
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
                                        Time t,
                                        const DayCounter& resultDC,
                                        Compounding comp,
                                        Frequency freq = Annual);

        //! implied rate for a given compound factor between two dates.
        /*! The resulting rate is calculated taking the required
            day-counting rule into account.
        */
        static InterestRate impliedRate(Real compound,
                                        const Date& d1,
                                        const Date& d2,
                                        const DayCounter& resultDC,
                                        Compounding comp,
                                        Frequency freq = Annual) {
            QL_REQUIRE(d2>d1,
                       "d1 (" + DateFormatter::toString(d1) + ") "
                       "later than or equal to d2 "
                       "(" + DateFormatter::toString(d2) + ")");
            Time t = resultDC.yearFraction(d1, d2);
            return impliedRate(compound, t, resultDC, comp, freq);
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
        InterestRate equivalentRate(Time t,
                                    Compounding comp,
                                    Frequency freq = Annual) const {
            return impliedRate(compoundFactor(t), t, dc_, comp, freq);
        }

        //! equivalent rate for a compounding period between two dates
        /*! The resulting rate is calculated taking the required
            day-counting rule into account.
        */
        InterestRate equivalentRate(Date d1,
                                    Date d2,
                                    const DayCounter& resultDC,
                                    Compounding comp,
                                    Frequency freq = Annual) const {
            QL_REQUIRE(d2>d1,
                       "d1 (" + DateFormatter::toString(d1) + ") "
                       "later than or equal to d2 "
                       "(" + DateFormatter::toString(d2) + ")");
            Time t1 = dc_.yearFraction(d1, d2);
            Time t2 = resultDC.yearFraction(d1, d2);
            return impliedRate(compoundFactor(t1), t2, resultDC, comp, freq);
        }
        //@}
      private:
        Rate r_;
        DayCounter dc_;
        Compounding comp_;
        bool freqMakesSense_;
        Real freq_;
    };

    //! Formats compounding rule for output
    /*! Uses FrequencyFormatter and adds compounding informations */
    class CompoundingRuleFormatter {
      public:
        static std::string toString(Compounding comp,
                                    Frequency freq = NoFrequency);
    };

    //! Formats interest rates for output
    /*! Combines RateFormatter and CompoundingRuleFormatter with
        information about the day-counting convention
    */
    class InterestRateFormatter {
      public:
        static std::string toString(InterestRate ir,
                                    Integer precision = 5);
    };

}


#endif
