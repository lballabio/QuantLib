
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.org/
 *
 * QuantLib is free software and you are allowed to use, copy, modify, merge,
 * publish, distribute, and/or sell copies of it under the conditions stated
 * in the QuantLib License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
 *
 * You should have received a copy of the license along with this file;
 * if not, please email quantlib-users@lists.sourceforge.net
 * The license is also available at http://quantlib.org/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.org/group.html
*/

/*! \file couponbondoption.cpp
    \brief Coupon bond option pricer

    \fullpath
    ql/Pricers/%couponbondoption.cpp
*/

// $Id$

#include "ql/Pricers/couponbondoption.hpp"
#include "ql/Solvers1D/brent.hpp"

namespace QuantLib {

    namespace Pricers {

        using namespace InterestRateModelling;

        class CouponBondOption::rStarFinder : public ObjectiveFunction {
          public:
            rStarFinder(CouponBondOption *cbo);
            double operator()(double x) const;
          private: 
            double strike_;
            Time maturity_;
            const std::vector<Time>& times_;
            const std::vector<double>& amounts_;
            const Handle<Model>& model_;
        };      
            
        inline CouponBondOption::rStarFinder::rStarFinder(CouponBondOption *cbo) :
            strike_(cbo->strike_), maturity_(cbo->maturity_), 
            times_(cbo->times_), amounts_(cbo->amounts_), model_(cbo->model_)
        {}

        inline double CouponBondOption::rStarFinder::operator()(double x) const {
            double value = strike_;
            unsigned size = times_.size();
            for (unsigned i=0; i<size; i++) {
                double dbValue = model_->discountBond(maturity_, times_[i], x);
                value -= amounts_[i]*dbValue;
                cout << i << ": (" << x << ") " << amounts_[i] << " " << dbValue << endl;
            }
            return value;
        }

        CouponBondOption::CouponBondOption(
            Time maturity, const std::vector<Time>& times,
            const std::vector<double>& amounts, Option::Type type,
            double strike, const Handle<Model>& model) 
        : maturity_(maturity), times_(times), amounts_(amounts), type_(type),
          strike_(strike), model_(model) {}

        double CouponBondOption::value(Rate rate) {
            rStarFinder finder(this);
            Solvers1D::Brent s1d = Solvers1D::Brent();
            double minStrike = 0.00001;
            double maxStrike = 0.20000;
            s1d.setMaxEvaluations(10000);
            s1d.setLowBound(minStrike);
            s1d.setHiBound(maxStrike);
            double rStar = s1d.solve(finder, 1e-8, rate, minStrike, maxStrike);
            cout << "rStar = " << rStar*100.0 << "%" << endl;
            cout << finder(0.06) << endl;
            cout << finder(0.05) << endl;
            cout << finder(0.04) << endl;
            cout << finder(0.03) << endl;

            unsigned size = times_.size();
            double value = 0.0;
            for (unsigned i=0; i<size; i++) {
                double strike = model_->discountBond(maturity_, times_[i], rStar);
                double dboValue = model_->discountBondOption(type_, strike, maturity_, times_[i]);
                cout << "strike=" << strike << " dboValue=" << dboValue << endl;
                value += amounts_[i]*dboValue;
            }
            return value;
        }
    }

}
