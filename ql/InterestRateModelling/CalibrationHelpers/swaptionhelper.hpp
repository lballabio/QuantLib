
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

/*! \file swaptionhelper.hpp
    \brief Swaption calibration helper

    \fullpath
    ql/InterestRateModelling/CalibrationHelpers/%swaptionhelper.hpp
*/

// $Id$

#ifndef quantlib_interest_rate_modelling_calibration_helpers_swaption_h
#define quantlib_interest_rate_modelling_calibration_helpers_swaption_h

#include <ql/InterestRateModelling/calibrationhelper.hpp>
#include <ql/Instruments/simpleswap.hpp>
#include <ql/Instruments/swaption.hpp>


namespace QuantLib {

    namespace InterestRateModelling {

        namespace CalibrationHelpers {

            class SwaptionHelper : public CalibrationHelper {
              public:
                //!Constructor for ATM swaption
                SwaptionHelper(
                    const Period& maturity,
                    const Period& tenor,
                    const Handle<Indexes::Xibor>& index,
                    const RelinkableHandle<TermStructure>& termStructure);

                virtual ~SwaptionHelper() {}

                virtual double modelValue(const Handle<Model>& model);

                virtual double blackPrice(double volatility) const;

              private:
                Rate exerciseRate_;
                RelinkableHandle<TermStructure> termStructure_;
                Handle<Instruments::SimpleSwap> swap_;
                Handle<Instruments::Swaption> swaption_;
                Handle<Pricers::SwaptionPricingEngine> engine_;
            };
        }
    }
}

#endif
