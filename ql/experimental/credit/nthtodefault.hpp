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
        NthToDefault(
                Size n,
                const std::vector<Handle<DefaultProbabilityTermStructure> >&
                                                                probabilities,
                Real recoveryRate,
                const Handle<OneFactorCopula>& copula,
                Protection::Side side,
                Real nominal,
                const Schedule& premiumSchedule,
                Rate premiumRate,
                const DayCounter& dayCounter,
                bool settlePremiumAccrual,
                const Handle<YieldTermStructure>& yieldTS,
                const Period& integrationStepSize,
                boost::shared_ptr<Claim> claim = boost::shared_ptr<Claim>());

        bool isExpired() const;

        Rate fairPremium() const;

        // inspectors
        Rate premium() const { return premiumRate_; }
        Real nominal() const { return nominal_; }
        DayCounter dayCounter() const { return dayCounter_; }
        Protection::Side side() const { return side_; }
        Size rank() const { return n_; }
        Size basketSize() const { return probabilities_.size(); }

      private:
        Probability defaultProbability(const Date& d) const;

        void setupExpired() const;
        void performCalculations() const;

        Size n_;
        std::vector<Handle<DefaultProbabilityTermStructure> > probabilities_;
        Real recoveryRate_;
        Handle<OneFactorCopula> copula_;
        Protection::Side side_;
        Real nominal_;
        Schedule premiumSchedule_;
        Rate premiumRate_;
        DayCounter dayCounter_;
        bool settlePremiumAccrual_;
        Handle<YieldTermStructure> yieldTS_;
        Period integrationStepSize_;
        boost::shared_ptr<Claim> claim_;

        Leg premiumLeg_;

        // results
        mutable Rate fairPremium_;
    };

}

#endif
