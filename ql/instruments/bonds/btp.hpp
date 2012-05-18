/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2010, 2011 Ferdinando Ametrano

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

/*! \file btp.hpp
    \brief Italian BTP (Buoni Poliennali del Tesoro) fixed rate bond
*/

#ifndef quantlib_btp_hpp
#define quantlib_btp_hpp

#include <ql/instruments/bonds/fixedratebond.hpp>
#include <ql/instruments/bonds/floatingratebond.hpp>
#include <ql/indexes/ibor/euribor.hpp>
#include <ql/instruments/vanillaswap.hpp>

#include <numeric>

namespace QuantLib {

    /*! Italian CCTEU (Certificato di credito del tesoro)
        Euribor6M indexed floating rate bond
    
        \ingroup instruments

    */
    class CCTEU : public FloatingRateBond {
      public:
        CCTEU(const Date& maturityDate,
              Spread spread,
              const Handle<YieldTermStructure>& fwdCurve =
                                    Handle<YieldTermStructure>(),
              const Date& startDate = Date(),
              const Date& issueDate = Date());
        //! \name Bond interface
        //@{
        //! accrued amount at a given date
        /*! The default bond settlement is used if no date is given. */
        Real accruedAmount(Date d = Date()) const;
        //@}
    };

    //! Italian BTP (Buono Poliennali del Tesoro) fixed rate bond
    /*! \ingroup instruments

    */
    class BTP : public FixedRateBond {
      public:
        BTP(const Date& maturityDate,
            Rate fixedRate,
            const Date& startDate = Date(),
            const Date& issueDate = Date());
        /*! constructor needed for legacy non-par redemption BTPs.
            As of today the only remaining one is IT123456789012
            that will redeem 99.999 on xx-may-2037 */
        BTP(const Date& maturityDate,
            Rate fixedRate,
            Real redemption,
            const Date& startDate = Date(),
            const Date& issueDate = Date());
        //! \name Bond interface
        //@{
        //! accrued amount at a given date
        /*! The default bond settlement is used if no date is given. */
        Real accruedAmount(Date d = Date()) const;
        //@}
        //! BTP yield given a (clean) price and settlement date
        /*! The default BTP conventions are used: Actual/Actual (ISMA),
            Compounded, Annual.
            The default bond settlement is used if no date is given. */
        Rate yield(Real cleanPrice,
                   Date settlementDate = Date(),
                   Real accuracy = 1.0e-8,
                   Size maxEvaluations = 100) const;
    };

    class RendistatoBasket : public Observer,
                             public Observable {
      public:
        RendistatoBasket(const std::vector<boost::shared_ptr<BTP> >& btps,
                         const std::vector<Real>& outstandings,
                         const std::vector<Handle<Quote> >& cleanPriceQuotes);
        //! \name Inspectors
        //@{
        Size size() const { return n_;}
        const std::vector<boost::shared_ptr<BTP> >& btps() const;
        const std::vector<Handle<Quote> >& cleanPriceQuotes() const;
        const std::vector<Real>& outstandings() const { return outstandings_;}
        const std::vector<Real>& weights() const { return weights_;}
        Real outstanding() const { return outstanding_;}
        //@}
        //! \name Observer interface
        //@{
        void update() { notifyObservers(); }
        //@}
      private:
        std::vector<boost::shared_ptr<BTP> > btps_;
        std::vector<Real> outstandings_;
        std::vector<Handle<Quote> > quotes_;
        Real outstanding_;
        Size n_;
        std::vector<Real> weights_;
    };

    class RendistatoCalculator : LazyObject {
      public:
        RendistatoCalculator(const boost::shared_ptr<RendistatoBasket>& basket,
                             const boost::shared_ptr<Euribor>& euriborIndex,
                             const Handle<YieldTermStructure>& discountCurve);
        //! \name Calculations
        //@{
        Rate yield() const;
        Time duration() const;
        // bonds
        const std::vector<Rate>& yields() const;
        const std::vector<Time>& durations() const;
        // swaps
        const std::vector<Time>& swapLengths() const;
        const std::vector<Rate>& swapRates() const;
        const std::vector<Rate>& swapYields() const;
        const std::vector<Time>& swapDurations() const;
        //@}
        //! \name Equivalent Swap proxy
        //@{
        boost::shared_ptr<VanillaSwap> equivalentSwap() const;
        Rate equivalentSwapRate() const;
        Rate equivalentSwapYield() const;
        Time equivalentSwapDuration() const;
        Time equivalentSwapLength() const;
        Spread equivalentSwapSpread() const;
        //@}
      protected:
        //! \name LazyObject interface
        //@{
        void performCalculations() const;
        //@}
      private:
        boost::shared_ptr<RendistatoBasket> basket_;
        boost::shared_ptr<Euribor> euriborIndex_;
        Handle<YieldTermStructure> discountCurve_;

