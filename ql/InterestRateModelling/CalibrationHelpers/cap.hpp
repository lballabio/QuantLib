
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

/*! \file cap.hpp
    \brief Cap calibration helper

    \fullpath
    ql/InterestRateModelling/CalibrationHelpers/%cap.hpp
*/

// $Id$

#ifndef quantlib_interest_rate_modelling_calibration_helpers_cap_h
#define quantlib_interest_rate_modelling_calibration_helpers_cap_h

#include "ql/instrument.hpp"
#include "ql/Instruments/capfloor.hpp"
#include "ql/InterestRateModelling/onefactormodel.hpp"
#include <vector>

namespace QuantLib {

    namespace InterestRateModelling {

        namespace CalibrationHelpers {

            class Cap : public CalibrationHelper {
              public:
                Cap(
                    const Period& wait,
                    const Period& tenor,
                    const Handle<Indexes::Xibor>& index,
                    Rate exerciseRate,
                    const RelinkableHandle<TermStructure>& termStructure);
                virtual ~Cap() {}
                virtual double value(const Handle<Model>& model);
                virtual double blackPrice(double volatility) const;
              private:
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
