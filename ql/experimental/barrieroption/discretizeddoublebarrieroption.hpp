/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2015 Thema Consulting SA

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
#include <ql/experimental/barrieroption/doublebarrieroption.hpp>
#include <ql/pricingengines/vanilla/discretizedvanillaoption.hpp>

namespace QuantLib {

    //! Standard discretized option helper class
    /*! This class is used with the BinomialDoubleBarrierEngine to
        implement a standard binomial algorithm for double barrier
        options
    */
    class DiscretizedDoubleBarrierOption : public DiscretizedAsset {
      public:
        DiscretizedDoubleBarrierOption(const DoubleBarrierOption::arguments&,
                                 const StochasticProcess& process,
                                 const TimeGrid& grid = TimeGrid());

        void reset(Size size) override;

        const Array& vanilla() const { 
            return vanilla_.values(); 
        }

        const DoubleBarrierOption::arguments& arguments() const {
           return arguments_;
        }

        std::vector<Time> mandatoryTimes() const override { return stoppingTimes_; }

        void checkBarrier(Array &optvalues, const Array &grid) const;
      protected:
        void postAdjustValuesImpl() override;

      private:
        DoubleBarrierOption::arguments arguments_;
        std::vector<Time> stoppingTimes_;
        DiscretizedVanillaOption vanilla_; 
    };

    //! Derman-Kani-Ergener-Bardhan discretized option helper class
    /*! This class is used with the BinomialDoubleBarrierEngine to
        implement the enhanced binomial algorithm of E.Derman, I.Kani,
        D.Ergener, I.Bardhan ("Enhanced Numerical Methods for Options with
        Barriers", 1995)

        \note This algorithm is only suitable if the payoff can be approximated 
        linearly, e.g. is not usable for cash-or-nothing payoffs.
    */
    class DiscretizedDermanKaniDoubleBarrierOption : public DiscretizedAsset {
      public:
        DiscretizedDermanKaniDoubleBarrierOption(const DoubleBarrierOption::arguments&,
                                 const StochasticProcess& process,
                                 const TimeGrid& grid = TimeGrid());

        void reset(Size size) override;

        std::vector<Time> mandatoryTimes() const override { return unenhanced_.mandatoryTimes(); }

      protected:
        void postAdjustValuesImpl() override;

      private:
        void adjustBarrier(Array &optvalues, const Array &grid);
        DiscretizedDoubleBarrierOption unenhanced_;
    };
}

#endif


#ifndef id_d447aeef7a6c42e0c45d974dac925cad
#define id_d447aeef7a6c42e0c45d974dac925cad
inline bool test_d447aeef7a6c42e0c45d974dac925cad(int* i) { return i != 0; }
#endif
