/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Marco Bianchetti
 Copyright (C) 2007 Giorgio Facchinetti

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

/*! \file stickyratchet.hpp
    \brief Payoffs for double nested options of sticky or ratchet type
*/

#ifndef quantlib_stickyratchet_hpp
#define quantlib_stickyratchet_hpp

#include <ql/option.hpp>
#include <ql/payoff.hpp>

namespace QuantLib {
    
    class AcyclicVisitor;
    //! Intermediate class for single/double sticky/ratchet payoffs.
    //  initialValues can be a (forward) rate or a coupon/accrualFactor 
    class DoubleStickyRatchetPayoff : public Payoff {
      public:
        DoubleStickyRatchetPayoff(Real type1, Real type2,
                            Real gearing1, Real gearing2, Real gearing3,
                            Real spread1, Real spread2, Real spread3,
                            Real initialValue1, Real initialValue2, 
                            Real accrualFactor) 
        : type1_(type1), type2_(type2), 
          gearing1_(gearing1), gearing2_(gearing2), gearing3_(gearing3),
          spread1_(spread1), spread2_(spread2), spread3_(spread3),
          initialValue1_(initialValue1), initialValue2_(initialValue2), 
          accrualFactor_(accrualFactor) {}
        //! \name Payoff interface
        //@{
        std::string name() const;
        Real operator()(Real forward) const;
        std::string description() const;
        virtual void accept(AcyclicVisitor&);
        //@}
      protected:
        Real type1_ ,type2_;
        Real gearing1_, gearing2_, gearing3_;
        Real spread1_, spread2_, spread3_;
        Real initialValue1_, initialValue2_, accrualFactor_;
    };

    //! Ratchet payoff (single option)
    class RatchetPayoff : public DoubleStickyRatchetPayoff {
      public:
         RatchetPayoff(Real gearing1, Real gearing2,
                       Real spread1, Real spread2,
                       Real initialValue, Real accrualFactor)
        : DoubleStickyRatchetPayoff(-1.0, 0.0,
                            gearing1, 0.0, gearing2,
                            spread1, 0.0, spread2,
                            initialValue, 0.0, 
                            accrualFactor) {}
        //! \name Payoff interface
        //@{
        std::string name() const { return "Ratchet";}
        //@}
    };    

    //! Sticky payoff (single option)
    class StickyPayoff : public DoubleStickyRatchetPayoff {
      public:
         StickyPayoff(Real gearing1, Real gearing2,
                       Real spread1, Real spread2,
                       Real initialValue, Real accrualFactor)
        : DoubleStickyRatchetPayoff(+1.0, 0.0,
                            gearing1, 0.0, gearing2,
                            spread1, 0.0, spread2,
                            initialValue, 0.0, 
                            accrualFactor) {}
        //! \name Payoff interface
        //@{
        std::string name() const { return "Sticky";}
        //@}
    };

    //! RatchetMax payoff (double option)
    class RatchetMaxPayoff : public DoubleStickyRatchetPayoff {
      public:
         RatchetMaxPayoff(Real gearing1, Real gearing2, Real gearing3,
                          Real spread1, Real spread2, Real spread3,
                          Real initialValue1, Real initialValue2, 
                          Real accrualFactor)
        : DoubleStickyRatchetPayoff(-1.0, -1.0,
                                    gearing1, gearing2, gearing3,
                                    spread1, spread2, spread3,
                                    initialValue1, initialValue2, 
                                    accrualFactor) {}
        //! \name Payoff interface
        //@{
        std::string name() const { return "RatchetMax";}
        //@}
    };    

