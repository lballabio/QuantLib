/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Marco Bianchetti
 Copyright (C) 2006, 2007 Giorgio Facchinetti

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

/*! \file cmsmarket.hpp
    \brief set of CMS quotes
*/

#ifndef quantlib_cms_market_h
#define quantlib_cms_market_h

#include <ql/termstructures/volatility/swaption/swaptionvolmatrix.hpp>

namespace QuantLib {

    class CmsCouponPricer;
    class Swap;
    class SwapIndex;
    class IborIndex;
    class YieldTermStructure;

    //! set of CMS quotes
    class CmsMarket: public LazyObject{
      public:

        CmsMarket(
            const std::vector<Period>& expiries,
            const std::vector< boost::shared_ptr<SwapIndex> >& swapIndices,
            const boost::shared_ptr<IborIndex>& iborIndex,
            const std::vector<std::vector<Handle<Quote> > >& bidAskSpreads,
            const std::vector< boost::shared_ptr<CmsCouponPricer> >& pricers,
            const Handle<YieldTermStructure>& yieldTermStructure);

        //! \name LazyObject interface
        //@{
        void update() { LazyObject::update();}
        //@}

        // call during calibration procedure
        void reprice(const Handle<SwaptionVolatilityStructure>& volStructure,
                     Real meanReversion);

        //inspectors ...
        const std::vector<Period>& swapTenors() const {return swapTenors_;}
        Matrix meanReversions(){return meanReversions_;};
        Matrix impliedCmsSpreads(){return modelCmsSpreads_;};
        Matrix spreadErrors(){return spreadErrors_;};
        Matrix browse() const;

        //cms market calibration methods (they haven't Lazyness behaviour)
        Real weightedError(const Matrix& weights);
        Real weightedPriceError(const Matrix& weights);
        Real weightedForwardPriceError(const Matrix& weights);
        Disposable<Array> weightedErrors(const Matrix& weights);
        Disposable<Array> weightedPriceErrors(const Matrix& weights);
        Disposable<Array> weightedForwardPriceErrors(const Matrix& weights);

      private:
        void performCalculations() const;
        void registerWithMarketData();
        void createForwardStartingCms();
        void priceForwardStartingCms() const;
        void priceSpotFromForwardStartingCms() const;
        Real weightedMean(const Matrix& var, const Matrix& weights);
        Disposable<Array> weightedMeans(const Matrix& var, const Matrix& weights);

        std::vector<Period> expiries_;
        std::vector<Period> swapTenors_;
        Size nExercise_;
        Size nSwapTenors_;

        // market bid spreads
        mutable Matrix bids_;
        // market ask spreads
        mutable Matrix asks_;
        // market mid spreads
        mutable Matrix mids_;
        // Implied spreads to model prices
        mutable Matrix modelCmsSpreads_;
        // Differences between implied and mid spreads
        mutable Matrix spreadErrors_;

        // prices of constant maturity swaps with spread = 0
        mutable Matrix prices_;
        // market prices of Cms Leg corrisponding to bid spreads
        mutable Matrix marketBidCmsLegValues_;
        // market prices of Cms Leg corrisponding to ask spreads
        mutable Matrix marketAskCmsLegValues_;
        // market prices of Cms Leg corrisponding to mid spreads
        mutable Matrix marketMidCmsLegValues_;
        // model prices of Cms Leg corrisponding to mid spreads
        mutable Matrix modelCmsLegValues_;
        // Differences between modelCmsLegValue and marketMidCmsLegValue_
        mutable Matrix priceErrors_;

        mutable Matrix swapFloatingLegsPrices_,swapFloatingLegsBps_;

        // market prices of Forward Cms Leg corrisponding to bid spreads
        mutable Matrix marketBidForwardCmsLegValues_;
        // market prices of Forward Cms Leg corrisponding to ask spreads
        mutable Matrix marketAskForwardCmsLegValues_;
        // market prices of Forward Cms Leg corrisponding to mid spreads
        mutable Matrix marketMidForwardCmsLegValues_;
        // model prices of Forward Cms Leg corrisponding to mid spreads
        mutable Matrix modelForwardCmsLegValues_;
        // Differences between modelForwardCmsLegValues and marketMidCmsLegValues
        mutable Matrix forwardPriceErrors_;

        mutable Matrix meanReversions_;
        std::vector< boost::shared_ptr<CmsCouponPricer> > pricers_;
        std::vector< boost::shared_ptr<SwapIndex> > swapIndices_;
        boost::shared_ptr<IborIndex> iborIndex_;
        const std::vector<std::vector<Handle<Quote> > > bidAskSpreads_;
        std::vector< std::vector< boost::shared_ptr<Swap> > > swaps_;
        std::vector< std::vector< boost::shared_ptr<Swap> > > forwardSwaps_;
        Handle<YieldTermStructure> yieldTermStructure_;
     };

}

#endif
