
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.org/
 *
 * QuantLib is free software and you are allowed to use, copy, modify, merge,
 * publish, distribute, and/or sell copies of it under the conditions stated
 * in the QuantLib License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
 *
 * You should have received a copy of the license along with this file;
 * if not, please email quantlib-users@lists.sourceforge.net
 * The license is also available at http://quantlib.org/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.org/group.html
*/

/*! \file caphelper.hpp
    \brief CapHelper calibration helper

    \fullpath
    ql/InterestRateModelling/CalibrationHelpers/%caphelper.hpp
*/

// $Id$

#ifndef quantlib_interest_rate_modelling_calibration_helpers_cap_h
#define quantlib_interest_rate_modelling_calibration_helpers_cap_h

#include "ql/Instruments/capfloor.hpp"
#include "ql/InterestRateModelling/model.hpp"

namespace QuantLib {

    namespace InterestRateModelling {

        namespace CalibrationHelpers {

            class CapHelper : public CalibrationHelper {
              public:
                //Constructor for any cap
                CapHelper(
                    const Period& wait,
                    const Period& tenor,
                    const Handle<Indexes::Xibor>& index,
                    Rate exerciseRate,
                    const RelinkableHandle<TermStructure>& termStructure);

                //Constructor for ATM cap
                CapHelper(
                    const Period& tenor,
                    const Handle<Indexes::Xibor>& index,
                    const RelinkableHandle<TermStructure>& termStructure);

                virtual ~CapHelper() {}

                virtual double modelValue(const Handle<Model>& model);
                virtual double marketValue() { return marketValue_; }

                void setVolatility(double volatility) {
                    volatility_ = volatility;
                    marketValue_ = blackPrice(volatility);
                }

                void setMarketValue(double value) {
                    volatility_ = Null<double>();
                    marketValue_ = value;
                }

              private:
                virtual double blackPrice(double volatility) const;

                double volatility_;
                double marketValue_;
                Rate exerciseRate_;
                RelinkableHandle<TermStructure> termStructure_;
                Handle<Instruments::SimpleSwap> swap_;
                Handle<Instruments::EuropeanCap> cap_;
                unsigned int nbOfPeriods_;
                std::vector<Time> startTimes_;
                std::vector<Time> endTimes_;
            };
        }
    }
}

#endif
