
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.org/
 *
 * QuantLib is free software and you are allowed to use, copy, modify, merge,
 * publish, distribute, and/or sell copies of it under the conditions stated
 * in the QuantLib License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
 *
 * You should have received a copy of the license along with this file;
 * if not, please email quantlib-users@lists.sourceforge.net
 * The license is also available at http://quantlib.org/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.org/group.html
*/

/*! \file capfloor.hpp
    \brief European Cap and Floor class

    \fullpath
    ql/Instruments/%capfloor.hpp
*/

// $Id$

#ifndef quantlib_instruments_capfloor_h
#define quantlib_instruments_capfloor_h

#include "ql/instrument.hpp"
#include "ql/Instruments/simpleswap.hpp"
#include "ql/InterestRateModelling/model.hpp"

namespace QuantLib {

    namespace Instruments {

        class EuropeanCapFloor : public Instrument {
          public:
            enum Type { Cap, Floor };
            EuropeanCapFloor(Type type,
                const Handle<SimpleSwap>& swap,
                std::vector<Rate> exerciseRates,
                RelinkableHandle<TermStructure> termStructure);
            virtual ~EuropeanCapFloor() {}
            void useModel(const Handle<InterestRateModelling::Model>& model) {
                model_ = model;
            }
          private:
            virtual void performCalculations() const;
            Type type_;
            const Handle<SimpleSwap>& swap_;
            std::vector<Rate> exerciseRates_;
            RelinkableHandle<TermStructure> termStructure_;
            Handle<InterestRateModelling::Model> model_;
            size_t nPeriods_;
            std::vector<Time> startTimes_;
            std::vector<Time> endTimes_;
            std::vector<Time> tenors_;
            std::vector<Time> nominals_;
        };

        class EuropeanCap : public EuropeanCapFloor {
          public:
            EuropeanCap(const Handle<SimpleSwap>& swap,
                std::vector<Rate> exerciseRates,
                RelinkableHandle<TermStructure> termStructure)
            : EuropeanCapFloor( Cap, swap, exerciseRates, termStructure) {}
        };

        class EuropeanFloor : public EuropeanCapFloor {
          public:
            EuropeanFloor(const Handle<SimpleSwap>& swap,
                std::vector<Rate> exerciseRates,
                RelinkableHandle<TermStructure> termStructure)
            : EuropeanCapFloor( Floor, swap, exerciseRates, termStructure) {}
        };


    }
}

#endif
