
/*
 Copyright (C) 2003 Neil Firth
 Copyright (C) 2002, 2003 Ferdinando Ametrano
 Copyright (C) 2002, 2003 Sad Rejeb
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

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

/*! \file analyticamericanbinarybarrierengine.hpp
    \brief analytic American binary barrier option engines
*/

#ifndef quantlib_analytic_american_binarybarrier_engines_hpp
#define quantlib_analytic_american_binarybarrier_engines_hpp

#include <ql/Instruments/binarybarrieroption.hpp>

namespace QuantLib {

    /*! Analytic pricing engine for American binary barrier options formulae

        \deprecated use AnalyticAmericanEngine instead
    */
    class AnalyticAmericanBinaryBarrierEngine : public BinaryBarrierEngine {
      public:
        void calculate() const;
    };

}


#endif
