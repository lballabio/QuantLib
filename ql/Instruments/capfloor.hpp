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
/*! \file capfloor.hpp
    \brief Cap and Floor class

    \fullpath
    ql/Instruments/%capfloor.hpp
*/

// $Id$

#ifndef quantlib_instruments_capfloor_h
#define quantlib_instruments_capfloor_h

#include "ql/dataformatters.hpp"
#include <ql/instrument.hpp>
#include <ql/numericalmethod.hpp>
#include <ql/CashFlows/cashflowvectors.hpp>
#include <ql/InterestRateModelling/model.hpp>

namespace QuantLib {

    namespace Instruments {

        class VanillaCapFloor : public Option {
          public:
            enum Type { Cap, Floor };
            VanillaCapFloor(Type type,
                const CashFlows::FloatingRateCouponVector& floatingLeg,
                const std::vector<Rate>& exerciseRates,
                const RelinkableHandle<TermStructure>& termStructure,
                const Handle<OptionPricingEngine>& engine)
            : Option(engine), type_(type), floatingLeg_(floatingLeg),
              exerciseRates_(exerciseRates), termStructure_(termStructure) {}
            virtual ~VanillaCapFloor() {}
          protected:
            void performCalculations() const;
            void setupEngine() const;
          private:
            Type type_;
            CashFlows::FloatingRateCouponVector floatingLeg_;
            std::vector<Rate> exerciseRates_;
            RelinkableHandle<TermStructure> termStructure_;
        };

        class VanillaCap : public VanillaCapFloor {
          public:
            VanillaCap(
                const CashFlows::FloatingRateCouponVector& floatingLeg,
                const std::vector<Rate>& exerciseRates,
                const RelinkableHandle<TermStructure>& termStructure,
                const Handle<OptionPricingEngine>& engine)
            : VanillaCapFloor(Cap, floatingLeg, exerciseRates, 
                              termStructure, engine)
            {}
        };

        class VanillaFloor : public VanillaCapFloor {
          public:
            VanillaFloor(
                const CashFlows::FloatingRateCouponVector& floatingLeg,
                const std::vector<Rate>& exerciseRates,
                const RelinkableHandle<TermStructure>& termStructure,
                const Handle<OptionPricingEngine>& engine)
            : VanillaCapFloor(Floor, floatingLeg, exerciseRates, 
                              termStructure, engine)
            {}
        };
/*
        class Collar : public VanillaCapFloor {
          public:
            VanillaFloor(const CashFlows::FloatingRateCouponVector& swap,
                const std::vector<Rate>& exerciseRates,
                const RelinkableHandle<TermStructure>& termStructure,
                const Handle<OptionPricingEngine>& engine)
            : VanillaCapFloor(Collar, swap, exerciseRates, termStructure, engine)
            {}
        };
*/
        //! parameters for cap/floor calculation
        class CapFloorParameters : public virtual Arguments {
          public:
            CapFloorParameters() : type(VanillaCapFloor::Type(-1)),
                                   startTimes(0),
                                   endTimes(0),
                                   exerciseRates(0),
                                   nominals(0) {}
            VanillaCapFloor::Type type;
            std::vector<Time> startTimes;
            std::vector<Time> endTimes;
            std::vector<Rate> exerciseRates;
            std::vector<double> nominals;
            void validate() const {
                QL_REQUIRE(
                    endTimes.size() == startTimes.size(),
                    "Invalid pricing parameters: size of startTimes(" +
                    IntegerFormatter::toString(startTimes.size()) +
                    ") different from that of endTimes(" +
                    IntegerFormatter::toString(endTimes.size()) +
                    ")");
                QL_REQUIRE(exerciseRates.size()==startTimes.size(),
                    "Invalid pricing parameters");
            }

        };

        //! %results from cap/floor calculation
        class CapFloorResults : public OptionValue {};

    }

    namespace Pricers {

        class NumericalCapFloor : public NumericalDerivative {
          public:
            NumericalCapFloor(const Handle<NumericalMethod>& method,
                              const Instruments::CapFloorParameters& params)
            : NumericalDerivative(method), parameters_(params) {}

            void reset(Size size) {
                values_ = Array(size, 0.0);
                applyCondition();
            }

            virtual void applyCondition() {
                for (Size i=0; i<parameters_.startTimes.size(); i++) {
                    if (time_ == parameters_.startTimes[i]) {
                        Time end = parameters_.endTimes[i];
                        Handle<NumericalDerivative> bond(new 
                            NumericalDiscountBond(method()));
                        method()->initialize(bond, end);
                        method()->rollback(bond,time_);
                        double accrual = 
                            1.0 + parameters_.exerciseRates[i]*(end - time_);
                        double factor = parameters_.nominals[i]*accrual;
                        double strike = 1.0/accrual;
                        switch(parameters_.type) {
                          case Instruments::VanillaCapFloor::Floor:
                            for (i=0; i<values_.size(); i++)
                                values_[i] += factor*
                                    QL_MAX(bond->values()[i] - strike, 0.0);
                            break;
                          case Instruments::VanillaCapFloor::Call:
                            for (i=0; i<values_.size(); i++)
                                values_[i] += factor*
                                    QL_MAX(strike - bond->values()[i], 0.0);
                            break;
                        }
                    }
                }
            }
            void addTimes(std::list<Time>& times) const {
                for (Size i=0; i<parameters_.startTimes.size(); i++) {
                    times.push_back(parameters_.startTimes[i]);
                    times.push_back(parameters_.endTimes[i]);
                }
            }

          private:
            Instruments::CapFloorParameters parameters_;
        };

        //! base class for cap/floor pricing engines
        /*! Derived engines only need to implement the <tt>calculate()</tt>
            method
        */
        template<class ModelType>
        class CapFloorPricingEngine : public OptionPricingEngine,
                                      public Patterns::Observer,
                                      public Patterns::Observable {
          public:
            CapFloorPricingEngine() {}
            CapFloorPricingEngine(const Handle<ModelType>& model) 
            : model_(model) {
                registerWith(model_);
            }
            ~CapFloorPricingEngine() {
                unregisterWith(model_);
            }
            Arguments* parameters() { return &parameters_; }
            const Results* results() const { return &results_; }
            void validateParameters() const { parameters_.validate(); }

            void setModel(const Handle<ModelType>& model) {
                unregisterWith(model_);
                model_ = model;
                QL_REQUIRE(!model_.isNull(), "Not an adequate model!");
                registerWith(model_);
            }
            void update() {
                notifyObservers();
            }
          protected:
            Instruments::CapFloorParameters parameters_;
            mutable Instruments::CapFloorResults results_;
            Handle<ModelType> model_;
        };

    }

}

#endif
