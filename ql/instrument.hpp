
/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file instrument.hpp
    \brief Abstract instrument class
*/

#ifndef quantlib_instrument_h
#define quantlib_instrument_h

#include <ql/Patterns/lazyobject.hpp>
#include <ql/pricingengine.hpp>

namespace QuantLib {

    //! Abstract instrument class
    /*! This class is purely abstract and defines the interface of concrete
        instruments which will be derived from this one.
    */
    class Instrument : public LazyObject {
      public:
        Instrument();
        //! \name Inspectors
        //@{
        //! returns the net present value of the instrument.
        double NPV() const;
        //! returns the error estimate on the NPV when available.
        double errorEstimate() const;
        //! returns whether the instrument is still tradable.
        virtual bool isExpired() const = 0;
        //@}
        //! \name Modifiers
        //@{
        //! set the pricing engine to be used.
        /*! \warning calling this method will have no effects in
                     case the <b>performCalculation</b> method
                     was overridden in a derived class.
        */
        void setPricingEngine(const boost::shared_ptr<PricingEngine>&);
        //@}
        /*! When a derived argument structure is defined for an instrument,
            this method should be overridden to fill it. This is mandatory
            in case a pricing engine is used.
        */
        virtual void setupArguments(Arguments*) const;
      protected:
        //! \name Calculations 
        //@{
        void calculate() const;
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
        virtual void performCalculations() const;
        //@}
        /*! \name Results
            The value of this attribute and any other that derived 
            classes might declare must be set during calculation.
        */
        //@{
        mutable double NPV_, errorEstimate_;
        //@}
      protected:
        boost::shared_ptr<PricingEngine> engine_;
    };

    //! pricing results
    class Value : public virtual Results {
      public:
        Value() { reset(); }
        void reset() {
            value = errorEstimate = Null<double>();
        }
        double value;
        double errorEstimate;
    };


    // inline definitions

    inline Instrument::Instrument()
    : NPV_(0.0), errorEstimate_(Null<double>()) {}

    inline void Instrument::setPricingEngine(
                                  const boost::shared_ptr<PricingEngine>& e) {
        QL_REQUIRE(e, "null pricing engine");
        if (engine_)
            unregisterWith(engine_);
        engine_ = e;
        registerWith(engine_);
        // trigger (lazy) recalculation and notify observers
        update();
    }

    inline void Instrument::setupArguments(Arguments*) const {
        QL_FAIL("setupArguments() not implemented");
    }

    inline void Instrument::calculate() const {
        if (isExpired()) {
            setupExpired();
            calculated_ = true;
        } else {
            LazyObject::calculate();
        }
    }

    inline void Instrument::setupExpired() const {
        NPV_ = errorEstimate_ = 0.0;
    }

    inline void Instrument::performCalculations() const {
        QL_REQUIRE(engine_, "null pricing engine");
        engine_->reset();
        setupArguments(engine_->arguments());
        engine_->arguments()->validate();
        engine_->calculate();
        const Value* results = dynamic_cast<const Value*>(engine_->results());
        QL_ENSURE(results != 0,
                  "no results returned from pricing engine");
        NPV_ = results->value;
        errorEstimate_ = results->errorEstimate;
    }

    inline double Instrument::NPV() const {
        calculate();
        return NPV_;
    }

    inline double Instrument::errorEstimate() const {
        calculate();
        QL_REQUIRE(errorEstimate_ != Null<double>(),
                   "error estimate not provided");
        return errorEstimate_;
    }

}


#endif
