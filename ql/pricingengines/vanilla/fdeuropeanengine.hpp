/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005 Joseph Wang
 Copyright (C) 2007 StatPro Italia srl

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

/*! \file fdeuropeanengine.hpp
    \brief Finite-difference European engine
*/

#ifndef quantlib_fd_european_engine_hpp
#define quantlib_fd_european_engine_hpp

#include <ql/instruments/oneassetoption.hpp>
#include <ql/pricingengines/vanilla/fdvanillaengine.hpp>
#include <ql/math/sampledcurve.hpp>

namespace QuantLib {

    //! Pricing engine for European options using finite-differences
    /*! \ingroup vanillaengines

        \test the correctness of the returned value is tested by
              checking it against analytic results.
    */
    class FDEuropeanEngine : public OneAssetOption::engine,
                             public FDVanillaEngine {
      public:
        FDEuropeanEngine(
             const boost::shared_ptr<GeneralizedBlackScholesProcess>& process,
             Size timeSteps=100, Size gridPoints=100,
             bool timeDependent = false)
        : FDVanillaEngine(process, timeSteps, gridPoints, timeDependent),
          prices_(gridPoints) {
            registerWith(process);
        }
      private:
        mutable SampledCurve prices_;
        void calculate() const;
    };

}


#endif
