
/*
 Copyright (C) 2003 Ferdinando Ametrano

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file quantotermstructure.hpp
    \brief Quanto term structure

    \fullpath
    ql/TermStructures/%quantotermstructure.hpp
*/

// $Id$

#ifndef quantlib_quanto_term_structure_hpp
#define quantlib_quanto_term_structure_hpp

#include <ql/termstructure.hpp>
#include <ql/voltermstructure.hpp>

namespace QuantLib {
    namespace TermStructures {

        //! Quanto term structure
        /*! Quanto term structure for modelling quanto effect in option pricing.
            \note This term structure will remain linked to the original
                structures, i.e., any changes in the latters will be reflected in
                this structure as well.
        */
        class QuantoTermStructure : public ZeroYieldStructure,
                                    public Patterns::Observer {
          public:
              QuantoTermStructure(
                  const RelinkableHandle<TermStructure>& underlyingDividendTS,
                  const RelinkableHandle<TermStructure>& riskFreeTS,
                  const RelinkableHandle<TermStructure>& foreignRiskFreeTS,
                  const RelinkableHandle<BlackVolTermStructure>& underlyingLocalVolTS,
                  const RelinkableHandle<BlackVolTermStructure>& exchRateLocalVolTS,
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
            //! returns the discount factor as seen from the evaluation date
            Rate zeroYieldImpl(Time, bool extrapolate = false) const;
          private:
            RelinkableHandle<TermStructure> underlyingDividendTS_, riskFreeTS_,
                foreignRiskFreeTS_;
            RelinkableHandle<BlackVolTermStructure> underlyingLocalVolTS_,
                exchRateLocalVolTS_;
            double underlyingExchRateCorrelation_, underlyingLevel_;
            Date maxDate_;

        };



        inline QuantoTermStructure::QuantoTermStructure(
            const RelinkableHandle<TermStructure>& underlyingDividendTS,
            const RelinkableHandle<TermStructure>& riskFreeTS,
            const RelinkableHandle<TermStructure>& foreignRiskFreeTS,
            const RelinkableHandle<BlackVolTermStructure>& underlyingLocalVolTS,
            const RelinkableHandle<BlackVolTermStructure>& exchRateLocalVolTS,
            double underlyingExchRateCorrelation)
        : underlyingDividendTS_(underlyingDividendTS),
          riskFreeTS_(riskFreeTS), foreignRiskFreeTS_(foreignRiskFreeTS),
          underlyingLocalVolTS_(underlyingLocalVolTS),
          exchRateLocalVolTS_(exchRateLocalVolTS),
          underlyingExchRateCorrelation_(underlyingExchRateCorrelation) {
            registerWith(underlyingDividendTS_);
            registerWith(riskFreeTS_);
            registerWith(foreignRiskFreeTS_);
            registerWith(underlyingLocalVolTS_);
            registerWith(exchRateLocalVolTS_);

            maxDate_ = QL_MIN(underlyingDividendTS_->maxDate(),
                riskFreeTS_->maxDate());
            maxDate_ = QL_MIN(maxDate_, foreignRiskFreeTS_->maxDate());
            maxDate_ = QL_MIN(maxDate_, underlyingLocalVolTS_->maxDate());
            maxDate_ = QL_MIN(maxDate_, exchRateLocalVolTS_->maxDate());

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

        inline Rate QuantoTermStructure::zeroYieldImpl(Time t,
            bool extrapolate) const {
            // warning: here it is assumed that all TS have the same daycount.
            //          It should be QL_REQUIREd, or maybe even enforced in the
            //          whole QuantLib
                return underlyingDividendTS_->zeroYield(t, extrapolate)
                    + riskFreeTS_->discount(referenceDate(),extrapolate)
                    - foreignRiskFreeTS_->discount(referenceDate(),extrapolate)
                    + underlyingExchRateCorrelation_
                      * underlyingLocalVolTS_->blackVol(t, underlyingLevel_,
                                                                 extrapolate)
                      * exchRateLocalVolTS_->blackVol(t, underlyingLevel_,
                                                                extrapolate);
        }
    }
}


#endif
