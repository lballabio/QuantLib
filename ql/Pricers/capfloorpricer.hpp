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
/*! \file capfloorpricer.hpp
    \brief Cap and Floor pricer class

    \fullpath
    ql/Pricers/%capfloorpricer.hpp
*/

// $Id$

#ifndef quantlib_pricers_capfloor_pricer_h
#define quantlib_pricers_capfloor_pricer_h

#include <ql/Instruments/capfloor.hpp>

namespace QuantLib {

    namespace Pricers {

        class DiscretizedCapFloor : public DiscretizedAsset {
          public:
            DiscretizedCapFloor(const Handle<NumericalMethod>& method,
                                const Instruments::CapFloorParameters& params)
            : DiscretizedAsset(method), parameters_(params) {}

            void reset(Size size) {
                values_ = Array(size, 0.0);
                applyCondition();
            }

            virtual void applyCondition();

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
        class CapFloorPricer : public OptionPricingEngine,
                               public Patterns::Observer,
                               public Patterns::Observable {
          public:
            CapFloorPricer() {}
            CapFloorPricer(const Handle<ModelType>& model) 
            : model_(model) {
                registerWith(model_);
            }
            Arguments* parameters() { return &parameters_; }
            const Results* results() const { return &results_; }
            void validateParameters() const { parameters_.validate(); }

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
            Instruments::CapFloorParameters parameters_;
            mutable Instruments::CapFloorResults results_;
            Handle<ModelType> model_;
        };

    }

}

#endif
