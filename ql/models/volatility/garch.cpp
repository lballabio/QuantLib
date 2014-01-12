/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Joseph Wang
 Copyright (C) 2012 Liquidnet Holdings, Inc.

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

#include <ql/models/volatility/garch.hpp>
#include <ql/math/optimization/leastsquare.hpp>
#include <ql/math/optimization/simplex.hpp>
#include <ql/math/autocovariance.hpp>
#include <boost/foreach.hpp>

namespace QuantLib {

    namespace {

        const Real tol_level = 1.0e-8;

        class Garch11Constraint : public Constraint {
          private:
            class Impl : public Constraint::Impl {
                Real gammaLower_, gammaUpper_;
              public:
                Impl (Real gammaLower, Real gammaUpper)
                : gammaLower_(gammaLower), gammaUpper_(gammaUpper) {}
                bool test(const Array &x) const {
                    QL_REQUIRE(x.size() >= 3, "size of parameters vector < 3");
                    return x[0] > 0 && x[1] >= 0 && x[2] >= 0
                        && x[1] + x[2] < gammaUpper_
                        && x[1] + x[2] >= gammaLower_;
                }
            };
          public:
            Garch11Constraint(Real gammaLower, Real gammaUpper)
            : Constraint(boost::shared_ptr<Constraint::Impl>(
                      new Garch11Constraint::Impl(gammaLower, gammaUpper))) {}
        };


        class Garch11CostFunction : public CostFunction {
          public:
            Garch11CostFunction (const std::vector<Volatility> &);
            virtual Real value(const Array& x) const;
            virtual Disposable<Array> values(const Array& x) const;
            virtual void gradient(Array& grad, const Array& x) const;
            virtual Real valueAndGradient(Array& grad, const Array& x) const;
          private:
            const std::vector<Volatility> &r2_;
        };

        Garch11CostFunction::Garch11CostFunction(
                                            const std::vector<Volatility> &r2)
        : r2_(r2) {}

        Real Garch11CostFunction::value(const Array& x) const {
            Real retval(0.0);
            Real sigma2 = 0;
            Real u2 = 0;
            BOOST_FOREACH (Volatility r2, r2_) {
                sigma2 = x[0] + x[1] * u2 + x[2] * sigma2;
                u2 = r2;
                retval += std::log(sigma2) + u2 / sigma2;
            }
            return retval / (2.0*r2_.size());
        }

        Disposable<Array> Garch11CostFunction::values(const Array& x) const {
            Array retval (r2_.size());
            Real sigma2 = 0;
            Real u2 = 0;
            Size i = 0;
            BOOST_FOREACH (Volatility r2, r2_) {
                sigma2 = x[0] + x[1] * u2 + x[2] * sigma2;
                u2 = r2;
                retval[i++] = (std::log(sigma2) + u2 / sigma2)/(2.0*r2_.size());
            }
            return retval;
        }

        void Garch11CostFunction::gradient(Array& grad, const Array& x) const {
            std::fill (grad.begin(), grad.end(), 0.0);
            Real sigma2 = 0;
            Real u2 = 0;
            Real sigma2prev = sigma2;
            Real u2prev = u2;
            Real norm = 2.0 * r2_.size();
            BOOST_FOREACH (Volatility r2, r2_) {
                sigma2 = x[0] + x[1] * u2 + x[2] * sigma2;
                u2 = r2;
                Real w = (sigma2 - u2) / (sigma2*sigma2);
                grad[0] += w;
                grad[1] += u2prev * w;
                grad[2] += sigma2prev * w;
                u2prev = u2;
                sigma2prev = sigma2;
            }
            std::transform(grad.begin(), grad.end(), grad.begin(),
                           std::bind2nd(std::divides<Real>(), norm));
        }

        Real Garch11CostFunction::valueAndGradient(Array& grad,
                                                   const Array& x) const {
            std::fill (grad.begin(), grad.end(), 0.0);
            Real retval(0.0);
            Real sigma2 = 0;
            Real u2 = 0;
            Real sigma2prev = sigma2;
            Real u2prev = u2;
            Real norm = 2.0 * r2_.size();
            BOOST_FOREACH (Volatility r2, r2_) {
                sigma2 = x[0] + x[1] * u2 + x[2] * sigma2;
                u2 = r2;
                retval += std::log(sigma2) + u2 / sigma2;
                Real w = (sigma2 - u2) / (sigma2*sigma2);
                grad[0] += w;
                grad[1] += u2prev * w;
                grad[2] += sigma2prev * w;
                u2prev = u2;
                sigma2prev = sigma2;
            }
            std::transform(grad.begin(), grad.end(), grad.begin(),
                           std::bind2nd(std::divides<Real>(), norm));
            return retval / norm;
        }


