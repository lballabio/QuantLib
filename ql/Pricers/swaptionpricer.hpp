
/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb

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
*/

#ifndef quantlib_pricers_swaption_pricer_h
#define quantlib_pricers_swaption_pricer_h

#include <ql/Instruments/swaption.hpp>
#include <ql/PricingEngines/genericengine.hpp>

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
                Time t;
                Size i;
                for (i=0; i<arguments_.fixedPayTimes.size(); i++) {
                    t = arguments_.fixedPayTimes[i];
                    if (t >= 0.0)
                        times.push_back(t);
                }
                for (i=0; i<arguments_.floatingResetTimes.size(); i++) {
                    t = arguments_.floatingResetTimes[i];
                    if (t >= 0.0)
                        times.push_back(t);
                }
                for (i=0; i<arguments_.floatingPayTimes.size(); i++) {
                    t = arguments_.floatingPayTimes[i];
                    if (t >= 0.0)
                        times.push_back(t);
                }
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
                method->initialize(swap_,start);
            }

            void reset(Size size) {
                values_ = Array(size, 0.0);
                adjustValues();
            }

            virtual void adjustValues();

            void addTimes(std::list<Time>& times) const {
                swap_->addTimes(times);
                Time t;
                for (Size i=0; i<arguments_.exerciseTimes.size(); i++) {
                    t = arguments_.exerciseTimes[i];
                    if (t >= 0.0)
                        times.push_back(t);
                }
            }
          private:
            void applySpecificCondition() {
                for (Size i=0; i<values_.size(); i++)
                    values_[i] = QL_MAX(swap_->values()[i], values_[i]);
            }

            Instruments::SwaptionArguments arguments_;
            Handle<DiscretizedSwap> swap_;
        };

        template<class ModelType>
        class SpecificSwaptionPricer :
            public PricingEngines::GenericModelEngine<
                    ModelType,
                    Instruments::SwaptionArguments,
                    Instruments::SwaptionResults > {
          public:
            SpecificSwaptionPricer(const Handle<ModelType>& model)
            : PricingEngines::GenericModelEngine<
                    ModelType,
                    Instruments::SwaptionArguments,
                    Instruments::SwaptionResults >(model) {}

            void calculate() const {
                results_.value = model_->swaption(arguments_);
            }
        };

    }

}


#endif

