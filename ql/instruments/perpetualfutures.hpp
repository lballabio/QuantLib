/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2025 DRYXZW

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

/*! \file perpetualfutures.hpp
    \brief Perpetual Futures
*/

#ifndef quantlib_perpetual_futures_hpp
#define quantlib_perpetual_futures_hpp

#include <ql/instrument.hpp>

namespace QuantLib {
    // Futures has no termination date mainly for cryptocurrencies
    // Base class of perpetual futures
    // Funding style is different in different exchange
    class PerpetualFutures : public Instrument {
        public:
            class arguments;
            class engine;
            enum PayoffType { Linear, Inverse, Quanto };
            enum FundingType { AHJ };

            PerpetualFutures(PerpetualFutures::PayoffType payoffType,
                             PerpetualFutures::FundingType fundingType);
            void setupArguments(PricingEngine::arguments*) const override;
    
        private:
            PayoffType payoffType_;
            FundingType fundingType_;
    };

    std::ostream& operator<<(std::ostream& out, PerpetualFutures::PayoffType type);
    std::ostream& operator<<(std::ostream& out, PerpetualFutures::FundingType type);

    //! %Arguments for perpetual futures calculation
    class PerpetualFutures::arguments : public PricingEngine::arguments {
      public:
        arguments();
        PerpetualFutures::PayoffType payoffType;
        PerpetualFutures::FundingType fundingType;
        void validate() const override;
    };

    //! %Perpetual futures %engine base class
    class PerpetualFutures::engine
    : public GenericEngine<PerpetualFutures::arguments, PerpetualFutures::results> {
    };
}

#endif