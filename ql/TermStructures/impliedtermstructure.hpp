
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

        \ingroup termstructures
    */
    class ImpliedTermStructure : public DiscountStructure,
                                 public Observer {
      public:
        ImpliedTermStructure(const Handle<TermStructure>&,
                             const Date& newTodaysDate,
                             const Date& newReferenceDate);
        //! \name TermStructure interface
        //@{
        DayCounter dayCounter() const;
        Date todaysDate() const;
        Date referenceDate() const;
        Date maxDate() const;
        Time maxTime() const;
        //@}
        //! \name Observer interface
        //@{
        void update();
        //@}
      protected:
        //! returns the discount factor as seen from the evaluation date
        DiscountFactor discountImpl(Time) const;
      private:
        Handle<TermStructure> originalCurve_;
        Date newTodaysDate_, newReferenceDate_;
    };



    inline ImpliedTermStructure::ImpliedTermStructure(
                                               const Handle<TermStructure>& h,
                                               const Date& newTodaysDate, 
                                               const Date& newReferenceDate)
    : originalCurve_(h), newTodaysDate_(newTodaysDate),
      newReferenceDate_(newReferenceDate) {
        registerWith(originalCurve_);
    }

    inline DayCounter ImpliedTermStructure::dayCounter() const {
        return originalCurve_->dayCounter();
    }

    inline Date ImpliedTermStructure::todaysDate() const {
        return newTodaysDate_;
    }

    inline Date ImpliedTermStructure::referenceDate() const {
        return newReferenceDate_;
    }

    inline Date ImpliedTermStructure::maxDate() const {
        return originalCurve_->maxDate();
    }

    inline Time ImpliedTermStructure::maxTime() const {
        return dayCounter().yearFraction(
                                 newReferenceDate_,originalCurve_->maxDate());
    }

    inline void ImpliedTermStructure::update() {
        notifyObservers();
    }

    inline DiscountFactor ImpliedTermStructure::discountImpl(Time t) const {
        /* t is relative to the current reference date
           and needs to be converted to the time relative
           to the reference date of the original curve */
        Time originalTime = 
            t + dayCounter().yearFraction(originalCurve_->referenceDate(), 
                                          newReferenceDate_);
        /* discount at evaluation date cannot be cached
           since the original curve could change between
           invocations of this method */
        return originalCurve_->discount(originalTime, true) /
            originalCurve_->discount(referenceDate(), true);
    }

}


#endif