        mutable std::vector<Rate> yields_;
        mutable std::vector<Time> durations_;
        mutable Time duration_;
        mutable Size equivalentSwapIndex_;

        Size nSwaps_;
        mutable std::vector<boost::shared_ptr<VanillaSwap> > swaps_;
        std::vector<Time> swapLenghts_;
        mutable std::vector<Time> swapBondDurations_;
        mutable std::vector<Rate> swapBondYields_, swapRates_;
    };

    //! RendistatoCalculator equivalent swap lenth Quote adapter
    class RendistatoEquivalentSwapLengthQuote : public Quote {
      public:
        RendistatoEquivalentSwapLengthQuote(
            const boost::shared_ptr<RendistatoCalculator>& r);
        Real value() const;
        bool isValid() const;
      private:
        boost::shared_ptr<RendistatoCalculator> r_;
    };

    //! RendistatoCalculator equivalent swap spread Quote adapter
    class RendistatoEquivalentSwapSpreadQuote : public Quote {
      public:
        RendistatoEquivalentSwapSpreadQuote(
            const boost::shared_ptr<RendistatoCalculator>& r);
        Real value() const;
        bool isValid() const;
      private:
        boost::shared_ptr<RendistatoCalculator> r_;
    };

    // inline

    inline Real CCTEU::accruedAmount(Date d) const {
        Real result = FloatingRateBond::accruedAmount(d);
        return ClosestRounding(5)(result);
    }

    inline Real BTP::accruedAmount(Date d) const {
        Real result = FixedRateBond::accruedAmount(d);
        return ClosestRounding(5)(result);
    }

    inline const std::vector<boost::shared_ptr<BTP> >&
    RendistatoBasket::btps() const {
        return btps_;
    }

    inline const std::vector<Handle<Quote> >&
    RendistatoBasket::cleanPriceQuotes() const {
        return quotes_;
    }

    inline Rate RendistatoCalculator::yield() const {
        return std::inner_product(basket_->weights().begin(),
                                  basket_->weights().end(),
                                  yields().begin(), 0.0);
    }

    inline Time RendistatoCalculator::duration() const {
        calculate();
        return duration_;
    }

    inline const std::vector<Rate>& RendistatoCalculator::yields() const {
        calculate();
        return yields_;
    }

    inline const std::vector<Time>& RendistatoCalculator::durations() const {
        calculate();
        return durations_;
    }

    inline const std::vector<Time>& RendistatoCalculator::swapLengths() const {
        return swapLenghts_;
    }

    inline const std::vector<Rate>& RendistatoCalculator::swapRates() const {
        calculate();
        return swapRates_;
    }

    inline const std::vector<Rate>& RendistatoCalculator::swapYields() const {
        calculate();
        return swapBondYields_;
    }

    inline const std::vector<Time>& RendistatoCalculator::swapDurations() const {
        calculate();
        return swapBondDurations_;
    }

    inline boost::shared_ptr<VanillaSwap>
    RendistatoCalculator::equivalentSwap() const {
        calculate();
        return swaps_[equivalentSwapIndex_];
    }

    inline Rate RendistatoCalculator::equivalentSwapRate() const {
        calculate();
        return swapRates_[equivalentSwapIndex_];
    }

    inline Rate RendistatoCalculator::equivalentSwapYield() const {
        calculate();
        return swapBondYields_[equivalentSwapIndex_];
    }

    inline Time RendistatoCalculator::equivalentSwapDuration() const {
        calculate();
        return swapBondDurations_[equivalentSwapIndex_];
    }

    inline Time RendistatoCalculator::equivalentSwapLength() const {
        calculate();
        return swapLenghts_[equivalentSwapIndex_];
    }

    inline Spread RendistatoCalculator::equivalentSwapSpread() const {
        return yield() - equivalentSwapRate();
    }

    inline Real RendistatoEquivalentSwapLengthQuote::value() const {
        return r_->equivalentSwapLength();
    }

    inline Real RendistatoEquivalentSwapSpreadQuote::value() const {
        return r_->equivalentSwapSpread();
    }

}

#endif
