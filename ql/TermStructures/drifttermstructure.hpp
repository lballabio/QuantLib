/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 Ferdinando Ametrano

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file drifttermstructure.hpp
    \brief Drift term structure
*/

#ifndef quantlib_drift_term_structure_hpp
#define quantlib_drift_term_structure_hpp

#include <ql/TermStructures/zeroyieldstructure.hpp>
#include <ql/voltermstructure.hpp>

namespace QuantLib {

    //! Drift term structure
    /*! Drift term structure for modelling the common drift term:
        riskFreeRate - dividendYield - 0.5*vol*vol

        \note This term structure will remain linked to the original
              structures, i.e., any changes in the latters will be
              reflected in this structure as well.
    */
    class DriftTermStructure : public ZeroYieldStructure {
      public:
        DriftTermStructure(const Handle<YieldTermStructure>& riskFreeTS,
                           const Handle<YieldTermStructure>& dividendTS,
                           const Handle<BlackVolTermStructure>& blackVolTS);
        //! \name YieldTermStructure interface
        //@{
        DayCounter dayCounter() const { return riskFreeTS_->dayCounter(); }
        Calendar calendar() const;
        const Date& referenceDate() const;
        Date maxDate() const { return maxDate_; }
        //@}
      protected:
        //! returns the discount factor as seen from the evaluation date
        Rate zeroYieldImpl(Time) const;
      private:
        Handle<YieldTermStructure> riskFreeTS_, dividendTS_;
        Handle<BlackVolTermStructure> blackVolTS_;
        Real underlyingLevel_;
        Date maxDate_;
    };


    // inline definitions

    inline DriftTermStructure::DriftTermStructure(
                              const Handle<YieldTermStructure>& riskFreeTS,
                              const Handle<YieldTermStructure>& dividendTS,
                              const Handle<BlackVolTermStructure>& blackVolTS)
    : riskFreeTS_(riskFreeTS),
      dividendTS_(dividendTS),
      blackVolTS_(blackVolTS) {
        registerWith(riskFreeTS_);
        registerWith(dividendTS_);
        registerWith(blackVolTS_);

        maxDate_ = std::min(dividendTS_->maxDate(),
                            riskFreeTS_->maxDate());
        maxDate_ = std::min(maxDate_, blackVolTS_->maxDate());
    }

    inline Calendar DriftTermStructure::calendar() const {
        return riskFreeTS_->calendar();
    }

    inline const Date& DriftTermStructure::referenceDate() const {
        // warning: here it is assumed that all TS have the same referenceDate
        //          It should be QL_REQUIREd
        return riskFreeTS_->referenceDate();
    }

    inline Rate DriftTermStructure::zeroYieldImpl(Time t) const {
        // warning: here it is assumed that
        //          a) all TS have the same daycount.
        //          b) all TS have the same referenceDate
        //          It should be QL_REQUIREd
        return riskFreeTS_->zeroRate(t, Continuous, NoFrequency, true)
             - dividendTS_->zeroRate(t, Continuous, NoFrequency, true)
             - 0.5 * blackVolTS_->blackVol(t, underlyingLevel_, true)
                   * blackVolTS_->blackVol(t, underlyingLevel_, true);
    }
}


#endif
