/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005, 2007 StatPro Italia srl

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

/*! \file flatforward.hpp
    \brief flat forward rate term structure
*/

#ifndef quantlib_flat_forward_curve_hpp
#define quantlib_flat_forward_curve_hpp

#include <ql/patterns/lazyobject.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/quote.hpp>

namespace QuantLib {

    //! Flat interest-rate curve
    /*! \ingroup yieldtermstructures */
    class FlatForward : public YieldTermStructure,
                        public LazyObject {
      public:
        //! \name Constructors
        //@{
        FlatForward(const Date& referenceDate,
                    Handle<Quote> forward,
                    const DayCounter& dayCounter,
                    Compounding compounding = Continuous,
                    Frequency frequency = Annual);
        FlatForward(const Date& referenceDate,
                    Rate forward,
                    const DayCounter& dayCounter,
                    Compounding compounding = Continuous,
                    Frequency frequency = Annual);
        FlatForward(Natural settlementDays,
                    const Calendar& calendar,
                    Handle<Quote> forward,
                    const DayCounter& dayCounter,
                    Compounding compounding = Continuous,
                    Frequency frequency = Annual);
        FlatForward(Natural settlementDays,
                    const Calendar& calendar,
                    Rate forward,
                    const DayCounter& dayCounter,
                    Compounding compounding = Continuous,
                    Frequency frequency = Annual);
        //@}

        // inspectors
        Compounding compounding() const { return compounding_; }
        Frequency compoundingFrequency() const { return frequency_; }

        //! \name TermStructure interface
        //@{
        Date maxDate() const override { return Date::maxDate(); }
        //@}

        //! \name Observer interface
        //@{
        void update() override;
        //@}
      private:
        //! \name LazyObject interface
        //@{
        void performCalculations() const override;
        //@}

        //! \name YieldTermStructure implementation
        //@{
        DiscountFactor discountImpl(Time) const override;
        //@}

        Handle<Quote> forward_;
        Compounding compounding_;
        Frequency frequency_;
        mutable InterestRate rate_;
    };

    // inline definitions

    inline void FlatForward::update() {
        LazyObject::update();
        YieldTermStructure::update();
    }

    inline DiscountFactor FlatForward::discountImpl(Time t) const {
        calculate();
        return rate_.discountFactor(t);
    }
  
    inline void FlatForward::performCalculations() const {
        rate_ = InterestRate(forward_->value(), dayCounter(),
                             compounding_, frequency_);
    }

}

#endif


#ifndef id_c29cecd937ff5d1b0e352e32806b3096
#define id_c29cecd937ff5d1b0e352e32806b3096
inline bool test_c29cecd937ff5d1b0e352e32806b3096(int* i) { return i != 0; }
#endif
