
/*
 Copyright (C) 2004 Neil Firth

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

/*! \file stulzengine.hpp
    \brief 2D European Basket formulae, due to Stulz (1982)
*/

#ifndef quantlib_stulz_engine_hpp
#define quantlib_stulz_engine_hpp

#include <ql/Instruments/basketoption.hpp>

namespace QuantLib {

    //! Pricing engine for 2D European Baskets
    /*! This class implements formulae from 
        "Options on the Minimum or the Maximum of Two Risky Assets", 
            Rene Stulz, 
            Journal of Financial Ecomomics (1982) 10, 161-185.
    */
    class StulzEngine : public BasketOption::engine {
      public:
        void calculate() const;
    };

}


#endif
