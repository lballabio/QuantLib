/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2010 Michael Heckl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/experimental/processes/vegastressedblackscholesprocess.hpp>

namespace QuantLib {

    VegaStressedBlackScholesProcess::VegaStressedBlackScholesProcess(
             const Handle<Quote>& x0,
             const Handle<YieldTermStructure>& dividendTS,
             const Handle<YieldTermStructure>& riskFreeTS,
             const Handle<BlackVolTermStructure>& blackVolTS,
             Time lowerTimeBorderForStressTest,
             Time upperTimeBorderForStressTest,
             Real lowerAssetBorderForStressTest,
             Real upperAssetBorderForStressTest,
             Real stressLevel,
             const ext::shared_ptr<discretization>& disc)
    : GeneralizedBlackScholesProcess(x0, dividendTS, riskFreeTS,
                                     blackVolTS, disc),
      lowerTimeBorderForStressTest_(lowerTimeBorderForStressTest),
      upperTimeBorderForStressTest_(upperTimeBorderForStressTest), 
      lowerAssetBorderForStressTest_(lowerAssetBorderForStressTest), 
      upperAssetBorderForStressTest_(upperAssetBorderForStressTest),
      stressLevel_(stressLevel) {}

    // returns the lower time border for the stress test
    Real VegaStressedBlackScholesProcess::getLowerTimeBorderForStressTest() const {
        return lowerTimeBorderForStressTest_;
    }

    // returns the upper time border for the stress test
    Real VegaStressedBlackScholesProcess::getUpperTimeBorderForStressTest() const {
        return upperTimeBorderForStressTest_;
    }

    // returns the lower asset border for the stress test
    Real VegaStressedBlackScholesProcess::getLowerAssetBorderForStressTest() const {
        return lowerAssetBorderForStressTest_;
    }

    // returns the upper asset border for the stress test
    Real VegaStressedBlackScholesProcess::getUpperAssetBorderForStressTest() const {
        return upperAssetBorderForStressTest_;
    }

    // returns the stress Level
    Real VegaStressedBlackScholesProcess::getStressLevel() const {
        return stressLevel_;
    }


    // set the lower time border for the stress test
    void VegaStressedBlackScholesProcess::setLowerTimeBorderForStressTest(Time LTB) {
        lowerTimeBorderForStressTest_ = LTB;
        update();
    }

    // set the upper time border for the stress test
    void VegaStressedBlackScholesProcess::setUpperTimeBorderForStressTest(Time UTB) {
        upperTimeBorderForStressTest_ = UTB;
        update();
    }

    // set the lower asset border for the stress test
    void VegaStressedBlackScholesProcess::setLowerAssetBorderForStressTest(Real LAB) {
        lowerAssetBorderForStressTest_ = LAB;
        update();
    }

    // set the upper asset border for the stress test
    void VegaStressedBlackScholesProcess::setUpperAssetBorderForStressTest(Real UBA) {
        upperAssetBorderForStressTest_ = UBA;
        update();
    }

    // set the stress Level
    void VegaStressedBlackScholesProcess::setStressLevel(Real SL) {
        stressLevel_ = SL;
        update();
    }



    Real VegaStressedBlackScholesProcess::diffusion(Time t, Real x) const {
        if (lowerTimeBorderForStressTest_ <= t && t <= upperTimeBorderForStressTest_ 
            && lowerAssetBorderForStressTest_ <= x && x <= upperAssetBorderForStressTest_) {
            return GeneralizedBlackScholesProcess::diffusion(t, x)+stressLevel_;
        } else {
            return GeneralizedBlackScholesProcess::diffusion(t, x);
        }
    }

}
