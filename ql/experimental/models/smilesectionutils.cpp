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

#include <ql/experimental/models/smilesectionutils.hpp>

namespace QuantLib {

    const std::pair<Real,Real>
    SmileSectionUtils::arbitragefreeRegion(
                               const SmileSection& section,
                               const std::vector<Real>& moneynessGrid) const {
        std::pair<Size,Size> indices = arbitragefreeIndices(section,
                                                            moneynessGrid);
        return std::pair<Real,Real>(k_[indices.first],k_[indices.second]);
    }

    const std::pair<Size,Size>
    SmileSectionUtils::arbitragefreeIndices(
                              const SmileSection& section,
                              const std::vector<Real>& moneynessGrid ) const {

        makeStrikeGrid(section,moneynessGrid);

        c_.clear();
        Real atm = section.atmLevel(); // this is not NULL because
                                       // makeStrikeGrid ensured that
        c_.push_back(atm);

        for(Size i=1;i<k_.size();i++) {
            c_.push_back(section.optionPrice(k_[i],Option::Call,1.0) );
        }

        Size centralIndex =
            std::upper_bound(m_.begin(),
                             m_.end(),
                             1.0-QL_EPSILON) - m_.begin();
        QL_REQUIRE(centralIndex < k_.size()-1 && centralIndex > 1,
                   "Atm point in moneyness grid (" << centralIndex
                   << ") too close to boundary.");
        Size leftIndex = centralIndex;
        Size rightIndex = centralIndex;

        bool isAf=true;
        do {
            rightIndex++;
            isAf = af(leftIndex,rightIndex);
        } while(isAf && rightIndex<k_.size()-1);
        if(!isAf) rightIndex--;

        do {
            leftIndex--;
            isAf = af(leftIndex,leftIndex) && af(leftIndex,leftIndex+1);
        } while(isAf && leftIndex>1);
        if(!isAf) leftIndex++;

        if(rightIndex < leftIndex)
            rightIndex = leftIndex;

        return std::pair<Size,Size>(leftIndex,rightIndex);
    }

    bool SmileSectionUtils::af(Size i0, Size i) const {
        if(i==0) return true;
        Size im = i-1 >= i0 ? i-1 : 0;
        Real q1 = (c_[i] - c_[im]) / (k_[i] - k_[im]);
        if(q1 < -1.0 || q1 > 0.0) return false;
        if(i >= k_.size()-1) return true;
        Real q2 = (c_[i+1] - c_[i]) / (k_[i+1] - k_[i]);
        if(q1 <= q2) return true;
        return false;
    }

    const std::vector<Real>&
    SmileSectionUtils::makeMoneynessGrid(
                               const SmileSection& section,
                               const std::vector<Real>& moneynessGrid) const {
        
        m_.clear();

        if(moneynessGrid.size()!=0) {
            QL_REQUIRE(moneynessGrid[0] >= 0.0, "moneyness grid should only containt non negative values (" <<
                       moneynessGrid[0] << ")");
            for(Size i=0;i<moneynessGrid.size()-1;i++) {
                QL_REQUIRE(moneynessGrid[i] < moneynessGrid[i+1],
                           "moneyness grid should containt strictly increasing values (" << moneynessGrid[i] << "," <<
                           moneynessGrid[i+1] << " at indices " << i << ", " << i+1 << ")");
            }
        }

        std::vector<Real> tmp;

        static const Real defaultMoney[] = {0.0,0.01,0.05,0.10,0.25,0.40,0.50,0.60,0.70,0.80,0.90,
                 1.0,1.25,1.5,1.75,2.0,5.0,7.5,10.0,15.0,20.0};

        if(moneynessGrid.size()==0) tmp = std::vector<Real>(defaultMoney, defaultMoney+21);
        else tmp = std::vector<Real>(moneynessGrid);

        if(tmp[0] > QL_EPSILON) m_.push_back(0.0);

        Real atm = section.atmLevel();
        QL_REQUIRE(atm != Null<Real>(),
                   "smile section must provide atm level to compute moneyness grid");

        for(Size i=0;i<tmp.size();i++) {
            if(fabs(tmp[i]) < QL_EPSILON ||
               (tmp[i]*atm >= section.minStrike() && tmp[i]*atm <= section.maxStrike()))
                m_.push_back( tmp[i] );
        }

        return m_;
    }

    const std::vector<Real>&
    SmileSectionUtils::makeStrikeGrid(
                               const SmileSection& section,
                               const std::vector<Real>& moneynessGrid) const {

        makeMoneynessGrid(section,moneynessGrid);

        k_.clear();

        Real atm = section.atmLevel();

        for(Size i=0;i<m_.size();i++) {
            k_.push_back( m_[i] * atm );
        }

        return k_;
    }

}
