/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2010, 2011 Chris Kenyon

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

/*! \file cpicapfloortermpricesurface.hpp
    \brief cpi inflation cap and floor term price structure.
*/

#ifndef quantlib_cpi_capfloor_term_price_surface_hpp
#define quantlib_cpi_capfloor_term_price_surface_hpp

#include <ql/termstructures/inflationtermstructure.hpp>
#include <ql/math/interpolation.hpp>
#include <ql/math/interpolations/interpolation2d.hpp>
#include <ql/experimental/inflation/polynomial2Dspline.hpp>
#include <ql/indexes/inflationindex.hpp>


namespace QuantLib {

    //! Provides cpi cap/floor prices by interpolation and put/call parity (not cap/floor/swap* parity).
    /*! The inflation index MUST contain a ZeroInflationTermStructure as
        this is used to create ATM.  Unlike YoY price surfaces we
        assume that 1) an ATM ZeroInflationTermStructure is available
        and 2) that it is safe to use it.  This is supported by the
        fact that no stripping is required for CPI cap/floors as they
        only give one flow.

        cpi cap/floors have a single (one) flow (unlike nominal
        caps) because they observe cumulative inflation up to
        their maturity.  Options are on CPI(T)/CPI(0) but strikes
        are quoted for yearly average inflation, so require transformation
        via (1+quote)^T to obtain actual strikes.  These are consistent
        with ZCIIS quoting conventions.

        The observationLag is that for the referenced instrument prices.
        Strikes are as-quoted not as-used.
    */
    class CPICapFloorTermPriceSurface : public InflationTermStructure {
      public:
        CPICapFloorTermPriceSurface(
            Real nominal,
            Real baseRate, // avoids an uncontrolled crash if index has no TS
            const Period& observationLag,
            const Calendar& cal, // calendar in index may not be useful
            const BusinessDayConvention& bdc,
            const DayCounter& dc,
            const std::shared_ptr<ZeroInflationIndex>& zii,
            CPI::InterpolationType interpolationType,
            Handle<YieldTermStructure> yts,
            const std::vector<Rate>& cStrikes,
            const std::vector<Rate>& fStrikes,
            const std::vector<Period>& cfMaturities,
            const Matrix& cPrice,
            const Matrix& fPrice);

        //! \name InflationTermStructure interface
        //@{
        Period observationLag() const override;
        Date baseDate() const override;
        //@}

        //! inspectors
        /*! \note you don't know if price() is a cap or a floor
                  without checking the ZeroInflation ATM level.
        */
        //@{
        virtual Real nominal() const;
        virtual BusinessDayConvention businessDayConvention() const;
        std::shared_ptr<ZeroInflationIndex> zeroInflationIndex() const { return zii_; }
        //@}

        Rate atmRate(Date maturity) const;

        //! \warning you MUST remind the compiler in any descendants with the using:: mechanism
        //!          because you overload the names
        //! remember that the strikes use the quoting convention
        //@{
        virtual Real price(const Period &d, Rate k) const;
        virtual Real capPrice(const Period &d, Rate k) const;
        virtual Real floorPrice(const Period &d, Rate k) const;
        virtual Real price(const Date &d, Rate k) const = 0;
        virtual Real capPrice(const Date &d, Rate k) const = 0;
        virtual Real floorPrice(const Date &d, Rate k) const = 0;
        //@}

        virtual std::vector<Rate> strikes() const {return cfStrikes_;}
        virtual std::vector<Rate> capStrikes() const {return cStrikes_;}
        virtual std::vector<Rate> floorStrikes() const {return fStrikes_;}
        virtual std::vector<Period> maturities() const {return cfMaturities_;}

        virtual const Matrix &capPrices() const { return cPrice_; }
        virtual const Matrix &floorPrices() const { return fPrice_; }

        virtual Rate minStrike() const {return cfStrikes_.front();};
        virtual Rate maxStrike() const {return cfStrikes_.back();};
        virtual Date minDate() const {return referenceDate()+cfMaturities_.front();}// \TODO deal with index interpolation
        Date maxDate() const override { return referenceDate() + cfMaturities_.back(); }
        //@}

        virtual Date cpiOptionDateFromTenor(const Period& p) const;

      protected:
        virtual bool checkStrike(Rate K) {
            return ( minStrike() <= K && K <= maxStrike() );
        }
        virtual bool checkMaturity(const Date& d) {
            return ( minDate() <= d && d <= maxDate() );
        }

