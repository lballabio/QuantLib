
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

/*! \file analyticdiscreteasianengine.hpp
    \brief Analytic discrete-averaging Asian option engine
*/

#ifndef quantlib_analytic_discrete_asian_engines_hpp
#define quantlib_analytic_discrete_asian_engines_hpp

#include <ql/Instruments/asianoption.hpp>

namespace QuantLib {

    //! Pricing engine for European discrete geometric average Asian option
    /*! This class implements a discrete geometric average price asian
        option, with european exercise.  The formula is from "Asian
        Option", E. Levy (1997) in "Exotic Options: The State of the
        Art", edited by L. Clewlow, C. Strickland, pag 65-97

        \bug calculated Greeks do not match numerical results

        \ingroup asianengines
    */
    class AnalyticDiscreteAveragingAsianEngine 
        : public DiscreteAveragingAsianOption::engine {
      public:
        void calculate() const;
    };

}


#endif
