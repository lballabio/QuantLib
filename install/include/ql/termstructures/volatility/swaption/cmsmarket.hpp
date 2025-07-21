/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Ferdinando Ametrano
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

#include <ql/cashflows/conundrumpricer.hpp>
#include <ql/math/matrix.hpp>
#include <ql/quote.hpp>

namespace QuantLib {

    class CmsCouponPricer;
    class Swap;
    class SwapIndex;
    class IborIndex;
    class YieldTermStructure;

    //! set of CMS quotes
    class CmsMarket: public LazyObject{
      public:
        CmsMarket(std::vector<Period> swapLengths,
                  std::vector<ext::shared_ptr<SwapIndex> > swapIndexes,
                  ext::shared_ptr<IborIndex> iborIndex,
                  const std::vector<std::vector<Handle<Quote> > >& bidAskSpreads,
                  const std::vector<ext::shared_ptr<CmsCouponPricer> >& pricers,
                  Handle<YieldTermStructure> discountingTS);
        //! \name LazyObject interface
        //@{
        void update() override { LazyObject::update(); }
        //@}
        // called during calibration procedure
        void reprice(const Handle<SwaptionVolatilityStructure>& volStructure,
                     Real meanReversion);
        // inspectors ...
        const std::vector<Period>& swapTenors() const { return swapTenors_;}
        const std::vector<Period>& swapLengths() const { return swapLengths_;}
        const Matrix& impliedCmsSpreads() { return mdlSpreads_; }
        const Matrix& spreadErrors() { return errSpreads_; }
        Matrix browse() const;

        // cms market calibration methods (they haven't Lazy behaviour)
        Real weightedSpreadError(const Matrix& weights);
        Real weightedSpotNpvError(const Matrix& weights);
        Real weightedFwdNpvError(const Matrix& weights);
        Array weightedSpreadErrors(const Matrix& weights);
        Array weightedSpotNpvErrors(const Matrix& weights);
        Array weightedFwdNpvErrors(const Matrix& weights);

      private:
        void performCalculations() const override;
        Real weightedMean(const Matrix& var, const Matrix& weights) const;
        Array weightedMeans(const Matrix& var, const Matrix& weights) const;

        std::vector<Period> swapLengths_;
        std::vector<ext::shared_ptr<SwapIndex> > swapIndexes_;
        ext::shared_ptr<IborIndex> iborIndex_;
        std::vector<std::vector<Handle<Quote> > > bidAskSpreads_;
        std::vector<ext::shared_ptr<CmsCouponPricer> > pricers_;
        Handle<YieldTermStructure> discTS_;

        Size nExercise_;
        Size nSwapIndexes_;
        std::vector<Period> swapTenors_;
        mutable Matrix spotFloatLegNPV_, spotFloatLegBPS_;

        // market spreads
        mutable Matrix mktBidSpreads_, mktAskSpreads_, mktSpreads_;
        // model (mid) spreads
        mutable Matrix mdlSpreads_;
        // differences between market and model mid spreads
        mutable Matrix errSpreads_;

        // market mid prices of spot starting Cms Leg
        mutable Matrix mktSpotCmsLegNPV_;
        // model mid prices of spot starting Cms Leg
        mutable Matrix mdlSpotCmsLegNPV_;
        // Differences between mdlSpotCmsLegNPV_ and mktSpotCmsLegNPV_
        mutable Matrix errSpotCmsLegNPV_;

        // market mid prices of forward starting Cms Leg
        mutable Matrix mktFwdCmsLegNPV_;
        // model mid prices of forward starting Cms Leg
        mutable Matrix mdlFwdCmsLegNPV_;
        // Differences between mdlFwdCmsLegNPV_ and mktFwdCmsLegNPV_
        mutable Matrix errFwdCmsLegNPV_;

        std::vector<std::vector<ext::shared_ptr<Swap> > > spotSwaps_;
        std::vector<std::vector<ext::shared_ptr<Swap> > > fwdSwaps_;

     };

}

#endif
