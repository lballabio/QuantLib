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
/*! \file swaptionpricer.hpp
    \brief Swaption pricer class

    \fullpath
    ql/Pricers/%swaptionpricer.hpp
*/

// $Id$

#ifndef quantlib_pricers_swaption_pricer_h
#define quantlib_pricers_swaption_pricer_h

#include <ql/numericalmethod.hpp>
#include <ql/Instruments/swaption.hpp>

namespace QuantLib {

    namespace Pricers {

        class DiscretizedSwap : public DiscretizedAsset {
          public:
            DiscretizedSwap(const Handle<NumericalMethod>& method, 
                            const Instruments::SwaptionArguments& params)
            : DiscretizedAsset(method), arguments_(params) {}

            void reset(Size size) {
                values_ = Array(size, 0.0);
                adjustValues();
            }

            void adjustValues();

            void addTimes(std::list<Time>& times) const {
                Size i;
                for (i=0; i<arguments_.fixedPayTimes.size(); i++)
                    times.push_back(arguments_.fixedPayTimes[i]);
                for (i=0; i<arguments_.floatingResetTimes.size(); i++)
                    times.push_back(arguments_.floatingResetTimes[i]);
                for (i=0; i<arguments_.floatingPayTimes.size(); i++)
                    times.push_back(arguments_.floatingPayTimes[i]);
            }

          private:
            Instruments::SwaptionArguments arguments_;
        };

        class DiscretizedSwaption : public DiscretizedAsset {
          public:
            DiscretizedSwaption(
                const Handle<NumericalMethod>& method,
                const Instruments::SwaptionArguments& params)
            : DiscretizedAsset(method), arguments_(params), 
              swap_(new DiscretizedSwap(method, params)) {
                Time lastFixedPay = arguments_.fixedPayTimes.back();
                Time lastFloatPay = arguments_.floatingPayTimes.back();
                Time start = QL_MAX(lastFixedPay, lastFloatPay);
                method->initialize(swap_, start);
            }

            void reset(Size size) {
                values_ = Array(size, 0.0);
                adjustValues();
            }

            virtual void adjustValues();

            void addTimes(std::list<Time>& times) const {
                swap_->addTimes(times);
                for (Size i=0; i<arguments_.exerciseTimes.size(); i++)
                    times.push_back(arguments_.exerciseTimes[i]);
            }
          private:
            void applySpecificCondition() {
                for (Size i=0; i<values_.size(); i++)
                    values_[i] = QL_MAX(swap_->values()[i], values_[i]);
            }

            Instruments::SwaptionArguments arguments_;
            Handle<DiscretizedSwap> swap_;
        };

        //! base class for swaption pricing engines
        /*! Derived engines only need to implement the <tt>calculate()</tt>
            method
        */
        template<class ModelType>
        class SwaptionPricer : public PricingEngine,
                               public Patterns::Observer,
                               public Patterns::Observable {
          public:
            SwaptionPricer() {}
            SwaptionPricer(const Handle<ModelType>& model) 
            : model_(model) {
                registerWith(model_);
            }
            Arguments* arguments() { return &arguments_; }
            const Results* results() const { return &results_; }
            void validateArguments() const { arguments_.validate(); }

            void setModel(const Handle<ModelType>& model) {
                unregisterWith(model_);
                model_ = model;
                QL_REQUIRE(!model_.isNull(), "Not an adequate model!");
                registerWith(model_);
                update();
            }
            virtual void update() {
                notifyObservers();
            }
          protected:
            Instruments::SwaptionArguments arguments_;
            mutable Instruments::SwaptionResults results_;
            Handle<ModelType> model_;
        };

        template<class ModelType>
        class SpecificSwaptionPricer : public SwaptionPricer<ModelType> {
          public:
            SpecificSwaptionPricer(const Handle<ModelType>& model) 
            : SwaptionPricer<ModelType>(model) {}

            void calculate() const { 
                results_.value = model_->swaption(arguments_); 
            }
        };

    }

}


#endif

