
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

/*! \file quantotermstructure.hpp
    \brief Quanto term structure
*/

#ifndef quantlib_quanto_term_structure_hpp
#define quantlib_quanto_term_structure_hpp

#include <ql/termstructure.hpp>
#include <ql/voltermstructure.hpp>

namespace QuantLib {

    //! Quanto term structure
    /*! Quanto term structure for modelling quanto effect in 
        option pricing.
    
        \note This term structure will remain linked to the original
              structures, i.e., any changes in the latters will be
              reflected in this structure as well.
    */
    class QuantoTermStructure : public ZeroYieldStructure,
                                public Observer {
      public:
        QuantoTermStructure(
                  const RelinkableHandle<TermStructure>& underlyingDividendTS,
                  const RelinkableHandle<TermStructure>& riskFreeTS,
                  const RelinkableHandle<TermStructure>& foreignRiskFreeTS,
                  const RelinkableHandle<BlackVolTermStructure>& 
                  underlyingBlackVolTS,
                  double strike,
                  const RelinkableHandle<BlackVolTermStructure>& 
                  exchRateBlackVolTS,
                  double exchRateATMlevel,
                  double underlyingExchRateCorrelation);
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
        //! returns the zero yield as seen from the evaluation date
        Rate zeroYieldImpl(Time) const;
      private:
        RelinkableHandle<TermStructure> underlyingDividendTS_, riskFreeTS_,
            foreignRiskFreeTS_;
        RelinkableHandle<BlackVolTermStructure> underlyingBlackVolTS_,
            exchRateBlackVolTS_;
        double underlyingExchRateCorrelation_, strike_, exchRateATMlevel_;
        Date maxDate_;
    };


    // inline definitions

    inline QuantoTermStructure::QuantoTermStructure(
                  const RelinkableHandle<TermStructure>& underlyingDividendTS,
                  const RelinkableHandle<TermStructure>& riskFreeTS,
                  const RelinkableHandle<TermStructure>& foreignRiskFreeTS,
                  const RelinkableHandle<BlackVolTermStructure>& 
                  underlyingBlackVolTS,
                  double strike,
                  const RelinkableHandle<BlackVolTermStructure>& 
                  exchRateBlackVolTS,
                  double exchRateATMlevel,
                  double underlyingExchRateCorrelation)
    : underlyingDividendTS_(underlyingDividendTS),
      riskFreeTS_(riskFreeTS), foreignRiskFreeTS_(foreignRiskFreeTS),
      underlyingBlackVolTS_(underlyingBlackVolTS),
      exchRateBlackVolTS_(exchRateBlackVolTS),
      underlyingExchRateCorrelation_(underlyingExchRateCorrelation),
      strike_(strike), exchRateATMlevel_(exchRateATMlevel) {
        registerWith(underlyingDividendTS_);
        registerWith(riskFreeTS_);
        registerWith(foreignRiskFreeTS_);
        registerWith(underlyingBlackVolTS_);
        registerWith(exchRateBlackVolTS_);

        maxDate_ = QL_MIN(underlyingDividendTS_->maxDate(),
                          riskFreeTS_->maxDate());
        maxDate_ = QL_MIN(maxDate_, foreignRiskFreeTS_->maxDate());
        maxDate_ = QL_MIN(maxDate_, underlyingBlackVolTS_->maxDate());
        maxDate_ = QL_MIN(maxDate_, exchRateBlackVolTS_->maxDate());

    }

    inline DayCounter QuantoTermStructure::dayCounter() const {
        return underlyingDividendTS_->dayCounter();
    }

    inline Date QuantoTermStructure::todaysDate() const {
        return underlyingDividendTS_->todaysDate();
    }

    inline Date QuantoTermStructure::referenceDate() const {
        return underlyingDividendTS_->referenceDate();
    }

    inline void QuantoTermStructure::update() {
        notifyObservers();
    }

    inline Rate QuantoTermStructure::zeroYieldImpl(Time t) const {
        // warning: here it is assumed that all TS have the same daycount.
        //          It should be QL_REQUIREd, or maybe even enforced in the
        //          whole QuantLib
        return underlyingDividendTS_->zeroYield(t, true)
            +            riskFreeTS_->zeroYield(t, true)
            -     foreignRiskFreeTS_->zeroYield(t, true)
            + underlyingExchRateCorrelation_
            * underlyingBlackVolTS_->blackVol(t, strike_, true)
            *   exchRateBlackVolTS_->blackVol(t, exchRateATMlevel_, true);
    }

}


#endif
