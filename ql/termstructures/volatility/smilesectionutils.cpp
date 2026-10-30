/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2013, 2015, 2018 Peter Caspers

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/termstructures/volatility/smilesectionutils.hpp>
#include <ql/math/comparison.hpp>
#include <algorithm>

namespace QuantLib {

    SmileSectionUtils::SmileSectionUtils(const SmileSection &section,
                                         const std::vector<Real> &moneynessGrid,
                                         const Real atm,
                                         const bool deleteArbitragePoints) {

        if (!moneynessGrid.empty()) {
            QL_REQUIRE(
                section.volatilityType() == Normal || moneynessGrid[0] >= 0.0,
                "moneyness grid should only contain non negative values ("
                    << moneynessGrid[0] << ")");
            for (Size i = 0; i < moneynessGrid.size() - 1; i++) {
                QL_REQUIRE(moneynessGrid[i] < moneynessGrid[i + 1],
                           "moneyness grid should contain strictly increasing "
                           "values ("
                               << moneynessGrid[i] << ","
                               << moneynessGrid[i + 1] << " at indices " << i
                               << ", " << i + 1 << ")");
            }
        }

        if (atm == Null<Real>()) {
            f_ = section.atmLevel();
            QL_REQUIRE(f_ != Null<Real>(),
                       "atm level must be provided by source section or given "
                       "in the constructor");
        } else {
            f_ = atm;
        }

        std::vector<Real> tmp;

        static const Real defaultMoney[] = { 0.0,  0.01, 0.05, 0.10, 0.25, 0.40,
                                             0.50, 0.60, 0.70, 0.80, 0.90, 1.0,
                                             1.25, 1.5,  1.75, 2.0,  5.0,  7.5,
                                             10.0, 15.0, 20.0 };
        static const Real defaultMoneyNormal[] = {
            -0.20,  -0.15,  -0.10,  -0.075,  -0.05,   -0.04,   -0.03,
            -0.02,  -0.015, -0.01,  -0.0075, -0.0050, -0.0025, 0.0,
            0.0025, 0.0050, 0.0075, 0.01,    0.015,   0.02,    0.03,
            0.04,   0.05,   0.075,  0.10,    0.15,    0.20
        };

        if (moneynessGrid.empty()) {
            tmp = section.volatilityType() == Normal
                      ? std::vector<Real>(defaultMoneyNormal,
                                          defaultMoneyNormal + 27)
                      : std::vector<Real>(defaultMoney, defaultMoney + 21);
        }
        else
            tmp = std::vector<Real>(moneynessGrid);

        Real shift = section.shift();

        if (section.volatilityType() == ShiftedLognormal && tmp[0] > QL_EPSILON) {
            m_.push_back(0.0);
            k_.push_back(-shift);
        }

        bool minStrikeAdded = false, maxStrikeAdded = false;
        for (Real& i : tmp) {
            Real k = section.volatilityType() == Normal ? Real(f_ + i) : Real(i * (f_ + shift) - shift);
            if ((section.volatilityType() == ShiftedLognormal && i <= QL_EPSILON) ||
                (k >= section.minStrike() && k <= section.maxStrike())) {
                if (!minStrikeAdded || !close(k, section.minStrike())) {
                    m_.push_back(i);
                    k_.push_back(k);
                }
                if (close(k, section.maxStrike()))
                    maxStrikeAdded = true;
            } else { // if the section provides a limited strike range
                     // we put the respective endpoint in our grid
                     // in order to not loose too much information
                if (k < section.minStrike() && !minStrikeAdded) {
                    m_.push_back(section.volatilityType() == Normal
                                     ? Real(section.minStrike() - f_)
                                     : Real((section.minStrike() + shift) / f_));
                    k_.push_back(section.minStrike());
                    minStrikeAdded = true;
                }
                if (k > section.maxStrike() && !maxStrikeAdded) {
                    m_.push_back(section.volatilityType() == Normal
                                     ? Real(section.maxStrike() - f_)
                                     : Real((section.maxStrike() + shift) / f_));
                    k_.push_back(section.maxStrike());
                    maxStrikeAdded = true;
                }
            }
        }

        // only known for shifted lognormal vols, otherwise we include
        // the lower strike in the loop below
        if(section.volatilityType() == ShiftedLognormal)
            c_.push_back(f_ + shift);

        for (Size i = (section.volatilityType() == Normal ? 0 : 1);
             i < k_.size(); i++) {
            c_.push_back(section.optionPrice(k_[i], Option::Call, 1.0));
        }

        Size centralIndex =
            std::upper_bound(m_.begin(), m_.end(),
                             (section.volatilityType() == Normal ? 0.0 : 1.0) -
                                 QL_EPSILON) -
            m_.begin();
        QL_REQUIRE(centralIndex < k_.size() - 1 && centralIndex > 1,
                   "Atm point in moneyness grid ("
                       << centralIndex << ") too close to boundary.");

        // shift central index to the right if necessary
        // (sometimes even the atm point lies in an arbitrageable area)

        while (centralIndex < k_.size() - 1 &&
               !af(centralIndex, centralIndex, centralIndex + 1))
            centralIndex++;

        QL_REQUIRE(centralIndex < k_.size(),
                   "central index is at right boundary");

        leftIndex_ = centralIndex;
        rightIndex_ = centralIndex;

        bool done = false;
        while (!done) {

            bool isAf = true;
            done = true;

            while (isAf && rightIndex_ < k_.size() - 1) {
                rightIndex_++;
                isAf = af(leftIndex_, rightIndex_, rightIndex_) &&
                       af(leftIndex_, rightIndex_ - 1, rightIndex_);
            }
            if (!isAf)
                rightIndex_--;

            isAf = true;
            while (isAf && leftIndex_ > 1) {
                leftIndex_--;
                isAf = af(leftIndex_, leftIndex_, rightIndex_) &&
                       af(leftIndex_, leftIndex_ + 1, rightIndex_);
            }
            if (!isAf)
                leftIndex_++;

            if (rightIndex_ < leftIndex_)
                rightIndex_ = leftIndex_;

            if (deleteArbitragePoints && leftIndex_ > 1) {
                m_.erase(m_.begin() + leftIndex_ - 1);
                k_.erase(k_.begin() + leftIndex_ - 1);
                c_.erase(c_.begin() + leftIndex_ - 1);
                leftIndex_--;
                if (rightIndex_ > 0)
                    rightIndex_--;
                done = false;
            }
            if (deleteArbitragePoints && rightIndex_ < k_.size() - 1) {
                m_.erase(m_.begin() + rightIndex_ + 1);
                k_.erase(k_.begin() + rightIndex_ + 1);
                c_.erase(c_.begin() + rightIndex_ + 1);
                if (rightIndex_ > 0)
                    rightIndex_--;
                done = false;
            }
        }

        QL_REQUIRE(rightIndex_ > leftIndex_,
                   "arbitrage free region must at least contain two "
                   "points (only index is "
                       << leftIndex_ << ")");

    }

    std::pair<Real, Real> SmileSectionUtils::arbitragefreeRegion() const {
        return {k_[leftIndex_], k_[rightIndex_]};
    }

    std::pair<Size, Size> SmileSectionUtils::arbitragefreeIndices() const {
        return {leftIndex_, rightIndex_};
    }

    bool SmileSectionUtils::af(const Size i0, const Size i,
                               const Size i1) const {
        if (i == 0)
            return true;
        Size im = i - 1 >= i0 ? i - 1 : 0;
        Real q1 = (c_[i] - c_[im]) / (k_[i] - k_[im]);
        if (q1 < -1.0 || q1 > 0.0)
            return false;
        if (i >= i1)
            return true;
        Real q2 = (c_[i + 1] - c_[i]) / (k_[i + 1] - k_[i]);
        return q1 <= q2 && q2 <= 0.0;
    }
}