        class FitAcfProblem : public LeastSquareProblem {
          public:
            FitAcfProblem(Real A2, const Array &acf,
                          const std::vector<std::size_t> &idx);
            virtual Size size();
            virtual void targetAndValue(const Array& x, Array& target,
                                        Array& fct2fit);
            virtual void targetValueAndGradient(const Array& x,
                                                Matrix& grad_fct2fit,
                                                Array& target, Array& fct2fit);
          private:
            Real A2_;
            Array acf_;
            std::vector<std::size_t> idx_;
        };

        FitAcfProblem::FitAcfProblem(Real A2, const Array &acf,
                                     const std::vector<std::size_t> &idx)
        : A2_(A2), acf_(acf), idx_(idx) {}

        Size FitAcfProblem::size() { return idx_.size(); }

        void FitAcfProblem::targetAndValue(const Array& x, Array& target,
                                           Array& fct2fit) {
            Real A4 = acf_[0] + A2_*A2_;
            Real gamma = x[0];
            Real beta = x[1];
            target[0] = A2_*A2_/A4;
            fct2fit[0] =
                (1 - 3*gamma*gamma - 2*beta*beta + 4*beta*gamma)
                / (3*(1 - gamma*gamma));
            target[1] = acf_[1] / A4;
            fct2fit[1] = gamma * (1 - fct2fit[0]) - beta;
            for (std::size_t i = 2; i < idx_.size(); ++i) {
                target[i] = acf_[idx_[i]] / A4;
                fct2fit[i] = std::pow(gamma, (int)idx_[i]-1)* fct2fit[1];
            }
        }

        void FitAcfProblem::targetValueAndGradient(const Array& x,
                                                   Matrix& grad_fct2fit,
                                                   Array& target,
                                                   Array& fct2fit) {
            Real A4 = acf_[0] + A2_*A2_;
            Real gamma = x[0];
            Real beta = x[1];
            target[0] = A2_*A2_/A4;
            Real w1 = (1 - 3*gamma*gamma - 2*beta*beta + 4*beta*gamma);
            Real w2 = (1 - gamma*gamma);
            fct2fit[0] = w1 / (3*w2);
            grad_fct2fit[0][0] = (2.0/3.0) * ((2*beta-3*gamma)*w2 + 2*w1*gamma) / (w2*w2);
            grad_fct2fit[0][1] = (4.0/3.0) * (gamma - beta) / w2;
            target[1] = acf_[1] / A4;
            fct2fit[1] = gamma * (1 - fct2fit[0]) - beta;
            grad_fct2fit[1][0] = (1 - fct2fit[0]) - gamma * grad_fct2fit[0][0];
            grad_fct2fit[1][1] = -gamma * grad_fct2fit[0][1] - 1;
            for (std::size_t i = 2; i < idx_.size(); ++i) {
                target[i] = acf_[idx_[i]] / A4;
                w1 = std::pow(gamma, (int)idx_[i]-1);
                fct2fit[i] = w1 * fct2fit[1];
                grad_fct2fit[i][0] = (idx_[i]-1) * (w1/gamma)*fct2fit[1] + w1*grad_fct2fit[1][0];
                grad_fct2fit[i][1] = w1 * grad_fct2fit[1][1];
            }
        }


        class FitAcfConstraint : public Constraint {
          private:
            class Impl : public Constraint::Impl {
                Real gammaLower_, gammaUpper_;
              public:
                Impl(Real gammaLower, Real gammaUpper)
                : gammaLower_(gammaLower), gammaUpper_(gammaUpper) {}
                bool test(const Array &x) const {
                    QL_REQUIRE(x.size() >= 2, "size of parameters vector < 2");
                    return x[0] >= gammaLower_ && x[0] < gammaUpper_
                        && x[1] >= 0 && x[1] <= x[0];
                }
            };
          public:
            FitAcfConstraint(Real gammaLower, Real gammaUpper)
            : Constraint(boost::shared_ptr<Constraint::Impl>(
                       new FitAcfConstraint::Impl(gammaLower, gammaUpper))) {}
        };


