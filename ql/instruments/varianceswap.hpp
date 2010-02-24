/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Warren Chou
 Copyright (C) 2008 StatPro Italia srl

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

/*! \file varianceswap.hpp
    \brief Variance swap
*/

#ifndef quantlib_variance_swap_hpp
#define quantlib_variance_swap_hpp

#include <ql/processes/blackscholesprocess.hpp>
#include <ql/instruments/payoffs.hpp>
#include <ql/option.hpp>
#include <ql/position.hpp>

namespace QuantLib {

    //! Variance swap
    /*! \warning This class does not manage seasoned variance swaps.

        \ingroup instruments
    */
    class VarianceSwap : public Instrument {
      public:
        class arguments;
        class results;
        class engine;
        VarianceSwap(Position::Type position,
                     Real strike,
                     Real notional,
                     const Date& startDate,
                     const Date& maturityDate);
        //! \name Instrument interface
        //@{
        bool isExpired() const;
        //@}
        //! \name Additional interface
        //@{
        // inspectors
        Real strike() const;
        Position::Type position() const;
        Date startDate() const;
        Date maturityDate() const;
        Real notional() const;
        // results
        Real variance() const;
        //@}
        // other
        void setupArguments(PricingEngine::arguments* args) const;
        void fetchResults(const PricingEngine::results*) const;
      protected:
        void setupExpired() const;
        // data members
        Position::Type position_;
        Real strike_;
        Real notional_;
        Date startDate_, maturityDate_;
        // results
        mutable Real variance_;
    };


    //! %Arguments for forward fair-variance calculation
    class VarianceSwap::arguments : public virtual PricingEngine::arguments {
      public:
        arguments() : strike(Null<Real>()), notional(Null<Real>()) {}
        void validate() const;
        Position::Type position;
        Real strike;
        Real notional;
        Date startDate;
        Date maturityDate;
    };


    //! %Results from variance-swap calculation
    class VarianceSwap::results : public Instrument::results {
      public:
        Real variance;
        void reset() {
            Instrument::results::reset();
            variance = Null<Real>();
        }
    };

    //! base class for variance-swap engines
    class VarianceSwap::engine :
        public GenericEngine<VarianceSwap::arguments,
                             VarianceSwap::results> {};


    // inline definitions

    inline Date VarianceSwap::startDate() const {
        return startDate_;
    }

    inline Date VarianceSwap::maturityDate() const {
        return maturityDate_;
    }

    inline Real VarianceSwap::strike() const {
        return strike_;
    }

    inline Real VarianceSwap::notional() const {
        return notional_;
    }

    inline Position::Type VarianceSwap::position() const {
        return position_;
    }

}


#endif
