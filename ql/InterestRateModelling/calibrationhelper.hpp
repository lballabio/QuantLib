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
/*! \file calibrationhelper.hpp
    \brief Calibration helper class

    \fullpath
    ql/InterestRateModelling/%calibrationhelper.hpp
*/

// $Id$

#ifndef quantlib_interest_rate_modelling_calibration_helper_h
#define quantlib_interest_rate_modelling_calibration_helper_h

#include <ql/grid.hpp>
#include <ql/InterestRateModelling/blackmodel.hpp>
#include <ql/InterestRateModelling/model.hpp>
#include <ql/Lattices/tree.hpp>

#include <list>

namespace QuantLib {

    namespace InterestRateModelling {

        class CalibrationHelper : public Patterns::Observer, 
                                  public Patterns::Observable {
          public:
            CalibrationHelper(
                const RelinkableHandle<MarketElement>& volatility)
            : volatility_(volatility), blackModel_(volatility_) {
                registerWith(volatility_);
            }
            void update() { 
                marketValue_ = blackPrice(volatility_->value());
                notifyObservers(); 
            }

            double marketValue() { return marketValue_; }

            virtual void addTimes(std::list<Time>& times) const = 0;

            virtual void setAnalyticalPricingEngine() = 0;
            virtual void setNumericalPricingEngine(
                const Handle<Lattices::Tree>& tree) = 0;
            virtual void setNumericalPricingEngine(Size timeSteps) = 0;

            virtual void setModel(const Handle<Model>& model) = 0;
            virtual double modelValue() = 0;

            virtual double calibrationError() {
                return QL_FABS(marketValue() - modelValue())/marketValue();
            }

            double impliedVolatility(double targetValue,
                                     double accuracy,
                                     Size maxEvaluations,
                                     double minVol,
                                     double maxVol) const;

            virtual double blackPrice(double volatility) const = 0;

            void setPricingEngine(const Handle<OptionPricingEngine>& engine) {
                engine_ = engine;
            }

          protected:
            double marketValue_;
            RelinkableHandle<MarketElement> volatility_;
            Handle<BlackModel> blackModel_;
            Handle<OptionPricingEngine> engine_;

          private:
            class ImpliedVolatilityHelper;
        };

        class CalibrationSet : public std::vector<Handle<CalibrationHelper> > {
          public:

            void setPricingEngine(const Handle<OptionPricingEngine>& engine) {
                for (Size i=0; i<size(); i++)
                    (*this)[i]->setPricingEngine(engine);
            }

        };

    }

}
#endif
