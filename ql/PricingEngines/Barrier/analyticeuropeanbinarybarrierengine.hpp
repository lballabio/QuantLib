
/*
 Copyright (C) 2003, 2004 Neil Firth
 Copyright (C) 2003, 2004 Ferdinando Ametrano
 Copyright (C) 2003, 2004 StatPro Italia srl

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

/*! \file analyticeuropeanbinarybarrierengine.hpp
    \brief analytic European binary barrier option engine
*/

#ifndef quantlib_analytic_european_binarybarrier_engine_hpp
#define quantlib_analytic_european_binarybarrier_engine_hpp

#include <ql/Instruments/binarybarrieroption.hpp>

namespace QuantLib {

    //! Analytic pricing engine for European binary barrier options
    class AnalyticEuropeanBinaryBarrierEngine : public BinaryBarrierEngine {
      public:
        void calculate() const;
    };

}


#endif
