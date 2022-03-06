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

/*! \file analytic_discr_geom_av_price.hpp
    \brief Analytic engine for discrete geometric average price Asian
*/

#ifndef quantlib_analytic_discrete_geometric_average_price_asian_engine_hpp
#define quantlib_analytic_discrete_geometric_average_price_asian_engine_hpp

#include <ql/instruments/asianoption.hpp>
#include <ql/processes/blackscholesprocess.hpp>

namespace QuantLib {

    //! Pricing engine for European discrete geometric average price Asian
    /*! This class implements a discrete geometric average price Asian
        option, with European exercise.  The formula is from "Asian
        Option", E. Levy (1997) in "Exotic Options: The State of the
        Art", edited by L. Clewlow, C. Strickland, pag 65-97

        \todo implement correct theta, rho, and dividend-rho calculation

        \test
        - the correctness of the returned value is tested by
          reproducing results available in literature.
        - the correctness of the available greeks is tested against
          numerical calculations.

        \ingroup asianengines
    */
    class AnalyticDiscreteGeometricAveragePriceAsianEngine
        : public DiscreteAveragingAsianOption::engine {
      public:
        AnalyticDiscreteGeometricAveragePriceAsianEngine(
            ext::shared_ptr<GeneralizedBlackScholesProcess> process);
        void calculate() const override;

      private:
        ext::shared_ptr<GeneralizedBlackScholesProcess> process_;
    };

}


#endif


#ifndef id_394e9a665e059e62fb6c074b12352b31
#define id_394e9a665e059e62fb6c074b12352b31
inline bool test_394e9a665e059e62fb6c074b12352b31(const int* i) {
    return i != nullptr;
}
#endif
