
/*
 Copyright (C) 2003 Neil Firth
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file mcbarrierengines.hpp
    \brief Monte Carlo Barrier option engines

    Based on the MC Vanilla Engine pattern
*/

//#include <ql/exercise.hpp>
//#include <ql/handle.hpp>
//#include <ql/payoff.hpp>
//#include <ql/termstructure.hpp>
//#include <ql/voltermstructure.hpp>
//#include <ql/Instruments/barrieroption.hpp>
//#include <ql/Math/normaldistribution.hpp>
//#include <ql/Pricers/barrieroption.hpp>
//#include <ql/PricingEngines/genericengine.hpp>
//#include <ql/PricingEngines/vanillaengines.hpp>
#include <ql/PricingEngines/barrierengines.hpp>

using QuantLib::Instruments::BarrierOption::BarrierType;
using QuantLib::Instruments::BarrierOptionArguments;
using QuantLib::Instruments::BarrierOptionResults;

namespace QuantLib {

    namespace PricingEngines {
        

    }
}