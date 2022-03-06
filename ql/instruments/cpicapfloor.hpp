/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2010, 2011 Chris Kenyon

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

/*! \file cpicapfloor.hpp
 \brief zero-inflation-indexed-ratio-with-base option
 */


#ifndef quantlib_cpicapfloor_hpp
#define quantlib_cpicapfloor_hpp

#include <ql/instrument.hpp>
#include <ql/option.hpp>
#include <ql/time/calendar.hpp>
#include <ql/time/daycounter.hpp>
#include <ql/indexes/inflationindex.hpp>
#include <ql/cashflows/cpicoupon.hpp>

namespace QuantLib {

    //! CPI cap or floor
    /*! Quoted as a fixed strike rate \f$ K \f$.  Payoff:
     \f[
     P_n(0,T) \max(y (N [(1+K)^{T}-1] -
                 N \left[ \frac{I(T)}{I(0)} -1 \right]), 0)
     \f]
     where \f$ T \f$ is the maturity time, \f$ P_n(0,t) \f$ is the
     nominal discount factor at time \f$ t \f$, \f$ N \f$ is the
     notional, and \f$ I(t) \f$ is the inflation index value at
     time \f$ t \f$.

     Inflation is generally available on every day, including
     holidays and weekends.  Hence there is a variable to state
     whether the observe/fix dates for inflation are adjusted or
     not.  The default is not to adjust.

     N.B. a cpi cap or floor is an option, not a cap or floor on a coupon.
     Thus this is very similar to a ZCIIS and has a single flow, this is
     as usual for cpi because it is cumulative up to option maturity from base
     date.

     We do not inherit from Option, although this would be reasonable,
     because we do not have that degree of generality.

     */
    class CPICapFloor : public Instrument {
    public:
        class arguments;
        class results;
        class engine;
        CPICapFloor(Option::Type type,
                    Real nominal,
                    const Date& startDate, // start date of contract (only)
                    Real baseCPI,
                    const Date& maturity, // this is pre-adjustment!
                    Calendar fixCalendar,
                    BusinessDayConvention fixConvention,
                    Calendar payCalendar,
                    BusinessDayConvention payConvention,
                    Rate strike,
                    Handle<ZeroInflationIndex> infIndex,
                    const Period& observationLag,
                    CPI::InterpolationType observationInterpolation = CPI::AsIndex);

        //! \name Inspectors
        //@{
        Option::Type type() const { return type_; }
        Real nominal() const { return nominal_; }
        //! \f$ K \f$ in the above formula.
        Rate strike() const { return strike_; }
        Date fixingDate() const;
        Date payDate() const;
        Handle<ZeroInflationIndex> inflationIndex() const { return infIndex_; }
        Period observationLag() const { return observationLag_; }
        //@}

        //! \name Instrument interface
        //@{
        bool isExpired() const override;
        void setupArguments(PricingEngine::arguments*) const override;
        void fetchResults(const PricingEngine::results* r) const override;
        //@}

    protected:
        Option::Type type_;
        Real nominal_;
        Date startDate_, fixDate_, payDate_;
        Real baseCPI_;
        Date maturity_;
        Calendar fixCalendar_;
        BusinessDayConvention fixConvention_;
        Calendar payCalendar_;
        BusinessDayConvention payConvention_;
        Rate strike_;
        Handle<ZeroInflationIndex> infIndex_;
        Period observationLag_;
        CPI::InterpolationType observationInterpolation_;
    };


    class CPICapFloor::arguments : public virtual PricingEngine::arguments{
    public:
        Option::Type type;
        Real nominal;
        Date startDate, fixDate, payDate;
        Real baseCPI;
        Date maturity;
        Calendar fixCalendar, payCalendar;
        BusinessDayConvention fixConvention, payConvention;
        Rate strike;
        Handle<ZeroInflationIndex> infIndex;
        Period observationLag;
        CPI::InterpolationType observationInterpolation;

        void validate() const override;
    };


    class CPICapFloor::results : public Instrument::results {
    public:
      void reset() override;
    };


    class CPICapFloor::engine : public GenericEngine<CPICapFloor::arguments,
                                                     CPICapFloor::results> {
    };

}


#endif



#ifndef id_5f1d856f82fd5a0836fc07854efe625c
#define id_5f1d856f82fd5a0836fc07854efe625c
inline bool test_5f1d856f82fd5a0836fc07854efe625c(const int* i) {
    return i != nullptr;
}
#endif
