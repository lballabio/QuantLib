
/*
 Copyright (C) 2000, 2001, 2002 Sadruddin Rejeb

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software developed by the QuantLib Group; you can
 redistribute it and/or modify it under the terms of the QuantLib License;
 either version 1.0, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 QuantLib License for more details.

 You should have received a copy of the QuantLib License along with this
 program; if not, please email ferdinando@ametrano.net

 The QuantLib License is also available at http://quantlib.org/license.html
 The members of the QuantLib Group are listed in the QuantLib License
*/
/*! \file capfloor.hpp
    \brief Cap and Floor class

    \fullpath
    ql/Instruments/%capfloor.hpp
*/

// $Id$

#ifndef quantlib_instruments_capfloor_h
#define quantlib_instruments_capfloor_h

#include "ql/instrument.hpp"
#include "ql/Instruments/simpleswap.hpp"
#include "ql/InterestRateModelling/model.hpp"

#include <list>
#include <vector>

namespace QuantLib {

    namespace Instruments {

        class VanillaCapFloor : public Option {
          public:
            enum Type { Cap, Floor };
            VanillaCapFloor(Type type,
                const SimpleSwap& swap,
                const std::vector<Rate>& exerciseRates,
                const RelinkableHandle<TermStructure>& termStructure,
                const Handle<OptionPricingEngine>& engine)
            : Option(engine), type_(type), swap_(swap), 
              exerciseRates_(exerciseRates), termStructure_(termStructure) {}
            virtual ~VanillaCapFloor() {}
          protected:
            void setupEngine() const;
          private:
            Type type_;
            const SimpleSwap& swap_;
            std::vector<Rate> exerciseRates_;
            RelinkableHandle<TermStructure> termStructure_;
        };

        class VanillaCap : public VanillaCapFloor {
          public:
            VanillaCap(const SimpleSwap& swap,
                const std::vector<Rate>& exerciseRates,
                const RelinkableHandle<TermStructure>& termStructure,
                const Handle<OptionPricingEngine>& engine)
            : VanillaCapFloor(Cap, swap, exerciseRates, termStructure, engine) {}
        };

        class VanillaFloor : public VanillaCapFloor {
          public:
            VanillaFloor(const SimpleSwap& swap,
                  const std::vector<Rate>& exerciseRates,
                  const RelinkableHandle<TermStructure>& termStructure,  
                  const Handle<OptionPricingEngine>& engine)
            : VanillaCapFloor(Floor, swap, exerciseRates, termStructure, engine) 
            {}
        };

        //! parameters for cap/floor calculation
        class CapFloorParameters : public virtual Arguments {
          public:
            CapFloorParameters() : startTimes(0),
                                   endTimes(0),
                                   exerciseRates(0),
                                   nominals(0) {}
            VanillaCapFloor::Type type;
            std::vector<Time> startTimes;
            std::vector<Time> endTimes;
            std::vector<Rate> exerciseRates;
            std::vector<double> nominals;
        };

        //! %results from cap/floor calculation
        class CapFloorResults : public OptionValue {};

    }

    namespace Pricers {

        //! base class for cap/floor pricing engines
        /*! Derived engines only need to implement the <tt>calculate()</tt>
            method
        */
        class CapFloorPricingEngine : public OptionPricingEngine {
          public:
            Arguments* parameters();
            void validateParameters() const;
            const Results* results() const;
            void setModel(const Handle<InterestRateModelling::Model>& model) {
                model_ = model;
            }
          protected:
            Instruments::CapFloorParameters parameters_;
            mutable Instruments::CapFloorResults results_;
            Handle<InterestRateModelling::Model> model_;
        };

    }

}

#endif
