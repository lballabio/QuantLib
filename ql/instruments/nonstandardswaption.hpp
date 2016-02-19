/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2013 Peter Caspers

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

/*! \file nonstandardswaption.hpp
    \brief nonstandard swap option class
*/

#ifndef quantlib_instruments_nonstandardswaption_hpp
#define quantlib_instruments_nonstandardswaption_hpp

#include <ql/option.hpp>
#include <ql/instruments/swaption.hpp>
#include <ql/instruments/nonstandardswap.hpp>
#include <ql/pricingengines/swaption/basketgeneratingengine.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/termstructures/volatility/swaption/swaptionvolstructure.hpp>
#include <ql/models/calibrationhelper.hpp>
#include <ql/utilities/disposable.hpp>
#include <ql/experimental/fx/proxyinstrument.hpp>

namespace QuantLib {

//! nonstandard swaption class
/*! \ingroup instruments
*/

class NonstandardSwaption : public Option, public ProxyInstrument {
  public:
    class arguments;
    class results;
    class engine;

    //! proxy description
    struct Proxy : ProxyDescription {
        struct ProxyFunction {
            // Function taking the model state and
            // returning the value.
            virtual Real operator()(const Real state) const = 0;
        };
        // open exercise expiry dates
        std::vector<Date> expiryDates;
        // original evaluation date
        Date origEvalDate;
        // original model
        boost::shared_ptr<Gaussian1dModel> model;
        // original discount curve (if given)
        Handle<YieldTermStructure> discount;
        // original oas (if given)
        Handle<Quote> oas;
        // regression functions model state => deflated npv
        // currently only models with one dimensional state
        // are supported
        std::vector<boost::shared_ptr<ProxyFunction> > regression;
        void validate() const {
            QL_REQUIRE(regression.size() == expiryDates.size(),
                       "Number of regression functions ("
                           << regression.size()
                           << ") does not match number of exercise dates ("
                           << expiryDates.size() << ")");
            QL_REQUIRE(model != NULL, "no model given");
        }
    };

    NonstandardSwaption(const Swaption &fromSwaption);
    NonstandardSwaption(const boost::shared_ptr<NonstandardSwap> &swap,
                        const boost::shared_ptr<Exercise> &exercise,
                        Settlement::Type delivery = Settlement::Physical);

    //! \name Instrument interface
    //@{
    bool isExpired() const;
    void setupArguments(PricingEngine::arguments *) const;
    void fetchResults(const PricingEngine::results *) const;
    //@}
    //! \name Inspectors
    //@{
    VanillaSwap::Type type() const { return swap_->type(); }

    const boost::shared_ptr<NonstandardSwap> &underlyingSwap() const {
        return swap_;
    }
    //@}

    //! description for proxy pricing
    boost::shared_ptr<ProxyDescription> proxy() const;

    Disposable<std::vector<boost::shared_ptr<CalibrationHelper> > >
    calibrationBasket(
        boost::shared_ptr<SwapIndex> standardSwapBase,
        boost::shared_ptr<SwaptionVolatilityStructure> swaptionVolatility,
        const BasketGeneratingEngine::CalibrationBasketType basketType =
            BasketGeneratingEngine::MaturityStrikeByDeltaGamma) const;

  private:
    // arguments
    boost::shared_ptr<NonstandardSwap> swap_;
    Settlement::Type settlementType_;
    // proxy pricing information
    mutable boost::shared_ptr<ProxyDescription> proxy_;
};

//! %Arguments for nonstandard swaption calculation
class NonstandardSwaption::arguments : public NonstandardSwap::arguments,
                                       public Option::arguments {
  public:
    arguments() {}
    boost::shared_ptr<NonstandardSwap> swap;
    Settlement::Type settlementType;
    void validate() const;
};

class NonstandardSwaption::results : public Option::results {
  public:
    void reset();
    boost::shared_ptr<NonstandardSwaption::Proxy> proxy;
};

//! base class for nonstandard swaption engines
class NonstandardSwaption::engine
    : public GenericEngine<NonstandardSwaption::arguments,
                           NonstandardSwaption::results> {};
}

#endif
