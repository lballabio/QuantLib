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

/*! \file cdo.hpp
    \brief collateralized debt obligation
*/

#ifndef quantlib_cdo_hpp
#define quantlib_cdo_hpp

#include <ql/instrument.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/termstructures/defaulttermstructure.hpp>
#include <ql/experimental/credit/lossdistribution.hpp>
#include <ql/experimental/credit/onefactorcopula.hpp>
#include <ql/time/schedule.hpp>

namespace QuantLib {

    //! collateralized debt obligation
    /*! The instrument prices a mezzanine CDO tranche with loss given
        default between attachment point \f$ D_1\f$ and detachment
        point \f$ D_2 > D_1 \f$.

        For purchased protection, the instrument value is given by the
        difference of the protection value \f$ V_1 \f$ and premium
        value \f$ V_2 \f$,

        \f[ V = V_1 - V_2. \f]

        The protection leg is priced as follows:

        - Build the probability distribution for volume of defaults
          \f$ L \f$ (before recovery) or Loss Given Default \f$ LGD =
          (1-r)\,L \f$ at times/dates \f$ t_i, i=1, ..., N\f$ (premium
          schedule times with intermediate steps)
        - Determine the expected value
          \f$ E_i = E_{t_i}\,\left[Pay(LGD)\right] \f$
          of the protection payoff \f$ Pay(LGD) \f$ at each time
          \f$ t_i\f$ where
         \f[
         Pay(L) = min (D_1, LGD) - min (D_2, LGD) = \left\{
         \begin{array}{lcl}
         \displaystyle 0 &;& LGD < D_1 \\
         \displaystyle LGD - D_1 &;& D_1 \leq LGD \leq D_2 \\
         \displaystyle D_2 - D_1 &;& LGD > D_2
         \end{array}
         \right.
         \f]
        - The protection value is then calculated as
          \f[ V_1 \:=\: \sum_{i=1}^N (E_i - E_{i-1}) \cdot  d_i \f]
          where \f$ d_i\f$ is the discount factor at time/date \f$ t_i \f$

        The premium is paid on the protected notional amount,
        initially \f$ D_2 - D_1. \f$ This notional amount is reduced
        by the expected protection payments \f$ E_i \f$ at times
        \f$ t_i, \f$ so that the premium value is calculated as

        \f[
        V_2 = m \, \cdot \sum_{i=1}^N \,(D_2 - D_1 - E_i)
                   \cdot \Delta_{i-1,i}\,d_i
        \f]

        where \f$ m \f$ is the premium rate, \f$ \Delta_{i-1, i}\f$ is
        the day count fraction between date/time \f$ t_{i-1}\f$ and
        \f$ t_i.\f$

        The construction of the portfolio loss distribution \f$ E_i
        \f$ is based on the probability bucketing algorithm described
        in

        <strong>
        John Hull and Alan White, "Valuation of a CDO and nth to default CDS
        without Monte Carlo simulation", Journal of Derivatives 12, 2, 2004
        </strong>

        The pricing algorithm allows for varying notional amounts and
        default termstructures of the underlyings.

        \todo Investigate and fix cases \f$ E_{i+1} < E_i. \f$
    */
    class CDO : public Instrument {
      public:
        /*! \param attachment  fraction of the LGD where protection starts
            \param detachment  fraction of the LGD where protection ends
            \param nominals    vector of basket nominal amounts
            \param basket      default basket represented by a vector of
                               default term structures that allow
                               computing single name default
                               probabilities depending on time
            \param copula      one-factor copula
            \param protectionSeller   sold protection if set to true, purchased
                                      otherwise
            \param premiumSchedule    schedule for premium payments
            \param premiumRate        annual premium rate, e.g. 0.05 for 5% p.a.
            \param dayCounter         day count convention for the premium rate
            \param recoveryRate       recovery rate as a fraction
            \param upfrontPremiumRate premium as a tranche notional fraction
            \param yieldTS            yield term structure handle
            \param nBuckets           number of distribution buckets
            \param integrationStep    time step for integrating over one
                                      premium period; if larger than premium
                                      period length, a single step is taken
        */
        CDO (Real attachment,
             Real detachment,
             const std::vector<Real>& nominals,
             const std::vector<Handle<DefaultProbabilityTermStructure> >& basket,
             const Handle<OneFactorCopula>& copula,
             bool protectionSeller,
             const Schedule& premiumSchedule,
             Rate premiumRate,
             const DayCounter& dayCounter,
             Rate recoveryRate,
             Rate upfrontPremiumRate,
             const Handle<YieldTermStructure>& yieldTS,
             Size nBuckets,
             const Period& integrationStep = Period(10, Years));

        Real nominal() { return nominal_; }
        Real lgd() { return lgd_; }
        Real attachment () { return attachment_; }
        Real detachment () { return detachment_; }
        std::vector<Real> nominals() { return nominals_; }
        Size size() { return basket_.size(); }

        bool isExpired () const;
        Rate fairPremium() const;
        Rate premiumValue () const;
        Rate protectionValue () const;
        Size error () const;

      private:
        void setupExpired() const;
        void performCalculations() const;
        Real expectedTrancheLoss (Date d) const;

        Real attachment_;
        Real detachment_;
        std::vector<Real> nominals_;
        std::vector<Handle<DefaultProbabilityTermStructure> > basket_;
        Handle<OneFactorCopula> copula_;
        bool protectionSeller_;

        Schedule premiumSchedule_;
        Rate premiumRate_;
        DayCounter dayCounter_;
        Rate recoveryRate_;
        Rate upfrontPremiumRate_;
        Handle<YieldTermStructure> yieldTS_;
        Size nBuckets_; // number of buckets up to detachment point
        Period integrationStep_;

        std::vector<Real> lgds_;

        Real nominal_;  // total basket volume (sum of nominals_)
        Real lgd_;      // maximum loss given default (sum of lgds_)
        Real xMax_;     // tranche detachment point (tranche_ * nominal_)
        Real xMin_;     // tranche attachment point (tranche_ * nominal_)

        mutable Size error_;

        mutable Real premiumValue_;
        mutable Real protectionValue_;
        mutable Real upfrontPremiumValue_;
    };

}

#endif
