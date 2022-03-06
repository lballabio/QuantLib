/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Roland Lichters

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

/*! \file nthtodefault.hpp
    \brief N-th to default swap
*/

#ifndef quantlib_nth_to_default_hpp
#define quantlib_nth_to_default_hpp

#include <ql/instrument.hpp>
#include <ql/cashflow.hpp>
#include <ql/default.hpp>
#include <ql/termstructures/defaulttermstructure.hpp>
#include <ql/experimental/credit/onefactorcopula.hpp>
#include <ql/time/schedule.hpp>

namespace QuantLib {

    class YieldTermStructure;
    class Claim;
    class Basket;

    //--------------------------------------------------------------------------
    //! N-th to default swap
    /*! A NTD instrument exchanges protection against the nth default
        in a basket of underlying credits for premium payments based
        on the protected notional amount.

        The pricing is analogous to the pricing of a CDS instrument
        which represents protection against default of a single
        underlying credit.  The only difference is the calculation of
        the probability of default.  In the CDS case, it is the
        probabilty of single name default; in the NTD case the
        probability of at least N defaults in the portfolio of
        underlying credits.

        This probability is computed using the algorithm in
        John Hull and Alan White, "Valuation of a CDO and nth to
        default CDS without Monte Carlo simulation", Journal of
        Derivatives 12, 2, 2004.

        The algorithm allows for varying probability of default across
        the basket. Otherwise, for identical probabilities of default,
        the probability of n defaults is given by the binomial
        distribution.

        Default correlation is modeled using a one-factor Gaussian copula
        approach.

        The class is tested against data in Hull-White (see reference
        above.)
    */
    class NthToDefault : public Instrument {
      public:
        class arguments;
        class results;
        class engine;

        //! This product is 'digital'; the basket might be tranched but this is 
        //  not relevant to it.
        NthToDefault(const ext::shared_ptr<Basket>& basket,
                Size n,
                Protection::Side side,
                const Schedule& premiumSchedule,
                Rate upfrontRate,
                Rate premiumRate,
                const DayCounter& dayCounter,
                Real nominal,
                bool settlePremiumAccrual);

        bool isExpired() const override;

        // inspectors
        Rate premium() const { return premiumRate_; }
        Real nominal() const { return nominal_; }
        DayCounter dayCounter() const { return dayCounter_; }
        Protection::Side side() const { return side_; }
        Size rank() const { return n_; }
        Size basketSize() const;

        const Date& maturity() const {return premiumSchedule_.endDate();}//???

        const ext::shared_ptr<Basket>& basket() const {return basket_;}

        // results
        Rate fairPremium() const;
        Real premiumLegNPV() const;
        Real protectionLegNPV() const;
        Real errorEstimate() const;

        void setupArguments(PricingEngine::arguments*) const override;
        void fetchResults(const PricingEngine::results*) const override;

      private:
        void setupExpired() const override;

        ext::shared_ptr<Basket> basket_;
        Size n_;
        Protection::Side side_;
        Real nominal_;
        Schedule premiumSchedule_;
        Rate premiumRate_;
        Rate upfrontRate_;
        DayCounter dayCounter_;
        bool settlePremiumAccrual_;

        Leg premiumLeg_;/////////////////// LEG AND SCHEDULE BOTH MEMBERS..... REVISE THIS!

        // results
        mutable Rate premiumValue_;
        mutable Real protectionValue_;
        mutable Real upfrontPremiumValue_;
        mutable Real fairPremium_;
        mutable Real errorEstimate_;
    };



    class NthToDefault::arguments : public virtual PricingEngine::arguments {
    public:
        arguments() : side(Protection::Side(-1)),
                      premiumRate(Null<Real>()),
                      upfrontRate(Null<Real>()) {}
        void validate() const override;

        ext::shared_ptr<Basket> basket;
        Protection::Side side;
        Leg premiumLeg;

        Size ntdOrder;
        bool settlePremiumAccrual;
        Real notional;// ALL NAMES WITH THE SAME WEIGHT, NOTIONAL IS NOT MAPPED TO THE BASKET HERE, this does not have to be that way, its perfectly possible to have irreg notionals...
        Real premiumRate;
        Rate upfrontRate;
    };

    class NthToDefault::results : public Instrument::results {
    public:
      void reset() override;
      Real premiumValue;
      Real protectionValue;
      Real upfrontPremiumValue;
      Real fairPremium;
      Real errorEstimate;
    };

    //! NTD base engine
    class NthToDefault::engine : 
        public GenericEngine<NthToDefault::arguments, 
                             NthToDefault::results> { };

}

#endif


#ifndef id_7590c2d0ec0d5ccd32a985de34a1ada7
#define id_7590c2d0ec0d5ccd32a985de34a1ada7
inline bool test_7590c2d0ec0d5ccd32a985de34a1ada7(int* i) { return i != 0; }
#endif
