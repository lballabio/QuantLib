
/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file marketelement.hpp
    \brief purely virtual base class for market observables
*/

#ifndef quantlib_market_element_hpp
#define quantlib_market_element_hpp

#include <ql/relinkablehandle.hpp>

namespace QuantLib {

    //! purely virtual base class for market observables
    class MarketElement : public Patterns::Observable {
      public:
        virtual ~MarketElement() {}
        //! returns the current value
        virtual double value() const = 0;
    };

    //! market element returning a stored value
    class SimpleMarketElement : public MarketElement {
      public:
        SimpleMarketElement(double value);
        //! \name Market element interface
        //@{
        double value() const;
        //@}
        //! \name Modifiers
        //@{
        void setValue(double value);
        //@}
      private:
        double value_;
    };

    //! market element whose value depends on another market element
    template <class UnaryFunction>
    class DerivedMarketElement : public MarketElement,
                                 public Patterns::Observer {
      public:
        DerivedMarketElement(const RelinkableHandle<MarketElement>& element,
                             const UnaryFunction& f);
        //! \name Market element interface
        //@{
        double value() const;
        //@}
        //! \name Observer interface
        //@{
        void update();
        //@}
      private:
        RelinkableHandle<MarketElement> element_;
        UnaryFunction f_;
    };

    //! market element whose value depends on two other market element
    template <class BinaryFunction>
    class CompositeMarketElement : public MarketElement,
                                   public Patterns::Observer {
      public:
        CompositeMarketElement(
            const RelinkableHandle<MarketElement>& element1,
            const RelinkableHandle<MarketElement>& element2,
            const BinaryFunction& f);
        //! \name Market element interface
        //@{
        double value() const;
        //@}
        //! \name Observer interface
        //@{
        void update();
        //@}
      private:
        RelinkableHandle<MarketElement> element1_, element2_;
        BinaryFunction f_;
    };


    // inline definitions

    // simple market element

    inline SimpleMarketElement::SimpleMarketElement(double value)
    : value_(value) {}

    inline double SimpleMarketElement::value() const {
        return value_;
    }

    inline void SimpleMarketElement::setValue(double value) {
        value_ = value;
        notifyObservers();
    }


    // derived market element

    template <class UnaryFunction>
    inline DerivedMarketElement<UnaryFunction>::DerivedMarketElement(
        const RelinkableHandle<MarketElement>& element,
        const UnaryFunction& f)
    : element_(element), f_(f) {
        registerWith(element_);
    }

    template <class UnaryFunction>
    inline double DerivedMarketElement<UnaryFunction>::value() const {
        QL_REQUIRE(!element_.isNull(),
            "DerivedMarketElement: null market element set");
        return f_(element_->value());
    }

    template <class UnaryFunction>
    inline void DerivedMarketElement<UnaryFunction>::update() {
        notifyObservers();
    }

    // composite market element

    template <class BinaryFunction>
    inline CompositeMarketElement<BinaryFunction>::CompositeMarketElement(
        const RelinkableHandle<MarketElement>& element1,
        const RelinkableHandle<MarketElement>& element2,
        const BinaryFunction& f)
    : element1_(element1), element2_(element2), f_(f) {
        registerWith(element1_);
        registerWith(element2_);
    }

    template <class BinaryFunction>
    inline double CompositeMarketElement<BinaryFunction>::value() const {
        QL_REQUIRE(!element1_.isNull() && !element2_.isNull(),
            "null market element set");
        return f_(element1_->value(),element2_->value());
    }

    template <class BinaryFunction>
    inline void CompositeMarketElement<BinaryFunction>::update() {
        notifyObservers();
    }

}


#endif

