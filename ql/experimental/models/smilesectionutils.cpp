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

        std::vector<Real> moneynessGrid1 = makeMoneynessGrid(moneynessGrid);
        makeStrikeGrid(section,moneynessGrid);

        c_.clear();
        Real atm = section.atmLevel(); // this is not NULL because
                                       // makeStrikeGrid ensured that
        c_.push_back(atm);

        for(Size i=1;i<k_.size();i++) {
            c_.push_back(section.optionPrice(k_[i],Option::Call,1.0) );
        }

        Size centralIndex =
            std::upper_bound(moneynessGrid1.begin(),
                             moneynessGrid1.end(),
                             1.0-QL_EPSILON) - moneynessGrid1.begin();
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

    const Disposable<std::vector<Real> >
    SmileSectionUtils::makeMoneynessGrid(
                               const std::vector<Real>& moneynessGrid) const {

        std::vector<Real> result;

        if(moneynessGrid.size()==0) {
            Real defaultMoneynesses[] =
                {0.0,0.01,0.05,0.10,0.25,0.40,0.50,0.60,0.70,0.80,0.90,
                 1.0,1.25,1.5,1.75,2.0,5.0,7.5,10.0,15.0,20.0};
            for(int i=0;i<21;i++) result.push_back(defaultMoneynesses[i]);
        }
        else {
            result = std::vector<Real>(moneynessGrid);
            std::sort(result.begin(),result.end());
            QL_REQUIRE(result[0]>=0.0,
                       "Moneynessgrid should contain only non negative values ("
                       << result[0] << ")");
            if(result[0]>QL_EPSILON) result.insert(result.begin(),0.0);
        }

        return result;
    }

    const std::vector<Real>&
    SmileSectionUtils::makeStrikeGrid(
                               const SmileSection& section,
                               const std::vector<Real>& moneynessGrid) const {

        std::vector<Real> moneyness = makeMoneynessGrid(moneynessGrid);

        k_.clear();
        Real atm = section.atmLevel();
        QL_REQUIRE(atm != Null<Real>(),
                   "smile section must provide atm level to compute strike grid");

        for(Size i=0;i<moneyness.size();i++) {
            Real strike = moneyness[i] * atm;
             // only use the strikes that are allowed in the section,
             // but add zero strike always
            if(fabs(strike) < QL_EPSILON ||
               (strike >= section.minStrike() && strike <= section.maxStrike()))
                    k_.push_back( strike  );
        }

        return k_;
    }

}
