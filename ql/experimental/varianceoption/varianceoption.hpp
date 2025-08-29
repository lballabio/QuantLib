/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file varianceoption.hpp
    \brief Variance option
*/

#ifndef quantlib_variance_option_hpp
#define quantlib_variance_option_hpp

#include <ql/processes/blackscholesprocess.hpp>
#include <ql/instruments/payoffs.hpp>
#include <ql/option.hpp>
#include <ql/position.hpp>

namespace QuantLib {

    //! Variance option
    /*! \warning This class does not manage seasoned variance options.

        \ingroup instruments
    */
    class VarianceOption : public Instrument {
      public:
        class arguments;
        class results;
        class engine;
        VarianceOption(ext::shared_ptr<Payoff> payoff,
                       Real notional,
                       const Date& startDate,
                       const Date& maturityDate);
        //! \name Instrument interface
        //@{
        bool isExpired() const override;
        //@}
        //! \name Inspectors
        //@{
        Date startDate() const;
        Date maturityDate() const;
        Real notional() const;
        ext::shared_ptr<Payoff> payoff() const;
        //@}
        void setupArguments(PricingEngine::arguments* args) const override;

      protected:
        // data members
        ext::shared_ptr<Payoff> payoff_;
        Real notional_;
        Date startDate_, maturityDate_;
    };


    //! %Arguments for forward fair-variance calculation
    class VarianceOption::arguments : public virtual PricingEngine::arguments {
      public:
        arguments() : notional(Null<Real>()) {}
        void validate() const override;
        ext::shared_ptr<Payoff> payoff;
        Real notional;
        Date startDate;
        Date maturityDate;
    };


    //! %Results from variance-option calculation
    class VarianceOption::results : public Instrument::results {};

    //! base class for variance-option engines
    class VarianceOption::engine :
        public GenericEngine<VarianceOption::arguments,
                             VarianceOption::results> {};


    // inline definitions

    inline Date VarianceOption::startDate() const {
        return startDate_;
    }

    inline Date VarianceOption::maturityDate() const {
        return maturityDate_;
    }

    inline Real VarianceOption::notional() const {
        return notional_;
    }

    inline ext::shared_ptr<Payoff> VarianceOption::payoff() const {
        return payoff_;
    }

}


#endif
