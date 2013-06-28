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
    \brief cpi inflation cap and floor term price structure.  N.B. 
           cpi cap/floors have a single (one) flow (unlike nominal
           caps) because they observe cumulative inflation up to
           their maturity.  Options are on CPI(T)/CPI(0) but strikes
           are quoted for yearly average inflation, so require transformation
           via (1+quote)^T to obtain actual strikes.  These are consistent
           with ZCIIS quoting conventions.
 
    The single-flow property of CPI cap/floors means that no stripping
    is required.  Additionally CPI swaps, i.e. ZCIIS zero coupon inflation
    indexed swaps are usually the most liquid of all the inflation products.
    These two facts mean that we can use a Zero Inflation term structure to
    give ATM.
 
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
    /*! 
        The inflation index MUST contain a ZeroInflationTermStructure as
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
        CPICapFloorTermPriceSurface(Real nominal, 
                                    Real baseRate,  // avoids an uncontrolled crash if index has no TS
                                    const Period &observationLag,   
                                    const Calendar &cal, // calendar in index may not be useful
                                    const BusinessDayConvention &bdc,
                                    const DayCounter &dc,
                                    const Handle<ZeroInflationIndex>& zii,
                                    const Handle<YieldTermStructure>& yts,
                                    const std::vector<Rate> &cStrikes,
                                    const std::vector<Rate> &fStrikes,
                                    const std::vector<Period> &cfMaturities,
                                    const Matrix &cPrice,
                                    const Matrix &fPrice);
 
        //! \name InflationTermStructure interface
        //@{
        Period observationLag() const;
        Date baseDate() const;
        //@}

        //! is based on
        Handle<ZeroInflationIndex> zeroInflationIndex() const { return zii_; }


        //! inspectors
        /*! \note you don't know if price() is a cap or a floor
                  without checking the ZeroInflation ATM level.
        */
        //@{
        virtual Real nominal() const;
        virtual BusinessDayConvention businessDayConvention() const;
        //@}

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
        virtual Date maxDate() const {return referenceDate()+cfMaturities_.back();}
        //@}

        
        virtual Date cpiOptionDateFromTenor(const Period& p) const;

      protected:
        virtual bool checkStrike(Rate K) {
            return ( minStrike() <= K && K <= maxStrike() );
        }
        virtual bool checkMaturity(const Date& d) {
            return ( minDate() <= d && d <= maxDate() );
        }
        
        

        Handle<ZeroInflationIndex> zii_;
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
                                                const Handle<ZeroInflationIndex>& zii,
                                                const Handle<YieldTermStructure>& yts,
                                                const std::vector<Rate> &cStrikes,
                                                const std::vector<Rate> &fStrikes,
                                                const std::vector<Period> &cfMaturities,
                                                const Matrix &cPrice,
                                                const Matrix &fPrice,
                                                const Interpolator2D &interpolator2d = Interpolator2D());

            //! \name LazyObject interface
            //@{
            void update();
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
            virtual Real price(const Date &d, Rate k) const;
            virtual Real capPrice(const Date &d, Rate k) const;
            virtual Real floorPrice(const Date &d, Rate k) const;
            //@}
            
        protected:
        
            // data for surfaces and curve
            mutable std::vector<Rate> allStrikes_;
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
                                            const Handle<ZeroInflationIndex>& zii,
                                            const Handle<YieldTermStructure>& yts,
                                            const std::vector<Rate> &cStrikes,
                                            const std::vector<Rate> &fStrikes,
                                            const std::vector<Period> &cfMaturities,
                                            const Matrix &cPrice,
                                            const Matrix &fPrice,
                                            const Interpolator2D &interpolator2d)
    : CPICapFloorTermPriceSurface(nominal, startRate, observationLag, cal, bdc, dc,
                                  zii, yts, cStrikes, fStrikes, cfMaturities, cPrice, fPrice),
      interpolator2d_(interpolator2d) {
        performCalculations();
    }

    #endif

    
    
    template<class I2D>
    void InterpolatedCPICapFloorTermPriceSurface<I2D>::
    update() {
        notifyObservers();
    }


    //! set up the interpolations for capPrice_ and floorPrice_
    //! since we know ATM, and we have single flows,
    //! we can use put/call parity to extend the surfaces
    //! across all strikes
    template<class I2D>
    void InterpolatedCPICapFloorTermPriceSurface<I2D>::
    performCalculations() const {
        
        
        Size nMat = cfMaturities_.size(), ncK = cStrikes_.size(), nfK = fStrikes_.size(),
            nK = ncK + nfK;
        Matrix cP(nK, nMat), fP(nK, nMat);
        Handle<ZeroInflationTermStructure> zts = zii_->zeroInflationTermStructure();
        Handle<YieldTermStructure> yts = this->nominalTermStructure();
        QL_REQUIRE(!zts.empty(),"Zts is empty!!!");
        QL_REQUIRE(!yts.empty(),"Yts is empty!!!");
        
        for (Size i =0; i<nfK; i++){
            allStrikes_.push_back(fStrikes_[i]);
            for (Size j=0; j<nMat; j++) {
                Period mat = cfMaturities_[j];
                Real df = yts->discount(cpiOptionDateFromTenor(mat));
                Real atm_quote = zts->zeroRate(cpiOptionDateFromTenor(mat));
                Real atm = std::pow(1.0+atm_quote, mat.length());
                Real S = atm * df;
                Real K_quote = fStrikes_[i]/100.0;
                Real K = std::pow(1.0+K_quote, mat.length());
                cP[i][j] = fPrice_[i][j] + S - K * df;
                fP[i][j] = fPrice_[i][j];
            }
        }
        for (Size i =0; i<ncK; i++){
            allStrikes_.push_back(cStrikes_[i]);
            for (Size j=0; j<nMat; j++) {
                Period mat = cfMaturities_[j];
                Real df = yts->discount(cpiOptionDateFromTenor(mat));
                Real atm_quote = zts->zeroRate(cpiOptionDateFromTenor(mat));
                Real atm = std::pow(1.0+atm_quote, mat.length());
                Real S = atm * df;
                Real K_quote = cStrikes_[i]/100.0;
                Real K = std::pow(1.0+K_quote, mat.length());
                cP[i+nfK][j] = cPrice_[i][j];
                fP[i+nfK][j] = cPrice_[i][j] + K * df - S;
            }
        }
        
        // copy to store        
        cPriceB_ = cP;
        fPriceB_ = fP;
        
        cfMaturityTimes_.clear();
        for (Size i=0; i<cfMaturities_.size();i++) {
            cfMaturityTimes_.push_back(timeFromReference(cpiOptionDateFromTenor(cfMaturities_[i])));
        }
        
        capPrice_ = interpolator2d_.interpolate(cfMaturityTimes_.begin(),cfMaturityTimes_.end(),
                                                allStrikes_.begin(), allStrikes_.end(),
                                                cPriceB_
                                                );
        capPrice_.enableExtrapolation();
        
        floorPrice_ = interpolator2d_.interpolate(cfMaturityTimes_.begin(),cfMaturityTimes_.end(),
                                                  allStrikes_.begin(), allStrikes_.end(),
                                                  fPriceB_
                                                  );
        floorPrice_.enableExtrapolation();
        
        /* test code - note order of indices
        for (Size i =0; i<nK; i++){
            std::cout << allStrikes_[i] << ":  ";
            Real qK = allStrikes_[i];
            for (Size j=0; j<nMat; j++) {
                Real t = cfMaturityTimes_[j];
                std::cout << fP[i][j] << "," << floorPrice_(t,qK) << " | " ;
            }
            std::cout << std::endl;
        }
        
        for (Size i =0; i<nK; i++){
            std::cout << allStrikes_[i] << ":  ";
            Real qK = allStrikes_[i];
            for (Size j=0; j<nMat; j++) {
                Real t = cfMaturityTimes_[j];
                std::cout << cP[i][j] << "," << capPrice_(t,qK) << " | " ;
            }
            std::cout << std::endl;
        }
        */
    }

    //! remember that the strike uses the quoting convention
    template<class I2D>
    Real InterpolatedCPICapFloorTermPriceSurface<I2D>::
    price(const Date &d, Rate k) const {
        
        Rate atm = zeroInflationIndex()->zeroInflationTermStructure()->zeroRate(d);
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

    // inline

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
