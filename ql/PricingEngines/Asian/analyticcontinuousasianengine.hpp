
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

/*! \file analyticcontinuousasianengine.hpp
    \brief Analytic continuous-averaging Asian option engine
*/

#ifndef quantlib_analytic_continuous_asian_engines_hpp
#define quantlib_analytic_continuous_asian_engines_hpp

#include <ql/Instruments/asianoption.hpp>

namespace QuantLib {

    //! Pricing engine for European continuous geometric average price Asian
    /*! This class implements a continuous geometric average price
        Asian option with European exercise.  The formula is from
        "Option Pricing Formulas", E. G. Haug (1997) pag 96-97.

        \ingroup asianengines
    */
    class AnalyticContinuousAveragingAsianEngine 
        : public ContinuousAveragingAsianOption::engine {
      public:
        void calculate() const;
    };

}


#endif
