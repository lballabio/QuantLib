
/*
 Copyright (C) 2003 Ferdinando Ametrano

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

/*! \file drifttermstructure.hpp
    \brief Drift term structure
*/

#ifndef quantlib_drift_term_structure_hpp
#define quantlib_drift_term_structure_hpp

#include <ql/termstructure.hpp>
#include <ql/voltermstructure.hpp>

namespace QuantLib {

    //! Drift term structure
    /*! Drift term structure for modelling the common drift term:
        riskFreeRate - dividendYield - 0.5*vol*vol
    
        \note This term structure will remain linked to the original
              structures, i.e., any changes in the latters will be
              reflected in this structure as well.
    */
    class DriftTermStructure : public ZeroYieldStructure,
                               public Observer {
      public:
        DriftTermStructure(
                   const RelinkableHandle<TermStructure>& riskFreeTS,
                   const RelinkableHandle<TermStructure>& dividendTS,
                   const RelinkableHandle<BlackVolTermStructure>& blackVolTS);
        //! \name TermStructure interface
        //@{
        DayCounter dayCounter() const;
        Date todaysDate() const;
        Date referenceDate() const;
        Date maxDate() const {return maxDate_; }
        //@}
        //! \name Observer interface
        //@{
        void update();
        //@}
      protected:
        //! returns the discount factor as seen from the evaluation date
        Rate zeroYieldImpl(Time) const;
      private:
        RelinkableHandle<TermStructure> riskFreeTS_, dividendTS_;
        RelinkableHandle<BlackVolTermStructure> blackVolTS_;
        Real underlyingLevel_;
        Date maxDate_;
    };



    inline DriftTermStructure::DriftTermStructure(
                    const RelinkableHandle<TermStructure>& riskFreeTS,
                    const RelinkableHandle<TermStructure>& dividendTS,
                    const RelinkableHandle<BlackVolTermStructure>& blackVolTS)
    : riskFreeTS_(riskFreeTS),
      dividendTS_(dividendTS),
      blackVolTS_(blackVolTS) {
        registerWith(riskFreeTS_);
        registerWith(dividendTS_);
        registerWith(blackVolTS_);

        maxDate_ = QL_MIN(dividendTS_->maxDate(),
                          riskFreeTS_->maxDate());
        maxDate_ = QL_MIN(maxDate_, blackVolTS_->maxDate());
    }

    inline DayCounter DriftTermStructure::dayCounter() const {
        return riskFreeTS_->dayCounter();
    }

    inline Date DriftTermStructure::todaysDate() const {
        return riskFreeTS_->todaysDate();
    }

    inline Date DriftTermStructure::referenceDate() const {
        return riskFreeTS_->referenceDate();
    }

    inline void DriftTermStructure::update() {
        notifyObservers();
    }

    inline Rate DriftTermStructure::zeroYieldImpl(Time t) const {
        // warning: here it is assumed that all TS have the same daycount.
        //          It should be QL_REQUIREd, or maybe even enforced in the
        //          whole QuantLib
        return riskFreeTS_->zeroYield(referenceDate(),true)
            - dividendTS_->zeroYield(t, true)
            - 0.5 * blackVolTS_->blackVol(t, underlyingLevel_, true)
                  * blackVolTS_->blackVol(t, underlyingLevel_, true);
    }
}


#endif
