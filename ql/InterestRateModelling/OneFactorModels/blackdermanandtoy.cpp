
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

/*! \file onefactormodel.hpp
    \brief Abstract one-factor interest rate model class

    \fullpath
    ql/%onefactormodel.hpp
*/

// $Id$

#include "ql/InterestRateModelling/OneFactorModels/blackdermanandtoy.hpp"
#include "ql/Solvers1D/brent.hpp"
using std::cout;
using std::endl;

namespace QuantLib {

    namespace InterestRateModelling {

        class BlackDermanAndToy::Process : public StochasticProcess {
          public:
            Process(BlackDermanAndToy * model) 
            : StochasticProcess(LogShortRate), model_(model) {}

            virtual double drift(double lnR, double t) const {
                return model_->theta(t) - model_->sigmaPrime(t)/model_->sigma(t)*lnR;
            }
            virtual double diffusion(double lnR, Time t) const {
                return model_->sigma_;
            }
          private:
            BlackDermanAndToy * model_;
        };

        BlackDermanAndToy::BlackDermanAndToy(
            const RelinkableHandle<TermStructure>& termStructure, 
            size_t timeSteps) 
        : OneFactorModel(1, termStructure), timeSteps_(timeSteps) {
            process_ = Handle<StochasticProcess>(new Process(this));

            dt_ = termStructure->maxTime()/(timeSteps_*1.0);

            u_.resize(timeSteps_+1);
            theta_.resize(timeSteps_+1);
            statePrices_.resize(timeSteps_+1);
            discountFactors_.resize(timeSteps_+1);
            for  (unsigned i=0; i<=timeSteps_; i++) {
                statePrices_[i].resize(i+1);
                discountFactors_[i].resize(i+1);
            }


            constraint_ = Handle<Constraint>(new Constraint(
                  std::vector<double>(1, 0.0),
                  std::vector<double>(1, 1.0)));

            sigma_ = 0.10;
            initializeTree();
        }

        double BlackDermanAndToy::discountBond(Time now, Time maturity, 
            Rate r) {

            unsigned iNow = (unsigned)(now/dt_);
            unsigned iMaturity = (unsigned)(maturity/dt_);
            if (iMaturity > iMax_)
                calculateTree(iMaturity);
            signed iDiff = iMaturity - iNow;
            signed jr = (signed)(QL_LOG(r/u_[iNow])/(sigma_*QL_SQRT(dt_)));
            std::vector<double> prices(iDiff+1, 1.0);
            for (signed i=iDiff; i>=0; i--) {
                unsigned index = 0;
                for (signed j=-i; j<=i; j+=2) {
                    double discountFactor = 1.0/(1.0 + u_[i+iNow]*
                        QL_EXP(sigma_*(jr+j)*QL_SQRT(dt_))*dt_);
                    prices[index] = 0.5*discountFactor*(prices[index] + 
                        prices[index+1]);
                    index++;
                }
            }
            return prices[0];
        }

        double BlackDermanAndToy::discountBondOption(
          Option::Type type, double strike, Time maturity, Time bondMaturity) {
            unsigned iBond = (unsigned)(bondMaturity/dt_);
            unsigned iOption = (unsigned)(maturity/dt_);

            if (iBond > iMax_)
                calculateTree(iBond);
 
            std::vector<double> dbValues(iBond + 1, 1.0);
            for (signed i=(signed)(iBond-1); i>=(signed)iOption; i--) {
                unsigned index = 0;
                for (signed j=-i; j<=i; j+=2) {
                    double discountFactor = 1.0/(1.0 + u_[i]*
                        QL_EXP(sigma_*(j)*QL_SQRT(dt_))*dt_);
                    dbValues[index] = 0.5*discountFactor*
                        (dbValues[index] + dbValues[index+1]);
                    index++;
                }
            }
            dbValues.resize(iOption+1);

            double value = 0.0;

            for (unsigned j=0; j<dbValues.size(); j++) {
                double payoff = 0.0;

                switch(type) {
                  case Option::Call:
                    payoff = QL_MAX(0.0, dbValues[j] - strike);
                    break;
                    
                  case Option::Put:
                    payoff = QL_MAX(0.0, strike - dbValues[j]);
                    break;

                  default:
                    throw Error("unsupported option type");
                }
                value += statePrices_[iOption][j]*payoff;
            }

            return value;
        }

