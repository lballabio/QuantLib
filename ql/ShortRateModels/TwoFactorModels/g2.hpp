
/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file g2.hpp
    \brief Two-factor additive Gaussian Model G2++
*/

#ifndef quantlib_two_factor_models_g2_h
#define quantlib_two_factor_models_g2_h

#include <ql/ShortRateModels/twofactormodel.hpp>
#include <ql/Instruments/swaption.hpp>


namespace QuantLib {

    //! Two-additive-factor gaussian model class.
    /*! This class implements a two-additive-factor model defined by 
        \f[ 
            dr_t = \varphi(t) + x_t + y_t
        \f]
        where \f$ x_t \f$ and \f$ y_t \f$ are defined by
        \f[
            dx_t = -a x_t dt + \sigma dW^1_t, x_0 = 0
        \f]
        \f[
            dy_t = -b y_t dt + \sigma dW^2_t, y_0 = 0
        \f]
        and \f$ dW^1_t dW^2_t = \rho dt \f$.

        \bug This class was not tested enough to guarantee
             its functionality.
    */
    class G2 : public TwoFactorModel, public AffineModel,
               public TermStructureConsistentModel {
      public:
        G2(const RelinkableHandle<TermStructure>& termStructure,
           double a = 0.1, 
           double sigma = 0.01,
           double b = 0.1, 
           double eta = 0.01,
           double rho = 0.9);

        boost::shared_ptr<ShortRateDynamics> dynamics() const;

        double discountBondOption(Option::Type type,
                                  double strike,
                                  Time maturity,
                                  Time bondMaturity) const;
        double swaption(const Swaption::arguments& arguments) const;

        DiscountFactor discount(Time t) const {
            return termStructure()->discount(t);
        }

      protected:
        void generateArguments();

        double A(Time t, Time T) const;
        double B(double x, Time t) const;

      private:
        class Dynamics;
        class FittingParameter;

        double sigmaP(Time t, Time s) const;

        Parameter& a_;
        Parameter& sigma_;
        Parameter& b_;
        Parameter& eta_;
        Parameter& rho_;

        Parameter phi_;

        double V(Time t) const;

        double a() const { return a_(0.0); }
        double sigma() const { return sigma_(0.0); }
        double b() const { return b_(0.0); }
        double eta() const { return eta_(0.0); }
        double rho() const { return rho_(0.0); }

        class SwaptionPricingFunction;
        friend class SwaptionPricingFunction;
    };

    class G2::Dynamics : public TwoFactorModel::ShortRateDynamics {
      public:
        Dynamics(const Parameter& fitting, 
                 double a, double sigma, double b, double eta, double rho)
        : ShortRateDynamics(boost::shared_ptr<DiffusionProcess>(
                                      new OrnsteinUhlenbeckProcess(a, sigma)),
                            boost::shared_ptr<DiffusionProcess>(
                                      new OrnsteinUhlenbeckProcess(b, eta)),
                            rho), 
          fitting_(fitting) {}
        virtual Rate shortRate(Time t, double x, double y) const {
            return fitting_(t) + x + y;
        }
      private:
        Parameter fitting_;
    };

    //! Analytical term-structure fitting parameter \f$ \varphi(t) \f$.
    /*! \f$ \varphi(t) \f$ is analytically defined by
        \f[
            \varphi(t) = f(t) + 
                 \frac{1}{2}(\frac{\sigma(1-e^{-at})}{a})^2 + 
                 \frac{1}{2}(\frac{\eta(1-e^{-bt})}{b})^2 + 
                 \rho\frac{\sigma(1-e^{-at})}{a}\frac{\eta(1-e^{-bt})}{b},
        \f]
        where \f$ f(t) \f$ is the instantaneous forward rate at \f$ t \f$.
    */
    class G2::FittingParameter : public TermStructureFittingParameter {
      private:
        class Impl : public Parameter::Impl {
          public:
            Impl(const RelinkableHandle<TermStructure>& termStructure,
                 double a, double sigma, double b, double eta, 
                 double rho) 
            : termStructure_(termStructure), 
              a_(a), sigma_(sigma), b_(b), eta_(eta), rho_(rho) {}

            double value(const Array& params, Time t) const {
                double forward = termStructure_->instantaneousForward(t);
                double temp1 = sigma_*(1.0-QL_EXP(-a_*t))/a_;
                double temp2 = eta_*(1.0-QL_EXP(-b_*t))/b_;
                double value = 0.5*temp1*temp1 + 0.5*temp2*temp2 +
                    rho_*temp1*temp2 + forward;
                return value;
            }

          private:
            RelinkableHandle<TermStructure> termStructure_;
            double a_, sigma_, b_, eta_, rho_;
        };
      public:
        FittingParameter(const RelinkableHandle<TermStructure>& termStructure,
                         double a, double sigma, double b, 
                         double eta, double rho)
        : TermStructureFittingParameter(boost::shared_ptr<Parameter::Impl>(
                          new FittingParameter::Impl(termStructure, a, sigma, 
                                                     b, eta, rho))) {}
    };

}


#endif
