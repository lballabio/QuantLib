
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

/*! \file analytic_discr_geom_av_price.hpp
    \brief Analytic engine for discrete geometric average price Asian
*/

#ifndef quantlib_analytic_discrete_geometric_average_price_asian_engine_hpp
#define quantlib_analytic_discrete_geometric_average_price_asian_engine_hpp

#include <ql/Instruments/asianoption.hpp>

namespace QuantLib {

    //! Pricing engine for European discrete geometric average price Asian
    /*! This class implements a discrete geometric average price Asian
        option, with European exercise.  The formula is from "Asian
        Option", E. Levy (1997) in "Exotic Options: The State of the
        Art", edited by L. Clewlow, C. Strickland, pag 65-97

        \bug calculated Greeks do not match numerical results

        \test the correctness of the returned value is tested by
              reproducing results available in literature.

        \ingroup asianengines
    */
    class AnalyticDiscreteGeometricAveragePriceAsianEngine
        : public DiscreteAveragingAsianOption::engine {
      public:
        void calculate() const;
    };

}


#endif
