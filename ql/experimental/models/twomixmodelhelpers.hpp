/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Chris Kenyon

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file twomixmodelhelpers.hpp
    \brief Calibration helpers for two-components mixture models
*/

#ifndef quantlib_two_mix_model_helpers_hpp
#define quantlib_two_mix_model_helpers_hpp

#include <ql/experimental/models/twomixmodel.hpp>
#include <ql/experimental/inflation/inflationcapfloor.hpp>
#include <ql/experimental/inflation/yoyoptionletvolatilitystructures.hpp>
#include <ql/instruments/capfloor.hpp>
#include <ql/pricingengines/capfloor/blackcapfloorengine.hpp>
#include <ql/termstructures/volatility/optionlet/constantoptionletvol.hpp>

namespace QuantLib {

    /*! This makes a capLET or a floorLET because the calibration is
        done one time-segment at a time.  Basically just calls
        MakeYoYInflationCapFloor.
    */
    class TwoMixYoYModelHelper : public CalibrationHelper {
      public:
        TwoMixYoYModelHelper(const Handle<Quote> &volatility,
                             boost::shared_ptr<PricingEngine> marketValueEngine,
                             Real notional,
                             YoYInflationCapFloor::Type capFloorType,
                             Period &lag, Natural fixingDays,
                             const boost::shared_ptr<YoYInflationIndex>& index,
                             Rate strike, Size n, Real normalizingFactor = 1.0);

        //! not used
        void addTimesTo(std::list<Time> &) const {}

        Real modelValue() const;

        /*! returns Bachelier/Black/DD price depending on what
            marketValueEngine_ is set to from initial constructor call
        */
        Real blackPrice(Volatility volatility) const;

        /*! The default version gives percentage errors but this is
            bad for very small absolute market values.  This version
            adds a normalizing factor to top and bottom.

            error = (norm + true - model) / (norm + true)

            The normalizing factor is one of the inputs to the constructor.
        */
        Real calibrationError() const;

        YoYInflationCapFloor::Type type() const {return capFloorType_;}

      private:
        const Handle<Quote> volatility_;

        Real notional_;
        YoYInflationCapFloor::Type capFloorType_;
        Period lag_;
        Natural fixingDays_;
        boost::shared_ptr<YoYInflationIndex> index_;
        Rate strike_;
        Size n_;
        mutable boost::shared_ptr<YoYInflationCapFloor> capFloor_;

        // this gets used for converting market vols into prices
        mutable Handle<ConstantYoYOptionletVolatility> marketVol_;
        boost::shared_ptr<PricingEngine> marketValueEngine_;
        Real normalizingFactor_;
    };



    /*! This makes a capLET or a floorLET because the calibration is
        done one time-segment at a time.  Basically just calls
        MakeCapFloor.
    */
    class TwoMixSLMUPModelHelper : public CalibrationHelper {
      public:
        TwoMixSLMUPModelHelper(const Handle<Quote> &volatility,
                               Real notional,
                               CapFloor::Type capFloorType,
                               const boost::shared_ptr<IborIndex>& index,
                               Rate strike, Size n,
                               Real normalizingFactor = 1.0);

        //! not used
        void addTimesTo(std::list<Time> &) const {}

        Real modelValue() const;

        Real blackPrice(Volatility volatility) const;

        /*! The default version gives percentage errors but this is
            bad for very small absolute market values.  This version
            adds a normalizing factor to top and bottom.

            error = (norm + true - model) / (norm + true)

            The normalizing factor is one of the inputs to the constructor.
        */
        Real calibrationError() const;

        CapFloor::Type type() const {return capFloorType_;}

      private:
        const Handle<Quote> volatility_;
        Real notional_;
        CapFloor::Type capFloorType_;
        Natural fixingDays_;
        boost::shared_ptr<IborIndex> index_;
        Rate strike_;
        Size n_;
        mutable boost::shared_ptr<CapFloor> capFloor_;

        //! this gets used for converting market vols into prices
        mutable Handle<ConstantOptionletVolatility> marketVol_;
        boost::shared_ptr<BlackCapFloorEngine> marketValueEngine_;
        Real normalizingFactor_;
    };

}

#endif