    //! RatchetMin payoff (double option)
    class RatchetMinPayoff : public DoubleStickyRatchetPayoff {
      public:
         RatchetMinPayoff(Real gearing1, Real gearing2, Real gearing3,
                          Real spread1, Real spread2, Real spread3,
                          Real initialValue1, Real initialValue2, 
                          Real accrualFactor)
        : DoubleStickyRatchetPayoff(-1.0, +1.0,
                                    gearing1, gearing2, gearing3,
                                    spread1, spread2, spread3,
                                    initialValue1, initialValue2, 
                                    accrualFactor) {}
        //! \name Payoff interface
        //@{
        std::string name() const { return "RatchetMin";}
        //@}
    };    

    //! StickyMax payoff (double option)
    class StickyMaxPayoff : public DoubleStickyRatchetPayoff {
      public:
         StickyMaxPayoff(Real gearing1, Real gearing2, Real gearing3,
                          Real spread1, Real spread2, Real spread3,
                          Real initialValue1, Real initialValue2, 
                          Real accrualFactor)
        : DoubleStickyRatchetPayoff(+1.0, -1.0,
                                    gearing1, gearing2, gearing3,
                                    spread1, spread2, spread3,
                                    initialValue1, initialValue2, 
                                    accrualFactor) {}
        //! \name Payoff interface
        //@{
        std::string name() const { return "StickyMax";}
        //@}
    };    

    //! StickyMin payoff (double option)
    class StickyMinPayoff : public DoubleStickyRatchetPayoff {
      public:
         StickyMinPayoff(Real gearing1, Real gearing2, Real gearing3,
                          Real spread1, Real spread2, Real spread3,
                          Real initialValue1, Real initialValue2, 
                          Real accrualFactor)
        : DoubleStickyRatchetPayoff(+1.0, +1.0,
                                    gearing1, gearing2, gearing3,
                                    spread1, spread2, spread3,
                                    initialValue1, initialValue2, 
                                    accrualFactor) {}
        //! \name Payoff interface
        //@{
        std::string name() const { return "StickyMin";}
        //@}
    };    

/*---------------------------------------------------------------------------------
    // Old code for single sticky/ratchet payoffs, 
    // superated by DoubleStickyRatchetPayoff class above

    //! Intermediate class for sticky/ratchet payoffs
    //  initialValue can be a (forward) rate or a coupon/accrualFactor 
    class StickyRatchetPayoff : public Payoff {
      public:
        StickyRatchetPayoff(Real type,
                            Real gearing1, Real gearing2,
                            Real spread1, Real spread2,
                            Real initialValue, Real accrualFactor) 
        : type_(type), gearing1_(gearing1), gearing2_(gearing2), 
          spread1_(spread1), spread2_(spread2), initialValue_(initialValue),
          accrualFactor_(accrualFactor) {}
        //! \name Payoff interface
        //@{
        Real operator()(Real forward) const;
        std::string description() const;
        virtual void accept(AcyclicVisitor&);
        //@}
      protected:
        Real type_;
        Real gearing1_, gearing2_;
        Real spread1_, spread2_;
        Real initialValue_, accrualFactor_;
    };

    //! Ratchet_2 payoff
    class RatchetPayoff_2 : public StickyRatchetPayoff {
      public:
         RatchetPayoff_2(Real gearing1, Real gearing2,
                       Real spread1, Real spread2,
                       Real initialValue, Real accrualFactor)
        : StickyRatchetPayoff(-1,
                              gearing1, gearing2,
                              spread1, spread2,
                              initialValue, accrualFactor) {}
        //! \name Payoff interface
        //@{
        std::string name() const { return "Ratchet";}
        //@}
    };

    //! Sticky_2 payoff
    class StickyPayoff_2 : public StickyRatchetPayoff {
      public:
         StickyPayoff_2(Real gearing1, Real gearing2,
                       Real spread1, Real spread2,
                       Real initialValue, Real accrualFactor) 
        : StickyRatchetPayoff(+1,
                              gearing1, gearing2,
                              spread1, spread2,
                              initialValue, accrualFactor) {}
        //! \name Payoff interface
        //@{
        std::string name() const { return "Sticky";}
        //@}
    };
-----------------------------------------------------------------------------*/

}

#endif
