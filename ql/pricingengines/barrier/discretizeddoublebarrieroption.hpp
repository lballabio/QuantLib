/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2012 Thema Consulting SA (programmer: Riccardo Ghetta)

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file discretizedbarrieroption.hpp
    \brief discretized barrier option
*/

#ifndef quantlib_discretized_double_barrier_option_h
#define quantlib_discretized_double_barrier_option_h

#include <ql/discretizedasset.hpp>
#include <ql/methods/lattices/bsmlattice.hpp>
#include <ql/instruments/doublebarrieroption.hpp>
#include <ql/pricingengines/vanilla/discretizedvanillaoption.hpp>

namespace QuantLib {

    class DiscretizedDoubleBarrierOption : public DiscretizedAsset {
      public:
        DiscretizedDoubleBarrierOption(const DoubleBarrierOption::arguments&,
                                 const StochasticProcess& process,
                                 const TimeGrid& grid = TimeGrid());

        void reset(Size size);

        const Array& vanilla() const { 
            return vanilla_.values(); 
        }

        virtual std::vector<Time> mandatoryTimes() const {
            return stoppingTimes_;
        }

        void checkBarrier(Array &optvalues, const Array &grid) const;
      protected:
        void postAdjustValuesImpl();
      private:
        DoubleBarrierOption::arguments arguments_;
        std::vector<Time> stoppingTimes_;
        DiscretizedVanillaOption vanilla_; 
    };
}

#endif
