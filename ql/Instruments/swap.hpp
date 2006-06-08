/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Ferdinando Ametrano
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file swap.hpp
    \brief Interest rate swap
*/

#ifndef quantlib_swap_hpp
#define quantlib_swap_hpp

#include <ql/instrument.hpp>
#include <ql/yieldtermstructure.hpp>
#include <ql/cashflow.hpp>

namespace QuantLib {

    //! Interest rate swap
    /*! The cash flows belonging to the first leg are paid;
        the ones belonging to the second leg are received.

        \ingroup instruments
    */
    class Swap : public Instrument {
      typedef std::vector<boost::shared_ptr<CashFlow> > SwapLeg;
      public:
        #ifndef QL_DISABLE_DEPRECATED
        //! \deprecated
        Swap(const std::vector<boost::shared_ptr<CashFlow> >& firstLeg,
             const std::vector<boost::shared_ptr<CashFlow> >& secondLeg,
             const Handle<YieldTermStructure>& termStructure);
        #endif
        /*! The cash flows belonging to the first leg are paid;
            the ones belonging to the second leg are received.
        */
        Swap(const Handle<YieldTermStructure>& termStructure,
             const SwapLeg& firstLeg,
             const SwapLeg& secondLeg);
        /*! Multi leg constructor. */
        Swap(const Handle<YieldTermStructure>& termStructure,
             const std::vector<SwapLeg>& legs,
             const std::vector<bool>& payer);
        //! \name Instrument interface
        //@{
        bool isExpired() const;
        //@}
        //! \name Additional interface
        //@{
        Date startDate() const;
        Date maturity() const;
        #ifndef QL_DISABLE_DEPRECATED
        //! \deprecated use legBPS(0) instead
        Real firstLegBPS() const {return legBPS(0); }
        //! \deprecated use legBPS(1) instead
        Real secondLegBPS() const {return legBPS(1); }
        #endif
        Real legBPS(Size j) const {
            QL_REQUIRE(j<legs_.size(),
                "legBPS: leg# " << j << " doesn't exist!");
            calculate();
            return legBPS_[j];
        }
        const SwapLeg& leg(Size j) const {
            QL_REQUIRE(j<legs_.size(),
                "leg: leg# " << j << " doesn't exist!");
            return legs_[j];
        }
        //@}
      protected:
        // methods
        void setupExpired() const;
        void performCalculations() const;
        // data members
        Handle<YieldTermStructure> termStructure_;
        std::vector<SwapLeg> legs_;
        std::vector<Real> payer_;
        mutable std::vector<Real> legBPS_;
    };
}


#endif
