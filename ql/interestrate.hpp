
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

#ifndef quantlib_interest_rate_h
#define quantlib_interest_rate_h

#include <ql/date.hpp>
#include <ql/types.hpp>
#include <ql/daycounter.hpp>

namespace QuantLib {

    //! Interest rate coumpounding rule
    enum Compounding { Simple = 0,     //!< 1+r*t
                       Compounded = 1, //!< (1+r)^t
                       Continuous = 2  //!< exp(r*t)
    };

    //! Concrete interest rate class
    /*! This class encapsulate the interest rate compounding algebra.
        It manages daycounting convention, compounding convention,
        conversion between different conventions, and discount and accrual
        calculations.

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
                     DayCounter dc,
                     Compounding comp,
                     Frequency freq = Annual);
        //@}
        //! \name inspectors
        //@{
        Rate rate() { return r_; }
        DayCounter dayCounter() { return dc_; }
        Compounding compounding() { return comp_; }
        Frequency frequency() {
            return comp_==Compounded ? Frequency(Integer(freq_)) : NoFrequency;
        }
        //@}
        //! \name calculations
        //@{
        /*! returns the discount factor implied by the rate
            compounded at time t
        */
        DiscountFactor discountFactor(Time t) const {
            return 1.0/compoundFactor(t);
        }
        /*! returns the compound (a.k.a capitalization) factor implied by
            the rate compounded at time t
        */
        Real compoundFactor(Time t) const;
        //! returns the equivalent rate for the compounding period t
        Rate equivalentRate(Time t,
                            DayCounter dc,
                            Compounding comp,
                            Frequency freq = Annual) const;
        /*! returns the discount factor implied by the rate
            compounded between two dates
        */
        DiscountFactor discountFactor(Date d1, Date d2) const {
            Time t = dc_.yearFraction(d1, d2);
            return discountFactor(t);
        }
        /*! returns the compound factor implied by the rate
            compounded between two dates
        */
        Real compoundFactor(Date d1, Date d2) const {
            Time t = dc_.yearFraction(d1, d2);
            return compoundFactor(t);
        }
        /*! returns the equivalent rate for the
            compounding period between two dates
        */
        Rate equivalentRate(Date d1,
                            Date d2,
                            DayCounter dc,
                            Compounding comp,
                            Frequency freq = Annual) const {
            Time t = dc_.yearFraction(d1, d2);
            return equivalentRate(t, dc, comp, freq);
        }
        //@}
      private:
        Rate r_;
        DayCounter dc_;
        Compounding comp_;
        Real freq_;
    };

    //! Formats coompounding rule for output
    /*! Uses FrequencyFormatter and adds compounding informations */
    class CompoundingRuleFormatter {
      public:
        static std::string toString(Compounding comp,
                                    Frequency freq = NoFrequency);
    };

    //! Formats interest rates for output
    /*! Combines RateFormatter and CompoundingRuleFormatter with
        information about the daycounting convention*/
    class InterestRateFormatter {
      public:
        static std::string toString(InterestRate ir,
                                    Integer precision = 5);
    };

}


#endif
