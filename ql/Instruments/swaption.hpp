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
/*! \file swaption.hpp
    \brief Swaption class

    \fullpath
    ql/Instruments/%swaption.hpp
*/

// $Id$

#ifndef quantlib_instruments_swaption_h
#define quantlib_instruments_swaption_h

#include <ql/exercise.hpp>
#include <ql/numericalmethod.hpp>
#include <ql/Instruments/simpleswap.hpp>
#include <ql/InterestRateModelling/model.hpp>

namespace QuantLib {

    namespace Instruments {

        class SwaptionParameters;

        //! Swaption class
        class Swaption : public Option {
          public:
            Swaption(const Handle<SimpleSwap>& swap,
                     const Exercise& exercise,
                     const RelinkableHandle<TermStructure>& termStructure,
                     const Handle<OptionPricingEngine>& engine);
            ~Swaption();
          protected:
            void performCalculations() const;
            void setupEngine() const;
          private:
            // parameters
            Handle<SimpleSwap> swap_;
            Exercise exercise_;
            const RelinkableHandle<TermStructure>& termStructure_;
            // helper class for implied volatility calculation
        };

        //! parameters for swaption calculation
        class SwaptionParameters : public virtual Arguments {
          public:
            SwaptionParameters() : payFixed(false),
                                   fixedPayTimes(0),
                                   fixedCoupons(0),
                                   floatingResetTimes(0),
                                   floatingPayTimes(0),
                                   nominals(0),
                                   exerciseType(Exercise::Type(-1)),
                                   exerciseTimes(0) {}
            bool payFixed;
            std::vector<Time> fixedPayTimes;
            std::vector<double> fixedCoupons;
            std::vector<Time> floatingResetTimes;
            std::vector<Time> floatingPayTimes;
            std::vector<double> nominals;
            Exercise::Type exerciseType;
            std::vector<Time> exerciseTimes;
        };

        //! %results from swaption calculation
        class SwaptionResults : public OptionValue {};

    }

    namespace Pricers {

        class NumericalSwap : public NumericalDerivative {
          public:
            NumericalSwap(const Handle<NumericalMethod>& method, 
                          const Instruments::SwaptionParameters& params)
            : NumericalDerivative(method), parameters_(params) {}

            void reset(Size size) {
                values_ = Array(size, 0.0);
                applyCondition();
            }

            virtual void applyCondition() {
                Size i;

                for (i=0; i<parameters_.fixedPayTimes.size(); i++) {
                    if (time_ == parameters_.fixedPayTimes[i]) {
                        if (parameters_.payFixed)
                            values_ -= parameters_.fixedCoupons[i];
                        else
                            values_ += parameters_.fixedCoupons[i];
                    }
                }

                for (i=0; i<parameters_.floatingResetTimes.size(); i++) {
                    if (time_ == parameters_.floatingResetTimes[i]) {
                        Handle<NumericalDerivative> bond(new 
                            NumericalDiscountBond(method()));
                        method()->initialize(bond, 
                            parameters_.floatingPayTimes[i]);
                        method()->rollback(bond,time_);

                        for (Size j=0; j<values_.size(); j++) {
                            double coupon = parameters_.nominals[i]*
                                (1.0 - bond->values()[j]);
                            if (parameters_.payFixed)
                                values_[j] += coupon;
                            else
                                values_[j] -= coupon;
                        }
                    }
                }
            }

            void addTimes(std::list<Time>& times) const {
                Size i;
                for (i=0; i<parameters_.fixedPayTimes.size(); i++)
                    times.push_back(parameters_.fixedPayTimes[i]);
                for (i=0; i<parameters_.floatingResetTimes.size(); i++)
                    times.push_back(parameters_.floatingResetTimes[i]);
                for (i=0; i<parameters_.floatingPayTimes.size(); i++)
                    times.push_back(parameters_.floatingPayTimes[i]);
            }

          private:
            Instruments::SwaptionParameters parameters_;
        };

        class NumericalSwaption : public NumericalDerivative {
          public:
            NumericalSwaption(
                const Handle<NumericalMethod>& method,
                const Instruments::SwaptionParameters& params)
            : NumericalDerivative(method), parameters_(params), 
              swap_(new NumericalSwap(method, params)) {
                Time lastFixedPay = parameters_.fixedPayTimes.back();
                Time lastFloatPay = parameters_.floatingPayTimes.back();
                Time start = QL_MAX(lastFixedPay, lastFloatPay);
                method->initialize(swap_, start);
            }

            void reset(Size size) {
                values_ = Array(size, 0.0);
                applyCondition();
            }

            virtual void applySpecificCondition() {
                for (Size i=0; i<values_.size(); i++)
                    values_[i] = QL_MAX(swap_->values()[i], values_[i]);
            }

            virtual void applyCondition() {
                method()->rollback(swap_, time());

                Size i;
                if (parameters_.exerciseType != Exercise::American) {
                    for (i=0; i<parameters_.exerciseTimes.size(); i++) {
                        if (time_ == parameters_.exerciseTimes[i]) {
                            applySpecificCondition();
                        }
                    }
                } else {
                    if (
                      (time_ >= parameters_.exerciseTimes[0]) &&
                      (time_ <= parameters_.exerciseTimes[1]))
                        applySpecificCondition();
                }
            }
            void addTimes(std::list<Time>& times) const {
                swap_->addTimes(times);
                for (Size i=0; i<parameters_.exerciseTimes.size(); i++)
                    times.push_back(parameters_.exerciseTimes[i]);
            }
          private:
            Instruments::SwaptionParameters parameters_;
            Handle<NumericalSwap> swap_;
        };

        //! base class for swaption pricing engines
        /*! Derived engines only need to implement the <tt>calculate()</tt>
            method
        */
        class SwaptionPricingEngine : public OptionPricingEngine {
          public:
            Arguments* parameters();
            void validateParameters() const;
            const Results* results() const;
            void setModel(const Handle<InterestRateModelling::Model>& model) {
                model_ = model;
            }
          protected:
            Instruments::SwaptionParameters parameters_;
            mutable Instruments::SwaptionResults results_;
            Handle<InterestRateModelling::Model> model_;
        };

    }

}


#endif