        std::shared_ptr<ZeroInflationIndex> zii_;
        CPI::InterpolationType interpolationType_;
        Handle<YieldTermStructure> nominalTS_;
        // data
        std::vector<Rate> cStrikes_;
        std::vector<Rate> fStrikes_;
        std::vector<Period> cfMaturities_;
        mutable std::vector<Real> cfMaturityTimes_;
        Matrix cPrice_;
        Matrix fPrice_;
        // constructed
        mutable std::vector<Rate> cfStrikes_;
      private:
        Real nominal_;
        BusinessDayConvention bdc_;
    };



    template<class Interpolator2D>
    class InterpolatedCPICapFloorTermPriceSurface
        : public CPICapFloorTermPriceSurface {
            public:
            InterpolatedCPICapFloorTermPriceSurface(Real nominal,
                                                Rate startRate,
                                                const Period &observationLag,
                                                const Calendar &cal,
                                                const BusinessDayConvention &bdc,
                                                const DayCounter &dc,
                                                const std::shared_ptr<ZeroInflationIndex>& zii,
                                                CPI::InterpolationType interpolationType,
                                                const Handle<YieldTermStructure>& yts,
                                                const std::vector<Rate> &cStrikes,
                                                const std::vector<Rate> &fStrikes,
                                                const std::vector<Period> &cfMaturities,
                                                const Matrix &cPrice,
                                                const Matrix &fPrice,
                                                const Interpolator2D &interpolator2d = Interpolator2D());

            //! \name LazyObject interface
            //@{
            void performCalculations() const;
            //@}

            //! required to allow for method hiding
            //@{
            using CPICapFloorTermPriceSurface::price;
            using CPICapFloorTermPriceSurface::capPrice;
            using CPICapFloorTermPriceSurface::floorPrice;
            //@}

            //! remember that the strikes use the quoting convention
            //@{
            Real price(const Date& d, Rate k) const override;
            Real capPrice(const Date& d, Rate k) const override;
            Real floorPrice(const Date& d, Rate k) const override;
            //@}

        protected:

            // data for surfaces and curve
            mutable Matrix cPriceB_;
            mutable Matrix fPriceB_;
            mutable Interpolation2D capPrice_, floorPrice_;
            mutable Interpolator2D interpolator2d_;
    };


    // template definitions, for some reason DOXYGEN doesn't like the first one

    #ifndef __DOXYGEN__

    template<class Interpolator2D>
    InterpolatedCPICapFloorTermPriceSurface<Interpolator2D>::
    InterpolatedCPICapFloorTermPriceSurface(Real nominal,
                                            Rate startRate,
                                            const Period &observationLag,
                                            const Calendar &cal,
                                            const BusinessDayConvention &bdc,
                                            const DayCounter &dc,
                                            const std::shared_ptr<ZeroInflationIndex>& zii,
                                            CPI::InterpolationType interpolationType,
                                            const Handle<YieldTermStructure>& yts,
                                            const std::vector<Rate> &cStrikes,
                                            const std::vector<Rate> &fStrikes,
                                            const std::vector<Period> &cfMaturities,
                                            const Matrix &cPrice,
                                            const Matrix &fPrice,
                                            const Interpolator2D &interpolator2d)
    : CPICapFloorTermPriceSurface(nominal, startRate, observationLag, cal, bdc, dc,
                                  zii, interpolationType, yts, cStrikes, fStrikes,
                                  cfMaturities, cPrice, fPrice),
      interpolator2d_(interpolator2d) {
        performCalculations();
    }

    #endif

