
/*
 Copyright (C) 2003, 2004 Ferdinando Ametrano

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

/*! \file analytic_cont_geom_av_price.hpp
    \brief Analytic engine for continuous geometric average price Asian
*/

#ifndef quantlib_analytic_continuous_geometric_average_price_asian_engine_hpp
#define quantlib_analytic_continuous_geometric_average_price_asian_engine_hpp

#include <ql/Instruments/asianoption.hpp>

namespace QuantLib {

    //! Pricing engine for European continuous geometric average price Asian
    /*! This class implements a continuous geometric average price
        Asian option with European exercise.  The formula is from
        "Option Pricing Formulas", E. G. Haug (1997) pag 96-97.

        \ingroup asianengines

        \todo handle seasoned options

    */
    class AnalyticContinuousGeometricAveragePriceAsianEngine 
        : public ContinuousAveragingAsianOption::engine {
      public:
        void calculate() const;
    };

}


#endif
