
/*
 Copyright (C) 2000-2003 RiskMap srl

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

/*! \file forwardspreadedtermstructure.hpp
    \brief Forward spreaded term structure
*/

#ifndef quantlib_forward_spreaded_term_structure_hpp
#define quantlib_forward_spreaded_term_structure_hpp

#include <ql/TermStructures/forwardstructure.hpp>

namespace QuantLib {

    //! Term structure with added spread on the instantaneous forward rate
    /*! \note This term structure will remain linked to the original
              structure, i.e., any changes in the latter will be
              reflected in this structure as well.

        \ingroup yieldtermstructures

        \test a) the correctness of the returned values is tested by
                 checking them against numerical calculations.

        \test b) observability against changes in the underlying term
                 structure and in the added spread is checked.
    */
    class ForwardSpreadedTermStructure : public ForwardRateStructure {
      public:
        ForwardSpreadedTermStructure(const Handle<YieldTermStructure>&,
                                     const Handle<Quote>& spread);
        //! \name YieldTermStructure interface
        //@{
        #ifndef QL_DISABLE_DEPRECATED
        DayCounter dayCounter() const { return originalCurve_->dayCounter(); }
        #endif
        Calendar calendar() const;
        #ifndef QL_DISABLE_DEPRECATED
        const Date& todaysDate() const;
        #endif
        const Date& referenceDate() const;
        Date maxDate() const;
        Time maxTime() const;
        //@}
      protected:
        //! returns the spreaded forward rate
        Rate forwardImpl(Time) const;
        //! returns the spreaded zero yield rate
        /*! \warning This method must disappear should the spread
                     become a curve
        */
        Rate zeroYieldImpl(Time) const;
      private:
        Handle<YieldTermStructure> originalCurve_;
        Handle<Quote> spread_;
    };

    inline ForwardSpreadedTermStructure::ForwardSpreadedTermStructure(
                                          const Handle<YieldTermStructure>& h,
                                          const Handle<Quote>& spread)
    : originalCurve_(h), spread_(spread) {
        registerWith(originalCurve_);
        registerWith(spread_);
    }

    inline Calendar ForwardSpreadedTermStructure::calendar() const {
        return originalCurve_->calendar();
    }

    #ifndef QL_DISABLE_DEPRECATED
    inline const Date& ForwardSpreadedTermStructure::todaysDate() const {
        return originalCurve_->todaysDate();
    }
    #endif

    inline const Date& ForwardSpreadedTermStructure::referenceDate() const {
        return originalCurve_->referenceDate();
    }

    inline Date ForwardSpreadedTermStructure::maxDate() const {
        return originalCurve_->maxDate();
    }

    inline Time ForwardSpreadedTermStructure::maxTime() const {
        return originalCurve_->maxTime();
    }

    inline Rate ForwardSpreadedTermStructure::forwardImpl(Time t) const {
        return originalCurve_->forwardRate(t, t, Continuous, NoFrequency, true)
            + spread_->value();
    }

    inline Rate ForwardSpreadedTermStructure::zeroYieldImpl(Time t) const {
        // return originalCurve_->zeroYield(t, true) + spread_->value();
        return originalCurve_->zeroRate(t, Continuous, NoFrequency) +
            spread_->value();
    }

}


#endif
