/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2010 Master IMAFA - Polytech'Nice Sophia - Université de Nice Sophia Antipolis

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

/*! \file margrabeoption.hpp
    \brief Margrabe option on two assets
*/

#ifndef quantlib_margrabe_option_hpp
#define quantlib_margrabe_option_hpp

#include <ql/instruments/multiassetoption.hpp>

namespace QuantLib {

    //! Margrabe option on two assets
    /*! This option gives the holder the right to exchange Q2 stocks
        of the second asset for Q1 stocks of the first at expiration.

        \ingroup instruments
    */
    class MargrabeOption : public MultiAssetOption {
      public:
        class arguments;
        class results;
        class engine;
        MargrabeOption(Integer Q1,
                       Integer Q2,
                       const std::shared_ptr<Exercise>&);
        void setupArguments(PricingEngine::arguments*) const override;
        Real delta1() const;
        Real delta2() const;
        Real gamma1() const;
        Real gamma2() const;
        void fetchResults(const PricingEngine::results*) const override;

      protected:
        Integer Q1_;
        Integer Q2_;
        mutable Real delta1_, delta2_, gamma1_, gamma2_;
    };

    //! Extra %arguments for Margrabe option
    class MargrabeOption::arguments
        : public MultiAssetOption::arguments {
      public:
        arguments() : Q1(Null<Integer>()),
                      Q2(Null<Integer>()) {}
        void validate() const override;
        Integer Q1;
        Integer Q2;
    };

    //! Extra %results for Margrabe option
    class MargrabeOption::results
        : public MultiAssetOption::results {
      public:
        results() : delta1(Null<Real>()),
                    delta2(Null<Real>()),
                    gamma1(Null<Real>()),
                    gamma2(Null<Real>()) {}
        Real delta1;
        Real delta2;
        Real gamma1;
        Real gamma2;
        void reset() override {
            MultiAssetOption::results::reset();
            delta1 = Null<Real>();
            delta2 = Null<Real>();
            gamma1 = Null<Real>();
            gamma2 = Null<Real>();
        }
    };

    //! %Margrabe option %engine base class
    class MargrabeOption::engine
        : public GenericEngine<MargrabeOption::arguments,
                               MargrabeOption::results> {};
}


#endif
