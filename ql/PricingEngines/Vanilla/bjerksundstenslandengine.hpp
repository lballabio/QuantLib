
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

/*! \file bjerksundstenslandengine.hpp
    \brief Bjerksund and Stensland approximation engine
*/

#ifndef quantlib_bjerkland_stensland_engine_hpp
#define quantlib_bjerkland_stensland_engine_hpp

#include <ql/Instruments/vanillaoption.hpp>

namespace QuantLib {

    /*! Pricing engine for American options with 
        Bjerksund and Stensland approximation (1993)

        \ingroup vanillaengines
    */
    class BjerksundStenslandApproximationEngine 
        : public VanillaOption::engine {
      public:
        void calculate() const;
    };

}


#endif
