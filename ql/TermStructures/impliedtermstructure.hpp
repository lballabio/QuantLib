
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

/*! \file impliedtermstructure.hpp
    \brief Implied term structure
*/

#ifndef quantlib_implied_term_structure_hpp
#define quantlib_implied_term_structure_hpp

#include <ql/termstructure.hpp>

namespace QuantLib {

    //! Implied term structure at a given date in the future
    /*! The given date will be the implied reference date.

        \note This term structure will remain linked to the original
              structure, i.e., any changes in the latter will be
              reflected in this structure as well.

        \ingroup yieldtermstructures

        \test a) the correctness of the returned values is tested by
                 checking them against numerical calculations.

        \test b) observability against changes in the underlying term
                 structure is checked.
    */
    class ImpliedTermStructure : public DiscountStructure {
      public:
        #ifndef QL_DISABLE_DEPRECATED
        /*! \deprecated use the constructor without today's date; set the
                        evaluation date through Settings::instance().
        */
        ImpliedTermStructure(const Handle<YieldTermStructure>&,
                             const Date& newTodaysDate,
                             const Date& newReferenceDate);
        #endif
        ImpliedTermStructure(const Handle<YieldTermStructure>&,
                             const Date& referenceDate);
        //! \name YieldTermStructure interface
        //@{
        DayCounter dayCounter() const;
        Calendar calendar() const;
        Date maxDate() const;
        //@}
      protected:
        //! returns the discount factor as seen from the evaluation date
        DiscountFactor discountImpl(Time) const;
      private:
        Handle<YieldTermStructure> originalCurve_;
    };


    // inline definitions

    #ifndef QL_DISABLE_DEPRECATED
    inline ImpliedTermStructure::ImpliedTermStructure(
                                          const Handle<YieldTermStructure>& h,
                                          const Date& todaysDate,
                                          const Date& referenceDate)
    : DiscountStructure(todaysDate, referenceDate),
      originalCurve_(h) {
        registerWith(originalCurve_);
    }
    #endif

    inline ImpliedTermStructure::ImpliedTermStructure(
                                          const Handle<YieldTermStructure>& h,
                                          const Date& referenceDate)
    : DiscountStructure(referenceDate), originalCurve_(h) {
        registerWith(originalCurve_);
    }

    inline DayCounter ImpliedTermStructure::dayCounter() const {
        return originalCurve_->dayCounter();
    }

    inline Calendar ImpliedTermStructure::calendar() const {
        return originalCurve_->calendar();
    }

    inline Date ImpliedTermStructure::maxDate() const {
        return originalCurve_->maxDate();
    }

    inline DiscountFactor ImpliedTermStructure::discountImpl(Time t) const {
        /* t is relative to the current reference date
           and needs to be converted to the time relative
           to the reference date of the original curve */
        Date ref = referenceDate();
        Time originalTime =
            t + dayCounter().yearFraction(originalCurve_->referenceDate(),ref);
        /* discount at evaluation date cannot be cached
           since the original curve could change between
           invocations of this method */
        return originalCurve_->discount(originalTime, true) /
               originalCurve_->discount(ref, true);
    }

}


#endif
