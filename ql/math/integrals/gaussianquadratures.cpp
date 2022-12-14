/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005 Klaus Spanderen
 Copyright (C) 2005 Gary Kennedy

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

/*! \file gaussianquadratures.hpp
    \brief Integral of a 1-dimensional function using the Gauss quadratures
*/

#include <ql/utilities/null.hpp>
#include <ql/math/integrals/gaussianquadratures.hpp>
#include <ql/math/matrixutilities/tqreigendecomposition.hpp>
#include <ql/math/matrixutilities/symmetricschurdecomposition.hpp>

namespace QuantLib {

    GaussianQuadrature::GaussianQuadrature(
                                Size n,
                                const GaussianOrthogonalPolynomial& orthPoly)
    : x_(n), w_(n) {

        // set-up matrix to compute the roots and the weights
        Array e(n-1);

        Size i;
        for (i=1; i < n; ++i) {
            x_[i] = orthPoly.alpha(i);
            e[i-1] = std::sqrt(orthPoly.beta(i));
        }
        x_[0] = orthPoly.alpha(0);

        TqrEigenDecomposition tqr(
                               x_, e,
                               TqrEigenDecomposition::OnlyFirstRowEigenVector,
                               TqrEigenDecomposition::Overrelaxation);

        x_ = tqr.eigenvalues();
        const Matrix& ev = tqr.eigenvectors();

        Real mu_0 = orthPoly.mu_0();
        for (i=0; i<n; ++i) {
            w_[i] = mu_0*ev[0][i]*ev[0][i] / orthPoly.w(x_[i]);
        }
    }


    namespace detail {
        template <class Integration>
        GaussianQuadratureIntegrator<Integration>::GaussianQuadratureIntegrator(
            Size n)
        : Integrator(Null<Real>(), n),
          integration_(ext::make_shared<Integration>(n))
         {  }

        template <class Integration>
        Real GaussianQuadratureIntegrator<Integration>::integrate(
            const ext::function<Real (Real)>& f, Real a, Real b) const {

            const Real c1 = 0.5*(b-a);
            const Real c2 = 0.5*(a+b);

            return c1*integration_->operator()(
                [c1, c2, f](Real x) { return f(c1*x+c2);});
        }

        template class GaussianQuadratureIntegrator<GaussLegendreIntegration>;
        template class GaussianQuadratureIntegrator<GaussChebyshevIntegration>;
        template class GaussianQuadratureIntegrator<GaussChebyshev2ndIntegration>;
    }

    void TabulatedGaussLegendre::order(Size order) {
        switch(order) {
          case(6):
            order_=order; x_=x6; w_=w6; n_=n6;
            break;
          case(7):
            order_=order; x_=x7; w_=w7; n_=n7;
            break;
          case(12):
            order_=order; x_=x12; w_=w12; n_=n12;
            break;
          case(20):
            order_=order; x_=x20; w_=w20; n_=n20;
            break;
          default:
            QL_FAIL("order " << order << " not supported");
        }
    }


    // Abscissas and Weights from Abramowitz and Stegun

    /* order 6 */
    const Real TabulatedGaussLegendre::x6[3] = { 0.238619186083197,
                                                 0.661209386466265,
                                                 0.932469514203152 };

    const Real TabulatedGaussLegendre::w6[3] = { 0.467913934572691,
                                                 0.360761573048139,
                                                 0.171324492379170 };

    const Size TabulatedGaussLegendre::n6 = 3;

    /* order 7 */
    const Real TabulatedGaussLegendre::x7[4] = { 0.000000000000000,
                                                 0.405845151377397,
                                                 0.741531185599394,
                                                 0.949107912342759 };

    const Real TabulatedGaussLegendre::w7[4] = { 0.417959183673469,
                                                 0.381830050505119,
                                                 0.279705391489277,
                                                 0.129484966168870 };

    const Size TabulatedGaussLegendre::n7 = 4;

    /* order 12 */
    const Real TabulatedGaussLegendre::x12[6] = { 0.125233408511469,
                                                  0.367831498998180,
                                                  0.587317954286617,
                                                  0.769902674194305,
                                                  0.904117256370475,
                                                  0.981560634246719 };

    const Real TabulatedGaussLegendre::w12[6] = { 0.249147045813403,
                                                  0.233492536538355,
                                                  0.203167426723066,
                                                  0.160078328543346,
                                                  0.106939325995318,
                                                  0.047175336386512 };

    const Size TabulatedGaussLegendre::n12 = 6;

    /* order 20 */
    const Real TabulatedGaussLegendre::x20[10] = { 0.076526521133497,
                                                   0.227785851141645,
                                                   0.373706088715420,
                                                   0.510867001950827,
                                                   0.636053680726515,
                                                   0.746331906460151,
                                                   0.839116971822219,
                                                   0.912234428251326,
                                                   0.963971927277914,
                                                   0.993128599185095 };

    const Real TabulatedGaussLegendre::w20[10] = { 0.152753387130726,
                                                   0.149172986472604,
                                                   0.142096109318382,
                                                   0.131688638449177,
                                                   0.118194531961518,
                                                   0.101930119817240,
                                                   0.083276741576704,
                                                   0.062672048334109,
                                                   0.040601429800387,
                                                   0.017614007139152 };

    const Size TabulatedGaussLegendre::n20 = 10;

}