        // Initial guess based on fitting ACF - initial guess for
        // fitting acf is a moment matching estimates for mean(r2),
        // acf(0), and acf(1).
        Real initialGuess1(const Array &acf, Real mean_r2,
                           Real &alpha, Real &beta, Real &omega) {
            Real A21 = acf[1];
            Real A4 = acf[0] + mean_r2*mean_r2;

            Real A = mean_r2*mean_r2/A4; // 1/sigma^2
            Real B = A21 / A4; // rho(1)

            Real gammaLower = A <= 1./3. - tol_level ? std::sqrt((1 - 3*A)/(3 - 3*A)) + tol_level : tol_level;
            Garch11Constraint constraints(gammaLower, 1.0 - tol_level);

            Real gamma = gammaLower + (1 - gammaLower) * 0.5;
            beta = std::min(gamma, std::max(gamma * (1 - A) - B, 0.0));
            alpha = gamma - beta;
            omega = mean_r2 * (1 - gamma);

            if (std::fabs(A-0.5) < QL_EPSILON) {
                gamma = std::max(gammaLower, -(1+4*B*B)/(4*B));
                beta = std::min(gamma, std::max(gamma * (1 - A) - B, 0.0));
                alpha = gamma - beta;
                omega = mean_r2 * (1 - gamma);
            } else {
                if (A > 1.0 - QL_EPSILON) {
                    gamma = std::max(gammaLower, -(1+B*B)/(2*B));
                    beta = std::min(gamma, std::max(gamma * (1 - A) - B, 0.0));
                    alpha = gamma - beta;
                    omega = mean_r2 * (1 - gamma);
                } else {
                    Real D = (3*A-1)*(2*B*B+(1-A)*(2*A-1));
                    if (D >= 0) {
                        Real d = std::sqrt(D);
                        Real b = (B - d)/(2*A-1);
                        Real g = 0;
                        if (b >= tol_level && b <= 1.0 - tol_level) {
                            g = (b + B) / (1 - A);
                        }
                        if (g < gammaLower) {
                            b = (B + d)/(2*A-1);
                            if (b >= tol_level && b <= 1.0 - tol_level) {
                                g = (b + B) / (1 - A);
                            }
                        }
                        if (g >= gammaLower) {
                            gamma = g;
                            beta = std::min(gamma, std::max(gamma * (1 - A) - B, 0.0));
                            alpha = gamma - beta;
                            omega = mean_r2 * (1 - gamma);
                        }
                    }
                }
            }

            std::vector<std::size_t> idx;
            std::size_t nCov = acf.size() - 1;
            for (std::size_t i = 0; i <= nCov; ++i) {
                if (i < 2 || (i > 1 && acf[i] > 0 && acf[i-1] > 0 && acf[i-1] > acf[i])) {
                    idx.push_back(i);
                }
            }

            Array x(2);
            x[0] = gamma;
            x[1] = beta;

            try {
                FitAcfConstraint c(gammaLower, 1.0 - tol_level);
                NonLinearLeastSquare nnls(c);
                nnls.setInitialValue(x);
                FitAcfProblem pr(mean_r2, acf, idx);
                x = nnls.perform(pr);
                Array guess(3);
                guess[0] = mean_r2 * (1 - x[0]);
                guess[1] = x[0] - x[1];
                guess[2] = x[1];
                if (constraints.test(guess)) {
                    omega = guess[0];
                    alpha = guess[1];
                    beta = guess[2];
                }
            } catch (const std::exception &) {
                // failed -- returning initial values
            }
            return gammaLower;
        }

