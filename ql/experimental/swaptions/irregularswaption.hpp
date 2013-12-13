/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb
 Copyright (C) 2006 Cristina Duminuco
 Copyright (C) 2006 Marco Bianchetti
 Copyright (C) 2007 StatPro Italia srl
 Copyright (C) 2010 Andre Miemiec

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

/*! \file irregularswaption.hpp
    \brief Irregular swaption class
*/

#ifndef quantlib_instruments_irregular_swaption_hpp
#define quantlib_instruments_irregular_swaption_hpp

#include <ql/option.hpp>
#include <ql/experimental/swaptions/irregularswap.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>

namespace QuantLib {

    //! %settlement information
    struct IrregularSettlement {
        enum Type { Physical, Cash };
    };

    std::ostream& operator<<(std::ostream& out,
                             IrregularSettlement::Type type);

    //! Irregular %Swaption class
    /*! \ingroup instruments */
    class IrregularSwaption : public Option {
      public:
        class arguments;
        class engine;
        IrregularSwaption(const boost::shared_ptr<IrregularSwap>& swap,
                          const boost::shared_ptr<Exercise>& exercise,
                          IrregularSettlement::Type delivery = IrregularSettlement::Physical);
        //! \name Instrument interface
        //@{
        bool isExpired() const;
        void setupArguments(PricingEngine::arguments*) const;
        //@}
        //! \name Inspectors
        //@{
        IrregularSettlement::Type settlementType() const { return settlementType_; }
        IrregularSwap::Type type() const { return swap_->type(); }
        const boost::shared_ptr<IrregularSwap>& underlyingSwap() const {
            return swap_;
        }
        //@}
        //! implied volatility
        Volatility impliedVolatility(
                              Real price,
                              const Handle<YieldTermStructure>& discountCurve,
                              Volatility guess,
                              Real accuracy = 1.0e-4,
                              Natural maxEvaluations = 100,
                              Volatility minVol = 1.0e-7,
                              Volatility maxVol = 4.0) const;
      private:
        // arguments
        boost::shared_ptr<IrregularSwap> swap_;
        IrregularSettlement::Type settlementType_;
    };

    //! %Arguments for irregular-swaption calculation
    class IrregularSwaption::arguments : public IrregularSwap::arguments,
                                         public Option::arguments {
      public:
        arguments() : settlementType(IrregularSettlement::Physical) {}
        boost::shared_ptr<IrregularSwap> swap;
        IrregularSettlement::Type settlementType;
        void validate() const;
    };

    //! base class for irregular-swaption engines
    class IrregularSwaption::engine
        : public GenericEngine<IrregularSwaption::arguments, IrregularSwaption::results> {};

}

#endif
