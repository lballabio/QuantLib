
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

/*! \file zerospreadedtermstructure.hpp
    \brief Zero spreaded term structure
*/

#ifndef quantlib_zero_spreaded_term_structure_hpp
#define quantlib_zero_spreaded_term_structure_hpp

#include <ql/TermStructures/zeroyieldstructure.hpp>

namespace QuantLib {

    //! Term structure with an added spread on the zero yield rate
    /*! \note This term structure will remain linked to the original
              structure, i.e., any changes in the latter will be
              reflected in this structure as well.

        \ingroup yieldtermstructures

        \test a) the correctness of the returned values is tested by
                 checking them against numerical calculations.

        \test b) observability against changes in the underlying term
                 structure and in the added spread is checked.
    */
    class ZeroSpreadedTermStructure : public ZeroYieldStructure {
      public:
        ZeroSpreadedTermStructure(const Handle<YieldTermStructure>&,
                                  const Handle<Quote>& spread);
        //! \name YieldTermStructure interface
        //@{
        #ifndef QL_DISABLE_DEPRECATED
        DayCounter dayCounter() const { return originalCurve_->dayCounter(); }
        #endif
        Calendar calendar() const;
        const Date& referenceDate() const;
        #ifndef QL_DISABLE_DEPRECATED
        const Date& todaysDate() const;
        #endif
        Date maxDate() const;
        Time maxTime() const;
        //@}
      protected:
        //! returns the spreaded zero yield rate
        Rate zeroYieldImpl(Time) const;
        //! returns the spreaded forward rate
        /*! \warning This method must disappear should the spread become a
                     curve
        */
        Rate forwardImpl(Time) const;
      private:
        Handle<YieldTermStructure> originalCurve_;
        Handle<Quote> spread_;
    };

    inline ZeroSpreadedTermStructure::ZeroSpreadedTermStructure(
                                          const Handle<YieldTermStructure>& h,
                                          const Handle<Quote>& spread)
    : originalCurve_(h), spread_(spread) {
        registerWith(originalCurve_);
        registerWith(spread_);
    }

    inline Calendar ZeroSpreadedTermStructure::calendar() const {
        return originalCurve_->calendar();
    }

    #ifndef QL_DISABLE_DEPRECATED
    inline const Date& ZeroSpreadedTermStructure::todaysDate() const {
        return originalCurve_->todaysDate();
    }
    #endif

    inline const Date& ZeroSpreadedTermStructure::referenceDate() const {
        return originalCurve_->referenceDate();
    }

    inline Date ZeroSpreadedTermStructure::maxDate() const {
        return originalCurve_->maxDate();
    }

    inline Time ZeroSpreadedTermStructure::maxTime() const {
        return originalCurve_->maxTime();
    }

    inline Rate ZeroSpreadedTermStructure::zeroYieldImpl(Time t) const {
        return originalCurve_->zeroYield(t, true) + spread_->value();
    }

    inline Rate ZeroSpreadedTermStructure::forwardImpl(Time t) const {
        return originalCurve_->instantaneousForward(t, true) +
               spread_->value();
    }

}


#endif
