
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

/*! \file europeanswaption.hpp
    \brief European swaption

    \fullpath
    ql/Instruments/%europeanswaption.hpp
*/

// $Id$

#ifndef quantlib_instruments_european_swaption_h
#define quantlib_instruments_european_swaption_h

#include <vector>
#include "instrument.hpp"
#include "ql/Instruments/simpleswap.hpp"
#include "ql/InterestRateModelling/model.hpp"


namespace QuantLib {

    namespace Instruments {

        using InterestRateModelling::Model;

        class EuropeanSwaption : public Instrument{
          public:
            EuropeanSwaption( 
                const Handle<SimpleSwap>& swap,
                Date maturity,
                RelinkableHandle<TermStructure> termStructure);
            virtual ~EuropeanSwaption() {}
            void useModel(const Handle<Model>& model) {
                model_ = model;
            }
          private:
            virtual void performCalculations() const;
            const Handle<SimpleSwap>& swap_;
            Date maturity_;
            RelinkableHandle<TermStructure> termStructure_;
            Handle<Model> model_;
            std::vector<Time> endTimes_;
            std::vector<double> coupons_;
        };
    }
}

#endif