        // Initial guess based on fitting ACF - initial guess for
        // fitting acf is an estimate of gamma = alpfa+beta based on
        // the property: acf(i+1) = gamma*acf(i) for i > 1.
        Real initialGuess2 (const Array &acf, Real mean_r2,
                            Real &alpha, Real &beta, Real &omega) {
            Real A21 = acf[1];
            Real A4 = acf[0] + mean_r2*mean_r2;
            Real A = mean_r2*mean_r2/A4; // 1/sigma^2
            Real B = A21 / A4; // rho(1)
            Real gammaLower = A <= 1./3. - tol_level ? std::sqrt((1 - 3*A)/(3 - 3*A)) + tol_level : tol_level;
            Garch11Constraint constraints(gammaLower, 1.0 - tol_level);

            // ACF
            Real gamma = 0;
            std::size_t nn = 0;
            std::vector<std::size_t> idx;
            std::size_t nCov = acf.size() - 1;
            for (std::size_t i = 0; i <= nCov; ++i) {
                if (i < 2) idx.push_back(i);
                if (i > 1 && acf[i] > 0 && acf[i-1] > 0 && acf[i-1] > acf[i]) {
                    gamma += acf[i]/acf[i-1];
                    nn++;
                    idx.push_back(i);
                }
            }
            if (nn > 0)
                gamma /= nn;
            if (gamma < gammaLower) gamma = gammaLower;
            beta = std::min(gamma, std::max(gamma * (1 - A) - B, 0.0));
            omega = mean_r2 * (1 - gamma);

            Array x(2);
            x[0] = gamma;
            x[1] = beta;

            try {
                FitAcfConstraint c(gammaLower, 1 - tol_level);
                NonLinearLeastSquare nnls(c);
                nnls.setInitialValue(x);
                FitAcfProblem pr(mean_r2, acf, idx);
                x = nnls.perform(pr);
                Array guess(3);
                guess[0] = mean_r2 * (1 - x[0]);
                guess[1] = x[0] - x[1];
                guess[2] = x[1];
                if (constraints.test(guess)) {
                    omega = guess[0];
                    alpha = guess[1];
                    beta = guess[2];
                }
            } catch (const std::exception &) {
                // failed -- returning initial values
            }
            return gammaLower;
        }

    }

    Garch11::time_series
    Garch11::calculate(const time_series& quoteSeries,
                       Real alpha, Real beta, Real omega) {
        time_series retval;
        const_iterator cur = quoteSeries.cbegin();
        Real u = cur->second;
        Real sigma2 = u*u;
        while (++cur != quoteSeries.end()) {
            sigma2 = omega + alpha * u * u + beta * sigma2;
            retval[cur->first] = std::sqrt(sigma2);
            u = cur->second;
        }
        sigma2 = omega + alpha * u * u + beta * sigma2;
        --cur;
        const_iterator prev = cur;
        retval[cur->first + (cur->first - (--prev)->first) ] = std::sqrt(sigma2);
        return retval;
    }


    boost::shared_ptr<Problem> Garch11::calibrate_r2(
                   Mode mode, const std::vector<Volatility> &r2, Real mean_r2,
                   Real &alpha, Real &beta, Real &omega) {
        EndCriteria endCriteria(10000, 500, tol_level, tol_level, tol_level);
        Simplex method(0.001);
        return calibrate_r2(mode, r2, mean_r2, method, endCriteria,
                            alpha, beta, omega);
    }

