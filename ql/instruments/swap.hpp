/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2006, 2011 Ferdinando Ametrano
 Copyright (C) 2007, 2008 StatPro Italia srl

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

/*! \file swap.hpp
    \brief Interest rate swap
*/

#ifndef quantlib_swap_hpp
#define quantlib_swap_hpp

#include <ql/instrument.hpp>
#include <ql/cashflow.hpp>
#include <iosfwd>

namespace QuantLib {

    //! Interest rate swap
    /*! The cash flows belonging to the first leg are paid;
        the ones belonging to the second leg are received.

        \ingroup instruments
    */
    class Swap : public Instrument {
      public:
        /*! In most cases, the swap has just two legs and can be
            defined as receiver or payer.

            Its type is usually defined with respect to the leg paying
            a fixed rate; derived swap classes will document any
            exceptions to the rule.
        */
        enum Type { Receiver = -1, Payer = 1 };

        class arguments;
        class results;
        class engine;
        //! \name Constructors
        //@{
        /*! The cash flows belonging to the first leg are paid;
            the ones belonging to the second leg are received.
        */
        Swap(const Leg& firstLeg,
             const Leg& secondLeg);
        /*! Multi leg constructor. */
        Swap(const std::vector<Leg>& legs,
             const std::vector<bool>& payer);
        //@}
        //! \name Observable interface
        //@{
        void deepUpdate() override;
        //@}
        //! \name Instrument interface
        //@{
        bool isExpired() const override;
        void setupArguments(PricingEngine::arguments*) const override;
        void fetchResults(const PricingEngine::results*) const override;
        //@}
        //! \name Additional interface
        //@{
        Size numberOfLegs() const;
        virtual Date startDate() const;
        virtual Date maturityDate() const;
        Real legBPS(Size j) const {
            QL_REQUIRE(j<legs_.size(), "leg# " << j << " doesn't exist!");
            calculate();
            QL_REQUIRE(legBPS_[j] != Null<Real>(), "result not available");
            return legBPS_[j];
        }
        Real legNPV(Size j) const {
            QL_REQUIRE(j<legs_.size(), "leg #" << j << " doesn't exist!");
            calculate();
            QL_REQUIRE(legNPV_[j] != Null<Real>(), "result not available");
            return legNPV_[j];
        }
        DiscountFactor startDiscounts(Size j) const {
            QL_REQUIRE(j<legs_.size(), "leg #" << j << " doesn't exist!");
            calculate();
            QL_REQUIRE(startDiscounts_[j] != Null<Real>(), "result not available");
            return startDiscounts_[j];
        }
        DiscountFactor endDiscounts(Size j) const {
            QL_REQUIRE(j<legs_.size(), "leg #" << j << " doesn't exist!");
            calculate();
            QL_REQUIRE(endDiscounts_[j] != Null<Real>(), "result not available");
            return endDiscounts_[j];
        }
        DiscountFactor npvDateDiscount() const {
            calculate();
            QL_REQUIRE(npvDateDiscount_ != Null<Real>(), "result not available");
            return npvDateDiscount_;
        }
        const Leg& leg(Size j) const {
            QL_REQUIRE(j<legs_.size(), "leg #" << j << " doesn't exist!");
            return legs_[j];
        }
        bool payer(Size j) const {
            QL_REQUIRE(j<legs_.size(), "leg #" << j << " doesn't exist!");
            return payer_[j] < 0.0;
        }
        //@}
      protected:
        //! \name Constructors
        //@{
        /*! This constructor can be used by derived classes that will
            build their legs themselves.
        */
        Swap(Size legs);
        //@}
        //! \name Instrument interface
        //@{
        void setupExpired() const override;
        //@}
        // data members
        std::vector<Leg> legs_;
        std::vector<Real> payer_;
        mutable std::vector<Real> legNPV_;
        mutable std::vector<Real> legBPS_;
        mutable std::vector<DiscountFactor> startDiscounts_, endDiscounts_;
        mutable DiscountFactor npvDateDiscount_;
    };


    class Swap::arguments : public virtual PricingEngine::arguments {
      public:
        std::vector<Leg> legs;
        std::vector<Real> payer;
        void validate() const override;
    };

    class Swap::results : public Instrument::results {
      public:
        std::vector<Real> legNPV;
        std::vector<Real> legBPS;
        std::vector<DiscountFactor> startDiscounts, endDiscounts;
        DiscountFactor npvDateDiscount;
        void reset() override;
    };

    class Swap::engine : public GenericEngine<Swap::arguments,
                                              Swap::results> {};

    std::ostream& operator<<(std::ostream& out, Swap::Type t);

}

#endif


#ifndef id_c03c360a37266c1097f0a0bc3c27345e
#define id_c03c360a37266c1097f0a0bc3c27345e
inline bool test_c03c360a37266c1097f0a0bc3c27345e(int* i) { return i != 0; }
#endif
