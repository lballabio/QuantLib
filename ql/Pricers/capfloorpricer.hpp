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
                                const Instruments::CapFloorArguments& params)
            : DiscretizedAsset(method), arguments_(params) {}

            void reset(Size size) {
                values_ = Array(size, 0.0);
                adjustValues();
            }

            virtual void adjustValues();

            void addTimes(std::list<Time>& times) const {
                for (Size i=0; i<arguments_.startTimes.size(); i++) {
                    times.push_back(arguments_.startTimes[i]);
                    times.push_back(arguments_.endTimes[i]);
                }
            }

          private:
            Instruments::CapFloorArguments arguments_;
        };

        //! base class for cap/floor pricing engines
        /*! Derived engines only need to implement the <tt>calculate()</tt>
            method
        */
        template<class ModelType>
        class CapFloorPricer : public PricingEngine,
                               public Patterns::Observer,
                               public Patterns::Observable {
          public:
            CapFloorPricer() {}
            CapFloorPricer(const Handle<ModelType>& model) 
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
            Instruments::CapFloorArguments arguments_;
            mutable Instruments::CapFloorResults results_;
            Handle<ModelType> model_;
        };

    }

}

#endif
