
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

/*! \file swaption.hpp
    \brief Swaption calibration helper

    \fullpath
    ql/Instruments/%swaption.hpp
*/

// $Id$

#ifndef quantlib_interest_rate_modelling_calibration_helpers_cap_h
#define quantlib_interest_rate_modelling_calibration_helpers_cap_h

#include <vector>
#include "ql/instrument.hpp"
#include "ql/Instruments/europeanswaption.hpp"
#include "ql/InterestRateModelling/onefactormodel.hpp"

namespace QuantLib {

    namespace InterestRateModelling {

        namespace CalibrationHelpers {

            using Instruments::SimpleSwap;
            using Instruments::EuropeanSwaption;

            class Swaption : public CalibrationHelper {
              public:
                Swaption( 
                    const Period& tenorPeriod,
                    const Period& swapPeriod,
                    const Handle<Indexes::Xibor>& index,
                    Rate exerciseRate,
                    const RelinkableHandle<TermStructure>& termStructure);

                virtual ~Swaption() {}
                virtual double value(const Handle<Model>& model);
                virtual double blackPrice(double volatility) const;
              private:
                Rate exerciseRate_;
                RelinkableHandle<TermStructure> termStructure_;
                Handle<SimpleSwap> swap_;
                Handle<EuropeanSwaption> swaption_;
                unsigned int nbOfPeriods_;
                std::vector<Time> startTimes_;
                std::vector<Time> endTimes_;
            };
        }
    }
}

#endif
