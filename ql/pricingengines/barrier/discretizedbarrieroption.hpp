/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2014 Thema Consulting SA

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

#ifndef quantlib_discretized_barrier_option_h
#define quantlib_discretized_barrier_option_h

#include <ql/discretizedasset.hpp>
#include <ql/methods/lattices/bsmlattice.hpp>
#include <ql/instruments/barrieroption.hpp>
#include <ql/pricingengines/vanilla/discretizedvanillaoption.hpp>

namespace QuantLib {

    class DiscretizedBarrierOption : public DiscretizedAsset {
      public:
        DiscretizedBarrierOption(const BarrierOption::arguments&,
                                 const StochasticProcess& process,
                                 const TimeGrid& grid = TimeGrid());

        void reset(Size size) override;

        const Array& vanilla() const { 
            return vanilla_.values(); 
        }

        const BarrierOption::arguments& arguments() const {
           return arguments_;
        }

        std::vector<Time> mandatoryTimes() const override { return stoppingTimes_; }

        void checkBarrier(Array &optvalues, const Array &grid) const;
      protected:
        void postAdjustValuesImpl() override;

      private:
        BarrierOption::arguments arguments_;
        std::vector<Time> stoppingTimes_;
        DiscretizedVanillaOption vanilla_; 
    };

    class DiscretizedDermanKaniBarrierOption : public DiscretizedAsset {
      public:
        DiscretizedDermanKaniBarrierOption(const BarrierOption::arguments&,
                                 const StochasticProcess& process,
                                 const TimeGrid& grid = TimeGrid());

        void reset(Size size) override;

        std::vector<Time> mandatoryTimes() const override { return unenhanced_.mandatoryTimes(); }

      protected:
        void postAdjustValuesImpl() override;

      private:
        void adjustBarrier(Array &optvalues, const Array &grid);
        DiscretizedBarrierOption unenhanced_;
    };
}





#endif


#ifndef id_cfbd3edf1c94268edf47522a5a48db49
#define id_cfbd3edf1c94268edf47522a5a48db49
inline bool test_cfbd3edf1c94268edf47522a5a48db49(int* i) { return i != 0; }
#endif
