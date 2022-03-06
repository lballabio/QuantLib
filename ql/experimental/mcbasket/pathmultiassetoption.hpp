/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Andrea Odetti

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

/*! \file pathmultiassetoption.hpp
    \brief Option on multiple assets
*/

#ifndef quantlib_path_multiasset_option_hpp
#define quantlib_path_multiasset_option_hpp

#include <ql/instrument.hpp>
#include <ql/stochasticprocess.hpp>
#include <ql/math/matrix.hpp>
#include <ql/experimental/mcbasket/pathpayoff.hpp>

namespace QuantLib {

    //! Base class for path-dependent options on multiple assets
    class PathMultiAssetOption : public Instrument {
      public:
        explicit PathMultiAssetOption(
                        const ext::shared_ptr<PricingEngine>& engine
                                        = ext::shared_ptr<PricingEngine>());

        //! \name Instrument interface
        //@{
        class arguments;
        class results;
        class engine;

        bool isExpired() const override;

        void setupArguments(PricingEngine::arguments*) const override;

        virtual ext::shared_ptr<PathPayoff> pathPayoff()  const = 0;
        virtual std::vector<Date>             fixingDates() const = 0;

      protected:
        void setupExpired() const override;
    };

    //! %Arguments for multi-asset option calculation
    class PathMultiAssetOption::arguments
        : public virtual PricingEngine::arguments {
      public:
        arguments() = default;
        void validate() const override;

        ext::shared_ptr<PathPayoff>        payoff;
        std::vector<Date>                    fixingDates;
    };

    //! %Results from multi-asset option calculation
    class PathMultiAssetOption::results : public Instrument::results {
      public:
        void reset() override { Instrument::results::reset(); }
    };

    class PathMultiAssetOption::engine
        : public GenericEngine<PathMultiAssetOption::arguments,
                               PathMultiAssetOption::results> {};

}


#endif


#ifndef id_3449b528dc9614466ba159eff67250ad
#define id_3449b528dc9614466ba159eff67250ad
inline bool test_3449b528dc9614466ba159eff67250ad(const int* i) {
    return i != nullptr;
}
#endif