    //! set up the interpolations for capPrice_ and floorPrice_
    //! since we know ATM, and we have single flows,
    //! we can use put/call parity to extend the surfaces
    //! across all strikes
    template<class I2D>
    void InterpolatedCPICapFloorTermPriceSurface<I2D>::
    performCalculations() const {

        cPriceB_ =
            Matrix(cfStrikes_.size(), cfMaturities_.size(), Null<Real>());
        fPriceB_ =
            Matrix(cfStrikes_.size(), cfMaturities_.size(), Null<Real>());

        Handle<YieldTermStructure> yts = nominalTS_;
        QL_REQUIRE(!yts.empty(), "Yts is empty!!!");

        for (Size j = 0; j < cfMaturities_.size(); ++j) {
            Period mat = cfMaturities_[j];
            Real df = yts->discount(cpiOptionDateFromTenor(mat));
            Real atm_quote = atmRate(cpiOptionDateFromTenor(mat));
            Real atm = std::pow(1.0 + atm_quote, mat.length());
            Real S = atm * df;
            for (Size i = 0; i < cfStrikes_.size(); ++i) {
                Real K_quote = cfStrikes_[i];
                Real K = std::pow(1.0 + K_quote, mat.length());
                auto close = [k = cfStrikes_[i]](Real x){ return close_enough(x, k); };
                Size indF = std::find_if(fStrikes_.begin(), fStrikes_.end(), close) - fStrikes_.begin();
                Size indC = std::find_if(cStrikes_.begin(), cStrikes_.end(), close) - cStrikes_.begin();
                bool isFloorStrike = indF < fStrikes_.size();
                bool isCapStrike = indC < cStrikes_.size();
                if (isFloorStrike) {
                    fPriceB_[i][j] = fPrice_[indF][j];
                    if (!isCapStrike) {
                        cPriceB_[i][j] = fPrice_[indF][j] + S - K * df;
                    }
                }
                if (isCapStrike) {
                    cPriceB_[i][j] = cPrice_[indC][j];
                    if (!isFloorStrike) {
                        fPriceB_[i][j] = cPrice_[indC][j] + K * df - S;
                    }
                }
            }
        }

        // check that all cells are filled
        for (Size i = 0; i < cPriceB_.rows(); ++i) {
            for (Size j = 0; j < cPriceB_.columns(); ++j) {
                QL_REQUIRE(cPriceB_[i][j] != Null<Real>(),
                           "InterpolatedCPICapFloorTermPriceSurface: did not "
                           "fill call price matrix at ("
                               << i << "," << j << "), this is unexpected");
                QL_REQUIRE(fPriceB_[i][j] != Null<Real>(),
                           "InterpolatedCPICapFloorTermPriceSurface: did not "
                           "fill floor price matrix at ("
                               << i << "," << j << "), this is unexpected");
            }
        }

        cfMaturityTimes_.clear();
        for (Size i=0; i<cfMaturities_.size();i++) {
            cfMaturityTimes_.push_back(timeFromReference(cpiOptionDateFromTenor(cfMaturities_[i])));
        }

        capPrice_ = interpolator2d_.interpolate(cfMaturityTimes_.begin(),cfMaturityTimes_.end(),
                                                cfStrikes_.begin(), cfStrikes_.end(),
                                                cPriceB_
                                                );
        capPrice_.enableExtrapolation();

        floorPrice_ = interpolator2d_.interpolate(cfMaturityTimes_.begin(),cfMaturityTimes_.end(),
                                                  cfStrikes_.begin(), cfStrikes_.end(),
                                                  fPriceB_
                                                  );
        floorPrice_.enableExtrapolation();
    }

    //! remember that the strike uses the quoting convention
    template<class I2D>
    Real InterpolatedCPICapFloorTermPriceSurface<I2D>::
    price(const Date &d, Rate k) const {

        Rate atm = atmRate(d);
        return k > atm ? capPrice(d,k): floorPrice(d,k);
    }

    //! remember that the strike uses the quoting convention
    template<class I2D>
    Real InterpolatedCPICapFloorTermPriceSurface<I2D>::
    capPrice(const Date &d, Rate k) const {
        Time t = timeFromReference(d);
        return capPrice_(t,k);
    }

    //! remember that the strike uses the quoting convention
    template<class I2D>
    Real InterpolatedCPICapFloorTermPriceSurface<I2D>::
    floorPrice(const Date &d, Rate k) const {
        Time t = timeFromReference(d);
        return floorPrice_(t,k);
    }

    // inline definitions

    inline Period CPICapFloorTermPriceSurface::observationLag() const {
        return zeroInflationIndex()->zeroInflationTermStructure()->observationLag();
    }

    inline Date CPICapFloorTermPriceSurface::baseDate() const {
        return zeroInflationIndex()->zeroInflationTermStructure()->baseDate();
    }

    inline Real CPICapFloorTermPriceSurface::nominal() const {
        return nominal_;
    }

    inline BusinessDayConvention
    CPICapFloorTermPriceSurface::businessDayConvention() const {
        return bdc_;
    }

}

#endif