    boost::shared_ptr<Problem> Garch11::calibrate_r2(
                   Mode mode, const std::vector<Volatility> &r2, Real mean_r2,
                   OptimizationMethod &method, const EndCriteria &endCriteria,
                   Real &alpha, Real &beta, Real &omega) {
        Real dataSize = Real(r2.size());
        alpha = 0.0;
        beta = 0.0;
        omega = 0.0;
        QL_REQUIRE (dataSize >= 4,
                    "Data series is too short to fit GARCH model");
        QL_REQUIRE (mean_r2 > 0, "Data series is constant");
        omega = mean_r2 * dataSize / (dataSize - 1);

        // ACF
        Size maxLag = (Size)std::sqrt(dataSize);
        Array acf(maxLag+1);
        std::vector<Volatility> tmp(r2.size());
        std::transform (r2.begin(), r2.end(), tmp.begin(),
                        std::bind2nd(std::minus<double>(), mean_r2));
        autocovariances (tmp.begin(), tmp.end(), acf.begin(), maxLag);
        QL_REQUIRE (acf[0] > 0, "Data series is constant");

        Garch11CostFunction cost (r2);

        // two initial guesses based on fitting ACF
        Real gammaLower = 0.0;
        Array opt1(3);
        Real fCost1 = QL_MAX_REAL;
        if (mode != GammaGuess) {
            gammaLower = initialGuess1(acf, mean_r2, opt1[1], opt1[2], opt1[0]);
            fCost1 = cost.value(opt1);
        }

        Array opt2(3);
        Real fCost2 = QL_MAX_REAL;
        if (mode != MomentMatchingGuess) {
            gammaLower = initialGuess2(acf, mean_r2, opt2[1], opt2[2], opt2[0]);
            fCost2 = cost.value(opt2);
        }

        Garch11Constraint constraints(gammaLower, 1.0 - tol_level);

        boost::shared_ptr<Problem> ret;
        if (mode != DoubleOptimization) {
            try {
                ret = calibrate_r2(r2, method, constraints, endCriteria,
                                   fCost1 <= fCost2 ? opt1 : opt2,
                                   alpha, beta, omega);
            } catch (const std::exception &) {
                if (fCost1 <= fCost2) {
                    alpha = opt1[1];
                    beta = opt1[2];
                    omega = opt1[0];
                } else {
                    alpha = opt2[1];
                    beta = opt2[2];
                    omega = opt2[0];
                }
            }
        } else {
            boost::shared_ptr<Problem> ret1, ret2;
            try {
                ret1 = calibrate_r2(r2, method, constraints, endCriteria,
                                    opt1, alpha, beta, omega);
                opt1[1] = alpha;
                opt1[2] = beta;
                opt1[0] = omega;
                double fCost = QL_MAX_REAL;
                if (constraints.test(opt1) && (fCost = cost.value(opt1)) < fCost1)
                    fCost1 = fCost;
            } catch (const std::exception &) {
                fCost1 = QL_MAX_REAL;
            }

            try {
                ret2 = calibrate_r2(r2, method, constraints, endCriteria,
                                    opt2, alpha, beta, omega);
                opt2[1] = alpha;
                opt2[2] = beta;
                opt2[0] = omega;
                double fCost = QL_MAX_REAL;
                if (constraints.test(opt2) && (fCost = cost.value(opt2)) < fCost2)
                    fCost2 = fCost;
            } catch (const std::exception &) {
                fCost2 = QL_MAX_REAL;
            }

            if (fCost1 <= fCost2) {
                alpha = opt1[1];
                beta = opt1[2];
                omega = opt1[0];
                ret = ret1;
            } else {
                alpha = opt2[1];
                beta = opt2[2];
                omega = opt2[0];
                ret = ret2;
            }
        }
        return ret;
    }

    boost::shared_ptr<Problem> Garch11::calibrate_r2(
               const std::vector<Volatility> &r2,
               OptimizationMethod &method,
               const EndCriteria &endCriteria,
               const Array &initGuess, Real &alpha, Real &beta, Real &omega) {
        Garch11Constraint constraints(0.0, 1.0 - tol_level);
        return calibrate_r2(r2, method, constraints, endCriteria,
                            initGuess, alpha, beta, omega);
    }

    boost::shared_ptr<Problem> Garch11::calibrate_r2(
               const std::vector<Volatility> &r2,
               Real mean_r2,
               OptimizationMethod &method,
               const EndCriteria &endCriteria,
               const Array &initGuess, Real &alpha, Real &beta, Real &omega) {
        std::vector<Volatility> tmp(r2.size());
        std::transform (r2.begin(), r2.end(), tmp.begin(),
                        std::bind2nd(std::minus<double>(), mean_r2));
        return calibrate_r2(tmp, method, endCriteria, initGuess,
                            alpha, beta, omega);
    }

    boost::shared_ptr<Problem> Garch11::calibrate_r2(
               const std::vector<Volatility> &r2,
               OptimizationMethod &method,
               Constraint &constraints,
               const EndCriteria &endCriteria,
               const Array &initGuess, Real &alpha, Real &beta, Real &omega) {
        Garch11CostFunction cost(r2);
        boost::shared_ptr<Problem> problem(
                               new Problem(cost, constraints, initGuess));
        // TODO: check return value from minimize()
        /* EndCriteria::Type ret = */
        method.minimize(*problem, endCriteria);
        const Array &optimum = problem->currentValue();
        alpha = optimum[1];
        beta = optimum[2];
        omega = optimum[0];
        return problem;
    }

    boost::shared_ptr<Problem> Garch11::calibrate_r2(
               const std::vector<Volatility> &r2,
               Real mean_r2,
               OptimizationMethod &method,
               Constraint &constraints,
               const EndCriteria &endCriteria,
               const Array &initGuess, Real &alpha, Real &beta, Real &omega) {
        std::vector<Volatility> tmp(r2.size());
        std::transform (r2.begin(), r2.end(), tmp.begin(),
                        std::bind2nd(std::minus<double>(), mean_r2));
        return calibrate_r2(tmp, method, constraints, endCriteria,
                            initGuess, alpha, beta, omega);
    }

}

