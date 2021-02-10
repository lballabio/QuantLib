/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003, 2004 Ferdinando Ametrano
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

/*! \file baroneadesiwhaleyengine.hpp
    \brief Barone-Adesi and Whaley approximation engine
*/

#ifndef quantlib_barone_adesi_whaley_engine_hpp
#define quantlib_barone_adesi_whaley_engine_hpp

#include <ql/instruments/vanillaoption.hpp>
#include <ql/processes/blackscholesprocess.hpp>

namespace QuantLib {

    //! Barone-Adesi and Whaley pricing engine for American options (1987)
    /*! \ingroup vanillaengines

        \test the correctness of the returned value is tested by
              reproducing results available in literature.
    */
    class BaroneAdesiWhaleyApproximationEngine
        : public VanillaOption::engine {
      public:
        BaroneAdesiWhaleyApproximationEngine(ext::shared_ptr<GeneralizedBlackScholesProcess>);
        static Real criticalPrice(
            const ext::shared_ptr<StrikedTypePayoff>& payoff,
            DiscountFactor riskFreeDiscount,
            DiscountFactor dividendDiscount,
            Real variance,
            Real tolerance = 1e-6);
        void calculate() const override;

      private:
        ext::shared_ptr<GeneralizedBlackScholesProcess> process_;
    };

}


#endif
