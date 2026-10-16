/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file shiftedtermstructure.hpp
    \brief Shifted yield term structure
*/

#ifndef quantlib_shifted_term_structure_hpp
#define quantlib_shifted_term_structure_hpp

#include <ql/termstructures/yield/derivedtermstructure.hpp>
#include <utility>

namespace QuantLib {

    //! Shifted yield term structure at a given date in the future
    /*!
        \note This term structure will remain linked to the original
              term structure, i.e., any changes in the latter will be
              reflected in this structure as well.

        \ingroup yieldtermstructures

        \test
        - the correctness of the returned values is tested by
          checking them against numerical calculations
    */

    class ShiftedYieldTermStructure : public DerivedYieldTermStructure<> {
      public:
        ShiftedYieldTermStructure(Handle<YieldTermStructure> h, const Date& referenceDate);
        ShiftedYieldTermStructure(Handle<YieldTermStructure> h, Natural settlementDays,
                                  const Calendar& calendar);
      protected:
        //! \name YieldTermStructure implementation
        //@{
        DiscountFactor discountImpl(Time) const override;
        //@}
    };

    // inline definitions

    inline ShiftedYieldTermStructure::ShiftedYieldTermStructure(Handle<YieldTermStructure> h,
                                                                const Date& referenceDate)
    : DerivedYieldTermStructure(std::move(h), referenceDate) {}

    inline ShiftedYieldTermStructure::ShiftedYieldTermStructure(Handle<YieldTermStructure> h,
                                                                Natural settlementDays,
                                                                const Calendar& calendar)
    : DerivedYieldTermStructure(std::move(h), settlementDays, calendar) {}

    inline DiscountFactor ShiftedYieldTermStructure::discountImpl(Time t) const {
        // t is relative to the current reference date
        return originalCurve_->discount(t, true);
    }

}


#endif
