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

#include "ql/instrument.hpp"
#include "ql/Instruments/simpleswap.hpp"
#include "ql/InterestRateModelling/model.hpp"

#include <list>

namespace QuantLib {

    namespace Instruments {

        class VanillaCapFloor : public Option {
          public:
            enum Type { Cap, Floor };
            VanillaCapFloor(Type type,
                const Handle<SimpleSwap>& swap,
                const std::vector<Rate>& exerciseRates,
                const RelinkableHandle<TermStructure>& termStructure,
                const Handle<OptionPricingEngine>& engine)
            : Option(engine), type_(type), swap_(swap),
              exerciseRates_(exerciseRates), termStructure_(termStructure) {}
            virtual ~VanillaCapFloor() {}
          protected:
            void performCalculations() const;
            void setupEngine() const;
          private:
            Type type_;
            Handle<SimpleSwap> swap_;
            std::vector<Rate> exerciseRates_;
            RelinkableHandle<TermStructure> termStructure_;
        };

        class VanillaCap : public VanillaCapFloor {
          public:
            VanillaCap(const Handle<SimpleSwap>& swap,
                const std::vector<Rate>& exerciseRates,
                const RelinkableHandle<TermStructure>& termStructure,
                const Handle<OptionPricingEngine>& engine)
            : VanillaCapFloor(Cap, swap, exerciseRates, termStructure, engine)
            {}
        };

        class VanillaFloor : public VanillaCapFloor {
          public:
            VanillaFloor(const Handle<SimpleSwap>& swap,
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
