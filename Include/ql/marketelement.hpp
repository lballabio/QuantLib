
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.org/
 *
 * QuantLib is free software and you are allowed to use, copy, modify, merge,
 * publish, distribute, and/or sell copies of it under the conditions stated
 * in the QuantLib License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
 *
 * You should have received a copy of the license along with this file;
 * if not, please email quantlib-users@lists.sourceforge.net
 * The license is also available at http://quantlib.org/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.org/group.html
*/

/*! \file marketelement.hpp
    \brief purely virtual base class for market observables

    $Id$
*/

// $Source$
// $Log$
// Revision 1.4  2001/08/07 11:25:53  sigmud
// copyright header maintenance
//
// Revision 1.3  2001/07/25 15:47:27  sigmud
// Change from quantlib.sourceforge.net to quantlib.org
//
// Revision 1.2  2001/07/24 08:49:32  sigmud
// pruned redundant header inclusions
//
// Revision 1.1  2001/07/09 16:29:27  lballabio
// Some documentation and market element
//

#ifndef quantlib_market_element_hpp
#define quantlib_market_element_hpp

#include "ql/relinkablehandle.hpp"

namespace QuantLib {

    //! purely purely virtual base class for market observables
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
        ~DerivedMarketElement();
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
        CompositeMarketElement(const RelinkableHandle<MarketElement>& element1,
                               const RelinkableHandle<MarketElement>& element2,
                               const BinaryFunction& f);
        ~CompositeMarketElement();
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
        element_.registerObserver(this);
    }

    template <class UnaryFunction>
    inline DerivedMarketElement<UnaryFunction>::~DerivedMarketElement() {
        element_.unregisterObserver(this);
    }
    
    template <class UnaryFunction>
    inline double DerivedMarketElement<UnaryFunction>::value() const {
        QL_REQUIRE(!element_.isNull(),
            "null market element set");
        return f_(element_->value());
    }

    template <class UnaryFunction>
    inline void DerivedMarketElement<UnaryFunction>::update() {
        notifyObserver();
    }

    // composite market element

    template <class BinaryFunction>
    inline CompositeMarketElement<BinaryFunction>::CompositeMarketElement(
        const RelinkableHandle<MarketElement>& element1,
        const RelinkableHandle<MarketElement>& element2,
        const BinaryFunction& f) 
    : element1_(element1), element2_(element2), f_(f) {
        element1_.unregisterObserver(this);
        element2_.unregisterObserver(this);
    }

    template <class BinaryFunction>
    inline CompositeMarketElement<BinaryFunction>::~CompositeMarketElement() {
        element1_.unregisterObserver(this);
        element2_.unregisterObserver(this);
    }

    template <class BinaryFunction>
    inline double CompositeMarketElement<BinaryFunction>::value() const {
        QL_REQUIRE(!element1_.isNull() && !element2_.isNull(),
            "null market element set");
        return f_(element1_->value(),element2_->value());
    }

    template <class BinaryFunction>
    inline void CompositeMarketElement<BinaryFunction>::update() {
        notifyObserver();
    }
    
}


#endif
