
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

/*! \file capfloor.hpp
    \brief Cap and Floor class

    \fullpath
    ql/Instruments/%capfloor.hpp
*/

// $Id$

#ifndef quantlib_instruments_capfloor_h
#define quantlib_instruments_capfloor_h

#include <ql/dataformatters.hpp>
#include <ql/numericalmethod.hpp>
#include <ql/option.hpp>
#include <ql/CashFlows/cashflowvectors.hpp>

namespace QuantLib {

    namespace Instruments {

        //! Base class for cap-like instruments
        class VanillaCapFloor : public Option {
          public:
            enum Type { Cap, Floor, Collar };
            VanillaCapFloor(Type type,
                const std::vector<Handle<CashFlow> >& floatingLeg,
                const std::vector<Rate>& capRates,
                const std::vector<Rate>& floorRates,
                const RelinkableHandle<TermStructure>& termStructure,
                const Handle<PricingEngine>& engine)
            : Option(engine), type_(type), floatingLeg_(floatingLeg),
              capRates_(capRates), floorRates_(floorRates),
              termStructure_(termStructure) {
                std::vector<Handle<CashFlow> >::const_iterator i;
                for (i = floatingLeg_.begin(); i != floatingLeg_.end(); ++i)
                    registerWith(*i);
                registerWith(termStructure);
                registerWith(engine);
            }
          protected:
            void performCalculations() const;
            void setupEngine() const;
          private:
            Type type_;
            std::vector<Handle<CashFlow> > floatingLeg_;
            std::vector<Rate> capRates_;
            std::vector<Rate> floorRates_;
            RelinkableHandle<TermStructure> termStructure_;
        };

        //! Concrete cap class
        class VanillaCap : public VanillaCapFloor {
          public:
            VanillaCap(
                const std::vector<Handle<CashFlow> >& floatingLeg,
                const std::vector<Rate>& exerciseRates,
                const RelinkableHandle<TermStructure>& termStructure,
                const Handle<PricingEngine>& engine)
            : VanillaCapFloor(Cap, floatingLeg, 
                              exerciseRates, std::vector<Rate>(1, 0.0),
                              termStructure, engine)
            {}
        };

        //! Concrete floor class
        class VanillaFloor : public VanillaCapFloor {
          public:
            VanillaFloor(
                const std::vector<Handle<CashFlow> >& floatingLeg,
                const std::vector<Rate>& exerciseRates,
                const RelinkableHandle<TermStructure>& termStructure,
                const Handle<PricingEngine>& engine)
            : VanillaCapFloor(Floor, floatingLeg, 
                              std::vector<Rate>(1, 0.0), exerciseRates,
                              termStructure, engine)
            {}
        };

        //! Concrete cap class
        class VanillaCollar : public VanillaCapFloor {
          public:
            VanillaCollar(
                const std::vector<Handle<CashFlow> >& floatingLeg,
                const std::vector<Rate>& capRates,
                const std::vector<Rate>& floorRates,
                const RelinkableHandle<TermStructure>& termStructure,
                const Handle<PricingEngine>& engine)
            : VanillaCapFloor(Collar, floatingLeg, capRates, floorRates,
                              termStructure, engine)
            {}
        };

        //! arguments for cap/floor calculation
        class CapFloorArguments : public virtual Arguments {
          public:
            CapFloorArguments() : type(VanillaCapFloor::Type(-1)),
                                   startTimes(0),
                                   endTimes(0),
                                   accrualTimes(0),
                                   capRates(0),
                                   floorRates(0),
                                   forwards(0),
                                   nominals(0) {}
            VanillaCapFloor::Type type;
            std::vector<Time> startTimes;
            std::vector<Time> endTimes;
            std::vector<Time> accrualTimes;
            std::vector<Rate> capRates;
            std::vector<Rate> floorRates;
            std::vector<Rate> forwards;
            std::vector<double> nominals;
            void validate() const;
        };

        //! %results from cap/floor calculation
        class CapFloorResults : public OptionValue {};

    }

}

#endif
