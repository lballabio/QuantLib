
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

/*! \file coxingersollrossplus.cpp
    \brief CIR++ model

    \fullpath
    ql/InterestRateModelling/OneFactorModels/%coxingersollrossplus.cpp
*/

// $Id$

#include "ql/InterestRateModelling/OneFactorModels/coxingersollrossplus.hpp"
#include "ql/Math/normaldistribution.hpp"
#include "ql/InterestRateModelling/trinomialtree.hpp"

namespace QuantLib {

    namespace InterestRateModelling {

        using Lattices::TimeGrid;

        class CoxIngersollRossPlus::Process : public ShortRateProcess {
          public:
            Process(CoxIngersollRossPlus * model) 
            : model_(model), k_(model_->k_), 
              theta_(model_->theta_), sigma_(model_->sigma_), y0_(0.0059379) {}

            virtual double variable(Time t, Rate r) const { 
                return QL_SQRT(r - model_->phi(t)) - y0_; 
            }
            virtual double shortRate(Time t, double y) const { 
                return (y+y0_)*(y+y0_) + model_->phi(t); 
            }
            virtual double drift(Time t, double y) const {
                return (0.5*theta_*k_ + 0.125*sigma_*sigma_)/(y+y0_) - 
                    0.5*k_*(y+y0_);
            }
            virtual double diffusion(Time t, double y) const {
                return 0.5*sigma_;
            }
          private:
            CoxIngersollRossPlus * model_;
            const double& k_;
            const double& theta_;
            const double& sigma_;
            double y0_;
        };

        CoxIngersollRossPlus::CoxIngersollRossPlus(
            const RelinkableHandle<TermStructure>& termStructure) 
        : OneFactorModel(3, termStructure), k_(params_[0]),
          theta_(params_[1]), sigma_(params_[2]) {
            process_ = Handle<ShortRateProcess>(new Process(this));
            constraint_ = Handle<Constraint>(new Constraint(3));
            constraint_->setLowerBound(0, 0.000001);
            constraint_->setLowerBound(1, 0.000001);
            constraint_->setLowerBound(2, 0.000001);
        }

        double CoxIngersollRossPlus::phi(Time t) const {
            double forwardRate = termStructure()->forward(t);
            double h = QL_SQRT(k_*k_ + 2*sigma_*sigma_);
            double expth = QL_EXP(t*h);
            double phi = forwardRate - 
                k_*sigma_*(expth - 1.0)/(2.0*h + (k_+h)*(expth - 1.0));
            return phi;
        }

        double CoxIngersollRossPlus::A(Time t, Time T) const {
            double h = QL_SQRT(k_*k_ + 2.0*sigma_*sigma_);
            double numerator = 2.0*h*QL_EXP(0.5*(k_+h)*(T-t));
            double denominator = 2.0*h + (k_+h)*(QL_EXP((T - t)*h) - 1.0);
            double value = 2.0*k_*theta_/(sigma_*sigma_)*
                QL_LOG(numerator/denominator);
            return QL_EXP(value);
        }

        double CoxIngersollRossPlus::B(Time t, Time T) const {
            double h = QL_SQRT(k_*k_ + 2.0*sigma_*sigma_);
            double numerator = 2.0*(QL_EXP((T-t)*h) - 1.0);
            double denominator = 2.0*h + (k_+h)*(QL_EXP((T - t)*h) - 1.0);
            double value = numerator/denominator;
            return value;
        }

        double CoxIngersollRossPlus::C(Time t, Time T) const {
            double Pt = termStructure()->discount(t);
            double PT = termStructure()->discount(T);
            double value = (Pt*A(0,t))/(PT*A(0,T))*A(t,T)*
                QL_EXP(B(t,T)*phi(t));
            return value;
        }

        double CoxIngersollRossPlus::discountBond(Time t, Time T, Rate r) {
            double value =  C(t,T)*QL_EXP(B(t,T)*r);
            return value;
        }

    }

}
