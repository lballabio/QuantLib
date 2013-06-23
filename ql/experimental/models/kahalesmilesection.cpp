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

    KahaleSmileSection::KahaleSmileSection(const boost::shared_ptr<SmileSection> source, const Real atm,
                                           const bool interpolate, const bool exponentialExtrapolation,
                                           const bool deleteArbitragePoints, 
                                           const std::vector<Real>& moneynessGrid, const Real gap) : 
          SmileSection(*source), source_(source), gap_(gap), moneynessGrid_(moneynessGrid),
          interpolate_(interpolate), exponentialExtrapolation_(exponentialExtrapolation),
          deleteArbitragePoints_(deleteArbitragePoints) {

        if(atm==Null<Real>()) {
            f_ = source_->atmLevel();
            QL_REQUIRE(f_ != Null<Real>(), "atm level must be provided by source section or given in the constructor");
        }

        else {
            f_ = atm;
        }

        SmileSectionUtils ssutils;
        moneynessGrid_ = ssutils.makeMoneynessGrid(*source_,moneynessGrid);
        k_ = ssutils.makeStrikeGrid(*source_,moneynessGrid_);

        QL_REQUIRE(k_.front() >= 0.0 && k_.back() >= 0.0,"Strikes (" << k_.front() << " ... " << k_.back() << 
                   ") must be positive.");

        c_.push_back(f_);

        for(Size i=1;i<k_.size();i++) {
            c_.push_back( source_->optionPrice(k_[i],Option::Call,1.0) );
        }

        compute();

    }

    void KahaleSmileSection::compute() {

        SmileSectionUtils ssutils;

        if(deleteArbitragePoints_) {
            while(leftIndex_>1 || rightIndex_<k_.size()-1) {
                std::pair<Size,Size> afIdx = ssutils.arbitragefreeIndices(*source_,moneynessGrid_);

                leftIndex_ = afIdx.first;
                rightIndex_ = afIdx.second;

                QL_REQUIRE(rightIndex_>leftIndex_,
                           "arbitrage free region must at least contain two points (only index is " << leftIndex_ << ")");
            
                if(leftIndex_>1) {
                    moneynessGrid_.erase(moneynessGrid_.begin()+leftIndex_-1);
                    k_.erase(k_.begin()+leftIndex_-1);
                    c_.erase(c_.begin()+leftIndex_-1);
                    leftIndex_--;
                    rightIndex_--; 
                }

                if(rightIndex_<k_.size()-1) {
                    moneynessGrid_.erase(moneynessGrid_.begin()+rightIndex_+1);
                    k_.erase(k_.begin()+rightIndex_+1);
                    c_.erase(c_.begin()+rightIndex_+1);
                    rightIndex_--;
                }
            }
        }
        else {
            std::pair<Size,Size> afIdx = ssutils.arbitragefreeIndices(*source_,moneynessGrid_);
            leftIndex_ = afIdx.first;
            rightIndex_ = afIdx.second;
        }

        cFunctions_ = std::vector<boost::shared_ptr<cFunction> >(rightIndex_-leftIndex_+2);

        // extrapolation in the leftmost interval

        Brent brent;
        bool success;
        Real secl = 0.0;

        do {
            success=true;
            try {
                Real k1 = k_[leftIndex_];
                Real c1 = c_[leftIndex_];
                Real c0 = c_[0];
                secl = (c_[leftIndex_]-c_[0]) / (k_[leftIndex_]-k_[0]);
                Real sec = (c_[leftIndex_+1]-c_[leftIndex_]) / (k_[leftIndex_+1]-k_[leftIndex_]);
                Real c1p;
                if(interpolate_) c1p=(secl+sec)/2;
                else {
                    c1p=(blackFormula(Option::Call, k1+gap_, f_, sqrt(source_->variance(k1+gap_)))-
                         blackFormula(Option::Call, k1, f_, sqrt(source_->variance(k1))))/gap_;
                    QL_REQUIRE(secl < c1p && c1p <= 0.0,"dummy"); 
                    // can not extrapolate so throw exception which is caught below
                }
                sHelper1 sh1(k1,c0,c1,c1p);
                Real s = brent.solve(sh1,QL_KAHALE_ACC,0.20,0.00,QL_KAHALE_SMAX); // numerical parameters hardcoded here
                sh1(s);
                boost::shared_ptr<cFunction> cFct1(new cFunction(sh1.f_,s,0.0,sh1.b_));
                cFunctions_[0]=cFct1;
            } catch(...) {
                leftIndex_++;
                success=false;
            }
        } while(!success && leftIndex_ < rightIndex_);

        QL_REQUIRE(leftIndex_ < rightIndex_, "can not extrapolate to left, right index of af region reached (" 
                   << rightIndex_ << ")");

        // interpolation

        Real cp0 = 0.0, cp1 = 0.0;

        if(interpolate_) {

            for(Size i = leftIndex_; i<rightIndex_; i++) {
                Real k0 = k_[i];
                Real k1 = k_[i+1];
                Real c0 = c_[i];
                Real c1 = c_[i+1];
                Real sec = (c_[i+1]-c_[i]) / (k_[i+1]-k_[i]);
                if(i==leftIndex_) cp0 = leftIndex_ > 0 ? (secl + sec) / 2.0 : sec;
                Real secr;
                if(i==rightIndex_-1) secr=0.0;
                else secr = (c_[i+2]-c_[i+1]) / (k_[i+2]-k_[i+1]);
                cp1 = (sec+secr) / 2.0;
                aHelper ah(k0,k1,c0,c1,cp0,cp1);
                Real a;
                try {
                    a = brent.solve(ah,QL_KAHALE_ACC,0.5*(cp1+(1.0+cp0)),cp1+QL_KAHALE_EPS,1.0+cp0-QL_KAHALE_EPS);
                    // numerical parameters hardcoded here
                } catch(...) {
                    // from theory there must exist a zero. if the solver does not find it, it most
                    // probably lies close one of the interval bounds. Just choose the better bound
                    // and relax the accuracy. This does not matter in practice usually.
                    Real la = std::fabs(ah(cp1+QL_KAHALE_EPS));
                    Real ra = std::fabs(ah(1.0+cp0-QL_KAHALE_EPS));
                    if( la < QL_KAHALE_ACC_RELAX || ra < QL_KAHALE_ACC_RELAX) { // tolerance hardcoded here
                        a = la < ra ? cp1+QL_KAHALE_EPS : 1.0+cp0-QL_KAHALE_EPS;
                    }
                    else
                        QL_FAIL("can not interpolate at index " << i);

                }
                ah(a);
                boost::shared_ptr<cFunction> cFct(new cFunction(ah.f_,ah.s_,a,ah.b_));
                cFunctions_[leftIndex_ > 0 ? i-leftIndex_+1 : 0]=cFct;
                cp0=cp1;
            }

        }

        // extrapolation of right wing

        do {
            success=true;
            try {
                Real k0 = k_[rightIndex_];
                Real c0 = c_[rightIndex_];
                Real cp0;
                if(interpolate_) cp0 = 0.5*(c_[rightIndex_]-c_[rightIndex_-1])/(k_[rightIndex_]-k_[rightIndex_-1]);
                else {
                    cp0=(blackFormula(Option::Call, k0, f_, sqrt(source_->variance(k0)))-
                         blackFormula(Option::Call, k0-gap_, f_, sqrt(source_->variance(k0-gap_))))/gap_;

                }
                boost::shared_ptr<cFunction> cFct;
                if(exponentialExtrapolation_) {
                    cFct = boost::shared_ptr<cFunction>(new cFunction(-cp0/c0 ,std::log(c0)-cp0/c0*k0));
                }
                else {
                    sHelper sh(k0,c0,cp0);
                    Real s;
                    s = brent.solve(sh,QL_KAHALE_ACC,0.20,0.0,QL_KAHALE_SMAX);  // numerical parameters hardcoded here
                    sh(s);
                    cFct = boost::shared_ptr<cFunction>(new cFunction(sh.f_,s,0.0,0.0));
                }
                cFunctions_[rightIndex_-leftIndex_+1]=cFct;
            } catch (...) {
                rightIndex_--;
                success=false;
            }
        } while(!success && rightIndex_ > leftIndex_);

        QL_REQUIRE(leftIndex_ < rightIndex_, "can not extrapolate to right, left index of af region reached (" << 
                   leftIndex_ << ")");


    }

    Real KahaleSmileSection::optionPrice(Rate strike, Option::Type type, Real discount) const { 
        // option prices are directly available, so implement this function rather than use smileSection 
        // standard implementation
        strike = std::max ( strike, QL_KAHALE_EPS );
        int i = index(strike);
        if(interpolate_ || (i==0 || i==(int) (rightIndex_-leftIndex_+1))) 
            return discount*(type == Option::Call ? cFunctions_[i]->operator()(strike) : 
                                                    cFunctions_[i]->operator()(strike)+strike-f_);
        else 
            return source_->optionPrice(strike,type,discount); 
    }

    Real KahaleSmileSection::volatilityImpl(Rate strike) const {
        strike = std::max ( strike, QL_KAHALE_EPS );
        int i = index(strike);
        if(!interpolate_ && !(i==0 || i==(int) (rightIndex_-leftIndex_+1))) return source_->volatility(strike);
        Real c = cFunctions_[i]->operator()(strike);
        Real vol=0.0;
        try {
            vol = blackFormulaImpliedStdDev(Option::Call,strike,f_,c) / sqrt(exerciseTime());
        } catch(...) { }
        return vol;
    }

    Size KahaleSmileSection::index(Rate strike) const {
        int i;
        i=static_cast<int>(std::upper_bound(k_.begin(),k_.end(),strike)-k_.begin());
        i -= leftIndex_;
        i = std::max(std::min(i,static_cast<int>(rightIndex_-leftIndex_+1)),0);
        return i;
    }


}
