/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Dimitri Reiswich

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

/*! \file kernelinterpolation.hpp
    \brief Kernel interpolation
*/

#ifndef quantlib_kernel_interpolation_hpp
#define quantlib_kernel_interpolation_hpp

#include <ql/math/interpolation.hpp>
#include <ql/math/kernelfunctions.hpp>
#include <ql/math/matrixutilities/qrdecomposition.hpp>

namespace QuantLib {

    namespace detail {

        template <class I1, class I2>
        class KernelInterpolationImpl
            : public Interpolation::templateImpl<I1,I2> {
          public:
            KernelInterpolationImpl(const I1& xBegin, const I1& xEnd,
                                    const I2& yBegin,
                                    boost::shared_ptr<KernelFunction>& kernel)
            : Interpolation::templateImpl<I1,I2>(xBegin, xEnd, yBegin),
              kernel_(kernel), xSize_(Size(xEnd-xBegin)) {
                updateAlphaVector();
            }

            void update() {
                updateAlphaVector();
            }

            Real value(Real x) const {
                Real sum = 0.0;
                for (Size i=0; i<xSize_; ++i) {
                    sum += alpha_[i]*kernelAbs(x,this->xBegin_[i]);
                }
                return sum/gammaFunc(x);
            }

            Real primitive(Real x) const {
                QL_FAIL("Primitive calculation not implemented "
                        "for kernel interpolation.");
            }

            Real derivative(Real x) const {
                QL_FAIL("First-derivative calculation not implemented "
                        "for kernel interpolation.");
            }

            Real secondDerivative(Real) const {
                QL_FAIL("Second-derivative calculation not implemented "
                        "for kernel interpolation.");
            }

          private:

            Real kernelAbs(Real x1, Real x2)const{
                return (*kernel_)(std::fabs(x1-x2));
            }

            Real gammaFunc(Real x)const{
                Real sum = 0.0;
                for (Size i=0; i<xSize_; ++i) {
                    sum += kernelAbs(x,this->xBegin_[i]);
                }
                return sum;
            }

            void updateAlphaVector(){
                // calculates the alpha vector with given fixed
                // pillars+values

                Matrix M(xSize_,xSize_);

                for (Size rowIt=0; rowIt<xSize_; ++rowIt) {
                    for (Size colIt=0; colIt<xSize_; ++colIt) {
                        M[rowIt][colIt] =
                            kernelAbs(this->xBegin_[rowIt],
                                      this->xBegin_[colIt]) /
                            gammaFunc(this->xBegin_[rowIt]);
                    }
                }

                const std::vector<Real>& y = this->yValues();

                // Solve y=M*\alpha for \alpha
                alpha_ = qrSolve(M, Array(y.begin(), y.end()));
            }

            boost::shared_ptr<KernelFunction> kernel_;
            Array alpha_;
            Size xSize_;
        };

    }


    //! Kernel interpolation between discrete points
    /*! Implementation of the kernel interpolation approach, which can
        be found in "Foreign Exchange Risk" by Hakala, Wystup page
        256.

        The kernel in the implementation is kept general, although a Gaussian
        is considered in the cited text.
    */
    class KernelInterpolation : public Interpolation {
      public:
        /*! \pre the \f$ x \f$ values must be sorted. */
        template <class I1, class I2>
        KernelInterpolation(const I1& xBegin, const I1& xEnd,
                            const I2& yBegin,
                            boost::shared_ptr<KernelFunction>& kernel) {
            impl_ = boost::shared_ptr<Interpolation::Impl>(
                  new detail::KernelInterpolationImpl<I1,I2>(xBegin, xEnd,
                                                             yBegin, kernel));
            impl_->update();
        }
    };

}

#endif
