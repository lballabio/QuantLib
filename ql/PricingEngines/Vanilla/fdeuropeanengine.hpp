
/*
 Copyright (C) 2005 Joseph Wang

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

/*! \file fdeuropeanengine.hpp
    \brief Finite-difference European engine
*/

#ifndef quantlib_fd_european_engine_hpp
#define quantlib_fd_european_engine_hpp

#include <ql/Instruments/vanillaoption.hpp>

namespace QuantLib {

    //! Pricing engine for European vanilla options using finite-differences
    /*! \ingroup vanillaengines

        \test the correctness of the returned value is tested by
              checking it against analytic results.
    */
    class FDEuropeanEngine : public VanillaOption::engine {
      public:
        FDEuropeanEngine(Size timeSteps, Size gridPoints)
        : timeSteps_(timeSteps), gridPoints_(gridPoints) {};
        void calculate() const;
      private:
        Size timeSteps_;
        Size gridPoints_;
    };

}


#endif
