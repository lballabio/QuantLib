/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005, 2006, 2007 StatPro Italia srl

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

/*! \file instrument.hpp
    \brief Abstract instrument class
*/

#ifndef quantlib_instrument_hpp
#define quantlib_instrument_hpp

#include <ql/patterns/lazyobject.hpp>
#include <ql/pricingengine.hpp>
#include <ql/utilities/null.hpp>
#include <ql/time/date.hpp>
#include <ql/any.hpp>
#include <map>
#include <string>
#include <any>

namespace QuantLib {

    //! Abstract instrument class
    /*! This class is purely abstract and defines the interface of concrete
        instruments which will be derived from this one.

        \test observability of class instances is checked.
    */
    class Instrument : public LazyObject {
      public:
        class results;
        Instrument();
        //! \name Inspectors
        //@{

        //! returns the net present value of the instrument.
        Real NPV() const;
        //! returns the error estimate on the NPV when available.
        Real errorEstimate() const;
        //! returns the date the net present value refers to.
        const Date& valuationDate() const;

        //! returns any additional result returned by the pricing engine.
        template <typename T> T result(const std::string& tag) const;
        //! returns all additional result returned by the pricing engine.
        const std::map<std::string, std::any>& additionalResults() const;

        //! returns whether the instrument might have value greater than zero.
        virtual bool isExpired() const = 0;
        //@}
        //! \name Modifiers
        //@{
        //! set the pricing engine to be used.
        /*! \warning calling this method will have no effects in
                     case the <b>performCalculation</b> method
                     was overridden in a derived class.
        */
        void setPricingEngine(const std::shared_ptr<PricingEngine>&);
        //@}
        /*! When a derived argument structure is defined for an
            instrument, this method should be overridden to fill
            it. This is mandatory in case a pricing engine is used.
        */
        virtual void setupArguments(PricingEngine::arguments*) const;
        /*! When a derived result structure is defined for an
            instrument, this method should be overridden to read from
            it. This is mandatory in case a pricing engine is used.
        */
        virtual void fetchResults(const PricingEngine::results*) const;
      protected:
        //! \name Calculations
        //@{
        void calculate() const override;
        /*! This method must leave the instrument in a consistent
            state when the expiration condition is met.
        */
        virtual void setupExpired() const;
        /*! In case a pricing engine is <b>not</b> used, this
            method must be overridden to perform the actual
            calculations and set any needed results. In case
            a pricing engine is used, the default implementation
            can be used.
        */
        void performCalculations() const override;
        //@}
        /*! \name Results
            The value of this attribute and any other that derived
            classes might declare must be set during calculation.
        */
        //@{
        mutable Real NPV_, errorEstimate_;
        mutable Date valuationDate_;
        mutable std::map<std::string, std::any> additionalResults_;
        //@}
        std::shared_ptr<PricingEngine> engine_;
    };

    class Instrument::results : public virtual PricingEngine::results {
      public:
        void reset() override {
            value = errorEstimate = Null<Real>();
            valuationDate = Date();
            additionalResults.clear();
        }
        Real value;
        Real errorEstimate;
        Date valuationDate;
        std::map<std::string, std::any> additionalResults;
    };


    // inline definitions

    inline Instrument::Instrument() : NPV_(Null<Real>()), errorEstimate_(Null<Real>()) {}

    inline void Instrument::setPricingEngine(
                                  const std::shared_ptr<PricingEngine>& e) {
        if (engine_ != nullptr)
            unregisterWith(engine_);
        engine_ = e;
        if (engine_ != nullptr)
            registerWith(engine_);
        // trigger (lazy) recalculation and notify observers
        update();
    }

    inline void Instrument::setupArguments(PricingEngine::arguments*) const {
        QL_FAIL("Instrument::setupArguments() not implemented");
    }

    inline void Instrument::calculate() const {
        if (!calculated_) {
            if (isExpired()) {
                setupExpired();
                calculated_ = true;
            } else {
                LazyObject::calculate();
            }
        }
    }

    inline void Instrument::setupExpired() const {
        NPV_ = errorEstimate_ = 0.0;
        valuationDate_ = Date();
        additionalResults_.clear();
    }

    inline void Instrument::performCalculations() const {
        QL_REQUIRE(engine_, "null pricing engine");
        engine_->reset();
        setupArguments(engine_->getArguments());
        engine_->getArguments()->validate();
        engine_->calculate();
        fetchResults(engine_->getResults());
    }

    inline void Instrument::fetchResults(
                                      const PricingEngine::results* r) const {
        const auto* results = dynamic_cast<const Instrument::results*>(r);
        QL_ENSURE(results != nullptr, "no results returned from pricing engine");

        NPV_ = results->value;
        errorEstimate_ = results->errorEstimate;
        valuationDate_ = results->valuationDate;

        additionalResults_ = results->additionalResults;
    }

    inline Real Instrument::NPV() const {
        calculate();
        QL_REQUIRE(NPV_ != Null<Real>(), "NPV not provided");
        return NPV_;
    }

    inline Real Instrument::errorEstimate() const {
        calculate();
        QL_REQUIRE(errorEstimate_ != Null<Real>(),
                   "error estimate not provided");
        return errorEstimate_;
    }

    inline const Date& Instrument::valuationDate() const {
        calculate();
        QL_REQUIRE(valuationDate_ != Date(),
                   "valuation date not provided");
        return valuationDate_;
    }

    template <class T>
    inline T Instrument::result(const std::string& tag) const {
        calculate();
        std::map<std::string, std::any>::const_iterator value =
            additionalResults_.find(tag);
        QL_REQUIRE(value != additionalResults_.end(),
                   tag << " not provided");
        return std::any_cast<T>(value->second);
    }

    inline const std::map<std::string, std::any>&
    Instrument::additionalResults() const {
        calculate();
        return additionalResults_;
    }

}

#endif
