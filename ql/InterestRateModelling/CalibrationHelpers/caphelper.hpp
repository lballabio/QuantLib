
/*
 Copyright (C) 2000, 2001, 2002 Sadruddin Rejeb

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software developed by the QuantLib Group; you can
 redistribute it and/or modify it under the terms of the QuantLib License;
 either version 1.0, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 QuantLib License for more details.

 You should have received a copy of the QuantLib License along with this
 program; if not, please email ferdinando@ametrano.net

 The QuantLib License is also available at http://quantlib.org/license.html
 The members of the QuantLib Group are listed in the QuantLib License
*/
/*! \file caphelper.hpp
    \brief CapHelper calibration helper

    \fullpath
    ql/InterestRateModelling/CalibrationHelpers/%caphelper.hpp
*/

// $Id$

#ifndef quantlib_interest_rate_modelling_calibration_helpers_cap_h
#define quantlib_interest_rate_modelling_calibration_helpers_cap_h

#include "ql/InterestRateModelling/calibrationhelper.hpp"
#include "ql/Instruments/capfloor.hpp"


namespace QuantLib {

    namespace InterestRateModelling {

        namespace CalibrationHelpers {

            class CapHelper : public CalibrationHelper {
              public:
                //Constructor for ATM cap
                CapHelper(
                    const Period& tenor,
                    const Handle<Indexes::Xibor>& index,
                    const RelinkableHandle<TermStructure>& termStructure);

                virtual ~CapHelper() {}

                virtual double modelValue(const Handle<Model>& model);

                virtual double blackPrice(double volatility) const;

              private:
                RelinkableHandle<TermStructure> termStructure_;
                Handle<Instruments::SimpleSwap> swap_;
                Handle<Instruments::VanillaCap> cap_;
                Handle<Pricers::CapFloorPricingEngine> engine_;
            };
        }
    }
}

#endif