        class BlackDermanAndToy::PrivateFunction : public ObjectiveFunction {
          public:
            PrivateFunction(BlackDermanAndToy* bdt, 
                const std::vector<double>& statePrices, 
                double discountBondPrice);
            double operator()(double x) const;
          private: 
            const std::vector<double>& statePrices_;
            double discountBondPrice_;
            std::vector<double> helper_;
            size_t nit_;
        };      
            
        inline BlackDermanAndToy::PrivateFunction::PrivateFunction( 
            BlackDermanAndToy *bdt,
            const std::vector<double>& statePrices, double discountBondPrice) 
          : statePrices_(statePrices), discountBondPrice_(discountBondPrice) {
                nit_ = statePrices.size();
                helper_.resize(nit_);

                signed i = nit_ - 1;
                double sigma = bdt->sigma_;
                double dt = bdt->dt_;
                unsigned index = 0;
                for (signed j=-i; j<=i; j+=2)
                    helper_[index++] = QL_EXP(sigma*j*QL_SQRT(dt))*dt;
        }

        inline double BlackDermanAndToy::PrivateFunction::operator()(double x) 
            const {
            double value = discountBondPrice_;
            for (unsigned k=0; k<nit_; k++)
                value -= statePrices_[k]/(1.0 + x*helper_[k]);
            return value;
        }

        void BlackDermanAndToy::initializeTree() {
            cout << "Initializing tree for alpha = " << sigma_*100.0 << "%" << endl;
            double r0 = termStructure()->forward(0.0);

            statePrices_[0][0] = 1.0;
            discountFactors_[0][0] = 1.0/(1.0 + r0*dt_);
            u_[0] = r0;
            iMax_ = 0;
        }

        void BlackDermanAndToy::calculateTree(size_t newMax) {
            QL_REQUIRE(newMax>iMax_, "Tree already built");

            for (size_t i=iMax_+1; i<=newMax; i++) {
                //Compute state prices for t_i
                statePrices_[i][i]= 0.5*statePrices_[i-1][i-1]*
                    discountFactors_[i-1][i-1];
                for (size_t k=(i-1); k>=1; k--) {
                    statePrices_[i][k] = 0.5*
                        (statePrices_[i-1][k]*discountFactors_[i-1][k] +
                        statePrices_[i-1][k-1]*discountFactors_[i-1][k-1]);
                }
                statePrices_[i][0]= 0.5*statePrices_[i-1][0]*
                    discountFactors_[i-1][0];

                //Compute u(t_i)
                double discountBond = termStructure()->discount(dt_*(i+1));
                PrivateFunction finder(this, statePrices_[i], discountBond);
                Solvers1D::Brent s1d = Solvers1D::Brent();
                double minStrike = 0.001;
                double maxStrike = 0.200;
                double accuracy = 1e-5;
                double startValue = u_[i-1];
                s1d.setMaxEvaluations(1000);
                s1d.setLowBound(minStrike);
                s1d.setHiBound(maxStrike);
                u_[i] = s1d.solve(finder, accuracy, startValue, minStrike, 
                    maxStrike);

                //Compute discount factors for t_i
                size_t index = 0;
                for (int j=-int(i); j<=int(i); j+=2) {
                    discountFactors_[i][index] = 1.0/(1.0 + u_[i]*
                      QL_EXP(sigma_*j*QL_SQRT(dt_))*dt_);
                    index++;
                }

            }

            if (iMax_>0)
                theta_[iMax_] = 0.5*(u_[iMax_+1] - u_[iMax_-1])/dt_;
            else
                theta_[0] = (u_[1]  - u_[0])/dt_;
            for (size_t k=iMax_+1; k<newMax; k++)
                theta_[k] = 0.5*(u_[k+1] - u_[k-1])/dt_;
            theta_[newMax] = (u_[newMax] - u_[newMax-1])/dt_;

            iMax_ = newMax;
        }

        double BlackDermanAndToy::theta(Time t) {
            size_t low = (unsigned int)(t/dt_);
            if ((low+1)>iMax_)
                calculateTree(low+1);
            double weight = t/dt_ - low*1.0;
            return theta_[low]*weight + theta_[low+1]*(1-weight);
        }

 
    }
}
