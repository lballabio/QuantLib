
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

/*! \file stochasticprocess.hpp
    \brief Base stochastic process class
*/

#ifndef quantlib_stochasticprocess_h
#define quantlib_stochasticprocess_h

#include <ql/termstructure.hpp>
#include <ql/voltermstructure.hpp>

namespace QuantLib {

    //! Base stochastic process class
    /*! Just an arguments placeholder for the time being.
        To be merged/refactored with DiffusionProcess
    */
    class StochasticProcess : public Observer, public Observable {
      public:
        virtual ~StochasticProcess() {}
        void update() { notifyObservers(); }
    };

    class OneFactorStochasticProcess : public StochasticProcess {
      public:
        OneFactorStochasticProcess(
                                 const RelinkableHandle<Quote>& stateVariable)
        : stateVariable(stateVariable) {
            registerWith(stateVariable);
        }

        RelinkableHandle<Quote> stateVariable;
    };

    class BlackScholesStochasticProcess : public OneFactorStochasticProcess {
      public:
        BlackScholesStochasticProcess(
                         const RelinkableHandle<Quote>& stateVariable,
                         const RelinkableHandle<TermStructure>& dividendTS,
                         const RelinkableHandle<TermStructure>& riskFreeTS,
                         const RelinkableHandle<BlackVolTermStructure>& volTS)
        : OneFactorStochasticProcess(stateVariable), dividendTS(dividendTS),
          riskFreeTS(riskFreeTS), volTS(volTS) {
            registerWith(dividendTS);
            registerWith(riskFreeTS);
            registerWith(volTS);
        }

        RelinkableHandle<TermStructure> dividendTS, riskFreeTS;
        RelinkableHandle<BlackVolTermStructure> volTS;
    };

    class Merton76StochasticProcess : public BlackScholesStochasticProcess {
      public:
        Merton76StochasticProcess(
                         const RelinkableHandle<Quote>& stateVariable,
                         const RelinkableHandle<TermStructure>& dividendTS,
                         const RelinkableHandle<TermStructure>& riskFreeTS,
                         const RelinkableHandle<BlackVolTermStructure>& volTS,
                         const RelinkableHandle<Quote>& jumpInt,
                         const RelinkableHandle<Quote>& logJMean,
                         const RelinkableHandle<Quote>& logJVol)
        : BlackScholesStochasticProcess(stateVariable, dividendTS, 
                                        riskFreeTS, volTS),
          jumpIntensity(jumpInt), logJumpMean(logJMean),
          logJumpVolatility(logJVol) {
            registerWith(jumpIntensity);
            registerWith(logJumpMean);
            registerWith(logJumpVolatility);
        }

        RelinkableHandle<Quote> jumpIntensity, logJumpMean, 
                                logJumpVolatility;
    };

}


#endif
