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


#ifndef quantlib_kernel_interpolation_hpp
#define quantlib_kernel_interpolation_hpp

#include <ql/math/interpolation.hpp>
#include <ql/math/matrixutilities/qrdecomposition.hpp>
#include <utility>

/*! \file kernelinterpolation.hpp
    \brief Kernel interpolation
*/

namespace QuantLib {

    namespace detail {

        template <class I1, class I2, class Kernel>
        class KernelInterpolationImpl
            : public Interpolation::templateImpl<I1,I2> {
          public:
            KernelInterpolationImpl(const I1& xBegin,
                                    const I1& xEnd,
                                    const I2& yBegin,
                                    Kernel kernel,
                                    const Real epsilon)
            : Interpolation::templateImpl<I1, I2>(xBegin, xEnd, yBegin),
              xSize_(Size(xEnd - xBegin)), invPrec_(epsilon), M_(xSize_, xSize_), alphaVec_(xSize_),
              yVec_(xSize_), kernel_(std::move(kernel)) {}

            void update() override { updateAlphaVec(); }

            Real value(Real x) const override {

                Real res=0.0;

                for( Size i=0; i< xSize_;++i){
                    res+=alphaVec_[i]*kernelAbs(x,this->xBegin_[i]);
                }

                return res/gammaFunc(x);
            }

            Real primitive(Real) const override {
                QL_FAIL("Primitive calculation not implemented "
                        "for kernel interpolation");
            }

            Real derivative(Real) const override {
                QL_FAIL("First derivative calculation not implemented "
                        "for kernel interpolation");
            }

            Real secondDerivative(Real) const override {
                QL_FAIL("Second derivative calculation not implemented "
                        "for kernel interpolation");
            }

        private:

            Real kernelAbs(Real x1, Real x2)const{
                return kernel_(std::fabs(x1-x2));
            }

            Real gammaFunc(Real x)const{

                Real res=0.0;

                for(Size i=0; i< xSize_;++i){
                    res+=kernelAbs(x,this->xBegin_[i]);
                }
                return res;
            }

            void updateAlphaVec(){
                // Function calculates the alpha vector with given
                // fixed pillars+values

                // Write Matrix M
                Real tmp=0.0;

                for(Size rowIt=0; rowIt<xSize_;++rowIt){

                    yVec_[rowIt]=this->yBegin_[rowIt];
                    tmp=1.0/gammaFunc(this->xBegin_[rowIt]);

                    for(Size colIt=0; colIt<xSize_;++colIt){
                        M_[rowIt][colIt]=kernelAbs(this->xBegin_[rowIt],
                                                   this->xBegin_[colIt])*tmp;
                    }
                }

                // Solve y=M*\alpha for \alpha
                alphaVec_ = qrSolve(M_, yVec_);

                // check if inversion worked up to a reasonable precision.
                // I've chosen not to check determinant(M_)!=0 before solving

                Array diffVec=Abs(M_*alphaVec_ - yVec_);

                for (double i : diffVec) {
                    QL_REQUIRE(i < invPrec_, "Inversion failed in 1d kernel interpolation");
                }
            }

            Size xSize_;
            Real invPrec_;
            Matrix M_;
            Array alphaVec_,yVec_;
            Kernel kernel_;
        };

    } // end namespace detail


    //! Kernel interpolation between discrete points
    /*! Implementation of the kernel interpolation approach, which can
        be found in "Foreign Exchange Risk" by Hakala, Wystup page
        256.

        The kernel in the implementation is kept general, although a Gaussian
        is considered in the cited text.

        \ingroup interpolations
        \warning See the Interpolation class for information about the
                 required lifetime of the underlying data.
    */
    class KernelInterpolation : public Interpolation {
      public:
        /*! \pre the \f$ x \f$ values must be sorted.
            \pre kernel needs a Real operator()(Real x) implementation

            The calculation will solve \f$ y = Ma \f$ for \f$a\f$.
            Due to singularity or rounding errors the recalculation
            \f$ Ma \f$ may not give \f$ y\f$. Here, a failure will
            be thrown if
            \f[
            \left\| Ma-y \right\|_\infty \geq \epsilon
            \f] */
        template <class I1, class I2, class Kernel>
        KernelInterpolation(const I1& xBegin, const I1& xEnd,
                            const I2& yBegin,
                            const Kernel& kernel,
                            const double epsilon = 1.0E-7) {
            impl_ = ext::shared_ptr<Interpolation::Impl>(new
                detail::KernelInterpolationImpl<I1,I2,Kernel>(xBegin, xEnd,
                                                              yBegin, kernel,
                                                              epsilon));
            impl_->update();
        }

    };
}

#endif


#ifndef id_fb74158ccfda233537be239d8f0505e4
#define id_fb74158ccfda233537be239d8f0505e4
inline bool test_fb74158ccfda233537be239d8f0505e4(const int* i) {
    return i != nullptr;
}
#endif
