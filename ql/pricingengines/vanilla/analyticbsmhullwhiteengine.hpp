/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Klaus Spanderen

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

/*! \file analyticbsmhullwhiteengine.hpp
    \brief analytic Black-Scholes engines including stochastic interest rates
*/

#ifndef quantlib_analytic_bsm_hull_white_engine_hpp
#define quantlib_analytic_bsm_hull_white_engine_hpp

#include <ql/instruments/vanillaoption.hpp>
#include <ql/pricingengines/genericmodelengine.hpp>
#include <ql/models/shortrate/onefactormodels/hullwhite.hpp>
#include <ql/processes/blackscholesprocess.hpp>

namespace QuantLib {

    //! analytic european option pricer including stochastic interest rates
    /*! References:

        Brigo, Mercurio, Interest Rate Models

        \ingroup vanillaengines

        \test the correctness of the returned value is tested by
              reproducing results available in web/literature
    */

    class AnalyticBSMHullWhiteEngine
        : public GenericModelEngine<HullWhite,
                                    VanillaOption::arguments,
                                    VanillaOption::results> {
      public:
        AnalyticBSMHullWhiteEngine(Real equityShortRateCorrelation,
                                   ext::shared_ptr<GeneralizedBlackScholesProcess>,
                                   const ext::shared_ptr<HullWhite>&);
        void calculate() const override;

      private:
        Real rho_;
        ext::shared_ptr<GeneralizedBlackScholesProcess> process_;
    };

}


#endif
