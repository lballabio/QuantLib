/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb
 Copyright (C) 2015 Peter Caspers

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

/*! \file calibrationhelper.hpp
    \brief Calibration helper class
*/

#ifndef quantlib_interest_rate_modelling_calibration_helper_h
#define quantlib_interest_rate_modelling_calibration_helper_h

#include <ql/patterns/lazyobject.hpp>
#include <ql/quote.hpp>
#include <ql/termstructures/volatility/volatilitytype.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <list>
#include <utility>

namespace QuantLib {

    class PricingEngine;

    //! abstract base class for calibration helpers
    class CalibrationHelper {
      public:
        virtual ~CalibrationHelper() = default;
        //! returns the error resulting from the model valuation
        virtual Real calibrationError() = 0;
    };

    QL_DEPRECATED_DISABLE_WARNING
    //! liquid Black76 market instrument used during calibration
    class BlackCalibrationHelper : public LazyObject, public CalibrationHelper {
      public:
        enum CalibrationErrorType {
                            RelativePriceError, PriceError, ImpliedVolError};

        BlackCalibrationHelper(Handle<Quote> volatility,
                               CalibrationErrorType calibrationErrorType = RelativePriceError,
                               const VolatilityType type = ShiftedLognormal,
                               const Real shift = 0.0)
        : volatility_(std::move(volatility)), volatilityType_(type), shift_(shift),
          calibrationErrorType_(calibrationErrorType) {
            registerWith(volatility_);
        }

        void performCalculations() const override {
            marketValue_ = blackPrice(volatility_->value());
        }

        //! returns the volatility Handle
        Handle<Quote> volatility() const { return volatility_; }

        //! returns the volatility type
        VolatilityType volatilityType() const { return volatilityType_; }

        //! returns the actual price of the instrument (from volatility)
        Real marketValue() const { calculate(); return marketValue_; }

        //! returns the price of the instrument according to the model
        virtual Real modelValue() const = 0;

        //! returns the error resulting from the model valuation
        Real calibrationError() override;

        virtual void addTimesTo(std::list<Time>& times) const = 0;

        //! Black volatility implied by the model
        Volatility impliedVolatility(Real targetValue,
                                     Real accuracy,
                                     Size maxEvaluations,
                                     Volatility minVol,
                                     Volatility maxVol) const;

        //! Black or Bachelier price given a volatility
        virtual Real blackPrice(Volatility volatility) const = 0;

        void setPricingEngine(const ext::shared_ptr<PricingEngine>& engine) {
            engine_ = engine;
        }

      protected:
        mutable Real marketValue_;
        Handle<Quote> volatility_;
        /*! \deprecated Don't use this data member.  It you're
                        inheriting from BlackCalibrationHelper, move
                        `termStructure_` to your derived class.
                        Deprecated in version 1.24.
        */
        [[deprecated]] Handle<YieldTermStructure> termStructure_;
        ext::shared_ptr<PricingEngine> engine_;
        const VolatilityType volatilityType_;
        const Real shift_;

      private:
        class ImpliedVolatilityHelper;
        const CalibrationErrorType calibrationErrorType_;
    };
    QL_DEPRECATED_ENABLE_WARNING

}


#endif
