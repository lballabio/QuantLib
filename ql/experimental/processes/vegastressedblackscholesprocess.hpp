/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2010 Michael Heckl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file vegastressedblackscholesprocess.hpp
    \brief Black-Scholes process which supports local vega stress tests
*/

#ifndef quantlib_vega_stressed_black_scholes_process_hpp
#define quantlib_vega_stressed_black_scholes_process_hpp

#include <ql/processes/blackscholesprocess.hpp>

namespace QuantLib {

    //! Black-Scholes process which supports local vega stress tests
    class VegaStressedBlackScholesProcess
        : public GeneralizedBlackScholesProcess {
      public:
        VegaStressedBlackScholesProcess(
            const Handle<Quote>& x0,
            const Handle<YieldTermStructure>& dividendTS,
            const Handle<YieldTermStructure>& riskFreeTS,
            const Handle<BlackVolTermStructure>& blackVolTS,
            Time lowerTimeBorderForStressTest = 0,
            Time upperTimeBorderForStressTest = 1000000,
            Real lowerAssetBorderForStressTest = 0,
            Real upperAssetBorderForStressTest = 1000000,
            Real stressLevel = 0,
            const ext::shared_ptr<discretization>& d =
                  ext::shared_ptr<discretization>(new EulerDiscretization));
        //! \name StochasticProcess1D interface
        //@{
        Real diffusion(Time t, Real x) const override;
        //@}
        //! \name interface for vega stress test
        //@{
        // give back lower time border for stress test
        Real getLowerTimeBorderForStressTest() const;
        // set lower time border for stress test
        void setLowerTimeBorderForStressTest(Time LTB);
        // give back upper time border for stress test
        Real getUpperTimeBorderForStressTest() const;
        // set upper time border for stress test
        void setUpperTimeBorderForStressTest(Time UTB);
        // give back lower asset border for stress test
        Real getLowerAssetBorderForStressTest() const;
        // set lower asset border for stress test
        void setLowerAssetBorderForStressTest(Real LAB);
        // give back upper asset border for stress test
        Real getUpperAssetBorderForStressTest() const;
        // set upper asset border for stress test
        void setUpperAssetBorderForStressTest(Real UBA);
        // give back stress level
        Real getStressLevel() const;
        // set stress level
        void setStressLevel(Real SL);
        //@}
      private:
        Real lowerTimeBorderForStressTest_;
        Real upperTimeBorderForStressTest_;
        Real lowerAssetBorderForStressTest_;
        Real upperAssetBorderForStressTest_;
        Real stressLevel_;
    };

}


#endif
