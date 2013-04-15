/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2013 Peter Caspers

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

#include <ql/experimental/models/kahalesmilesection.hpp>
#include <ql/experimental/models/smilesectionutils.hpp>

namespace QuantLib {

    KahaleSmileSection::KahaleSmileSection(const boost::shared_ptr<SmileSection> source, const Real atm, const bool interpolate, const std::vector<Real>& moneynessGrid, const Real gap)
        : source_(source), interpolate_(interpolate), SmileSection(*source), moneynessGrid_(moneynessGrid), gap_(1E-8) {

        if(atm==Null<Real>()) {
            f_ = source_->atmLevel();
            QL_REQUIRE(f_ != Null<Real>(), "atm level must be provided by source section or given in the constructor");
        }
        else {
            f_ = atm;
        }

        SmileSectionUtils ssutils;
        k_ = ssutils.makeStrikeGrid(*source_,moneynessGrid);

        QL_REQUIRE(k_.front() >= 0.0 && k_.back() >= 0.0,"Strikes (" << k_.front() << " ... " << k_.back() << ") must be positive.");

        c_.push_back(f_);

        for(Size i=1;i<k_.size();i++) {
            c_.push_back( source_->optionPrice(k_[i],Option::Call,1.0) );
        }

        compute();

    }

    void KahaleSmileSection::compute() {
        SmileSectionUtils ssutils;
        std::pair<Size,Size> afIdx =
            ssutils.arbitragefreeIndices(*source_,moneynessGrid_);

        leftIndex_ = afIdx.first;
        rightIndex_ = afIdx.second;

        QL_REQUIRE(rightIndex_>leftIndex_,"arbitrage free region must at least contain two points (only index is " << leftIndex_ << ")");
        cFunctions_ = std::vector<boost::shared_ptr<cFunction> >(rightIndex_-leftIndex_+2);

        Brent brent;
        bool success;
        Real secl;

        if(leftIndex_ > 0) {
            do {
                success=true;
                try {
                    Real k1 = k_[leftIndex_];
                    Real c1 = c_[leftIndex_];
                    Real c0 = c_[0];
                    //Real c1p = cp_[leftIndex_-1];
                    secl = (c_[leftIndex_]-c_[0]) / (k_[leftIndex_]-k_[0]);
                    Real sec = (c_[leftIndex_+1]-c_[leftIndex_]) / (k_[leftIndex_+1]-k_[leftIndex_]);
                    Real c1p;
                    if(interpolate_) c1p=(secl+sec)/2;
                    else {
                        c1p=(blackFormula(Option::Call, k1+gap_, f_, sqrt(source_->variance(k1+1.0E-8)))-blackFormula(Option::Call, k1, f_, sqrt(source_->variance(k1))))/1.0E-8;
                        QL_REQUIRE(secl < c1p && c1p <= 0.0,"Dummy"); // can not extrapolate so throw exception which is caught below
                    }
                    sHelper1 sh1(k1,c0,c1,c1p);
                    Real s = brent.solve(sh1,1E-8,0.20,0.00,5.0);
                    if(sh1.f_<=100.0) { // FIXME what is a reasonable value here ?
                        boost::shared_ptr<cFunction> cFct1(new cFunction(sh1.f_,s,0.0,sh1.b_));
                        cFunctions_[0]=cFct1;
                    }
                    else {
                        leftIndex_++;
                        success=false;
                    }
                } catch(QuantLib::Error) {
                    leftIndex_++;
                    success=false;
                }
            } while(!success && leftIndex_ < rightIndex_);
            QL_REQUIRE(leftIndex_ < rightIndex_, "can not extrapolate to left, right index of af region reached (" << rightIndex_ << ")");
        }

        Real cp0, cp1;

        if(interpolate_) {

            for(Size i = leftIndex_; i<rightIndex_; i++) {
                Size im = i-1 >= leftIndex_ ? i-1 : 0;
                Real k0 = k_[i];
                Real k1 = k_[i+1];
                Real c0 = c_[i];
                Real c1 = c_[i+1];
                Real sec = (c_[i+1]-c_[i]) / (k_[i+1]-k_[i]);
                if(i==leftIndex_) cp0 = (secl + sec) / 2.0;
                Real secr;
                if(i==rightIndex_-1) secr=0.0;
                else secr = (c_[i+2]-c_[i+1]) / (k_[i+2]-k_[i+1]);
                cp1 = (sec+secr) / 2.0;
                aHelper ah(k0,k1,c0,c1,cp0,cp1);
                try {
                    Real a = brent.solve(ah,1E-8,0.5*(cp1+(1.0+cp0)),cp1+QL_EPSILON,1.0+cp0-QL_EPSILON);
                    ah(a);
                    QL_REQUIRE(ah.f_ <= 100.0 && ah.s_ <= 5.0, "can not interpolate at index " << i << " (f,s) = (" << ah.f_ << "," << ah.s_ << ")"); // FIXME what is reasonable here?
                    boost::shared_ptr<cFunction> cFct(new cFunction(ah.f_,ah.s_,a,ah.b_));
                    cFunctions_[leftIndex_ > 0 ? i-leftIndex_+1 : 0]=cFct;
                } catch(QuantLib::Error) {
                    QL_FAIL("can not interpolate at index " << i);
                }
                cp0=cp1;
            }

        }

        do {
            success=true;
            try {
                Real k0 = k_[rightIndex_];
                Real c0 = c_[rightIndex_];
                Real c0p;
                if(interpolate_) c0p = (c_[rightIndex_]-c_[rightIndex_-1])/(k_[rightIndex_]-k_[rightIndex_-1]);
                else {
                    c0p=(blackFormula(Option::Call, k0, f_, sqrt(source_->variance(k0)))-blackFormula(Option::Call, k0-gap_, f_, sqrt(source_->variance(k0-1.0E-8))))/1.0E-8;
                    QL_REQUIRE(-1 < c0p && c0p < 0.0, "dummy");
                }
                sHelper sh(k0,c0,c0p);
                Real s;
                s = brent.solve(sh,1E-8,0.20,0.00,5.0);
                if(f_<=100.0) { // FIXME what is reasonable here ?
                    boost::shared_ptr<cFunction> cFct(new cFunction(sh.f_,s,0.0,0.0));
                    cFunctions_[rightIndex_-leftIndex_+1]=cFct;
                }
                else {
                    rightIndex_--;
                    success=false;
                }
            } catch ( QuantLib::Error ) {
                rightIndex_--;
                success=false;
            }
        } while(!success && rightIndex_ > leftIndex_);

        QL_REQUIRE(leftIndex_ < rightIndex_, "can not extrapolate to right, left index of af region reached (" << leftIndex_ << ")");


    }

