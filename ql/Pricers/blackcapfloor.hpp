/*
 Copyright (C) 2001, 2002 Sadruddin Rejeb

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
/*! \file blackcapfloor.hpp
    \brief CapFloor calculated using the Black formula

    \fullpath
    ql/Pricers/%blackcapfloor.hpp
*/

// $Id$

#ifndef quantlib_pricers_black_capfloor_h
#define quantlib_pricers_black_capfloor_h

#include <ql/Instruments/capfloor.hpp>
#include <ql/InterestRateModelling/blackmodel.hpp>

namespace QuantLib {

    namespace Pricers {

        //! CapFloor priced by the Black formula
        class BlackCapFloor : public
            CapFloorPricingEngine<InterestRateModelling::BlackModel> {
          public:
            BlackCapFloor(const Handle<InterestRateModelling::BlackModel>& mod)
            : CapFloorPricingEngine<InterestRateModelling::BlackModel>(mod) {}
            void calculate() const;
        };

    }

}

#endif