    Real KahaleSmileSection::optionPrice(Rate strike, Option::Type type, Real discount) const { // option prices are directly available, so implement this function rather than use smileSection standard implementation
        strike = std::max ( strike, QL_EPSILON );
        int i = index(strike);
        if(interpolate_ || (i==0 || i==rightIndex_-leftIndex_+1)) return discount*(type == Option::Call ? cFunctions_[i]->operator()(strike) : cFunctions_[i]->operator()(strike)+strike-f_);
        return source_->optionPrice(strike,type,discount); 
    }

    Real KahaleSmileSection::volatilityImpl(Rate strike) const {
        strike = std::max ( strike, QL_EPSILON );
        int i = index(strike);
        if(!interpolate_ && !(i==0 || i==rightIndex_-leftIndex_+1)) return source_->volatility(strike);
        Real c = cFunctions_[i]->operator()(strike);
        Real vol=0.0;
        try {
            vol = blackFormulaImpliedStdDev(Option::Call,strike,f_,c) / sqrt(exerciseTime());
        } catch(QuantLib::Error) { }
        return vol;
    }

    Size KahaleSmileSection::index(Rate strike) const {
        int i;
        //if(strike>=k_.back()) i=k_.size();
        /*else*/ i=std::upper_bound(k_.begin(),k_.end(),strike)-k_.begin();
        i -= leftIndex_;
        i = std::max(std::min(i,static_cast<int>(rightIndex_-leftIndex_+1)),0);
        return i;
    }


}
