/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007, 2008 Klaus Spanderen

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

/*! \file jointstochasticprocess.hpp
    \brief multi model process for hybrid products
*/

#ifndef quantlib_joint_stochastic_process_hpp
#define quantlib_joint_stochastic_process_hpp

#include <ql/utilities/null.hpp>
#include <ql/stochasticprocess.hpp>
#include <vector>
#include <map>

namespace QuantLib {

    class JointStochasticProcess : public StochasticProcess {
      public:
        JointStochasticProcess(std::vector<ext::shared_ptr<StochasticProcess> > l,
                               Size factors = Null<Size>());

        Size size() const override;
        Size factors() const override;

        Array initialValues() const override;
        Array drift(Time t, const Array& x) const override;
        Array expectation(Time t0, const Array& x0, Time dt) const override;

        Matrix diffusion(Time t, const Array& x) const override;
        Matrix covariance(Time t0, const Array& x0, Time dt) const override;
        Matrix stdDeviation(Time t0, const Array& x0, Time dt) const override;

        Array apply(const Array& x0, const Array& dx) const override;
        Array evolve(Time t0, const Array& x0, Time dt, const Array& dw) const override;

        virtual void preEvolve(Time t0, const Array& x0,
                               Time dt, const Array& dw) const = 0;
        virtual Array postEvolve(Time t0, const Array& x0,
                                 Time dt, const Array& dw,
                                 const Array& y0) const = 0;

        virtual DiscountFactor numeraire(Time t, const Array& x) const = 0;
        virtual bool correlationIsStateDependent() const = 0;
        virtual Matrix crossModelCorrelation(Time t0, const Array& x0) const = 0;

        const std::vector<ext::shared_ptr<StochasticProcess> > &
                                                       constituents() const;

        void update() override;
        Time time(const Date& date) const override;

      protected:
        std::vector<ext::shared_ptr<StochasticProcess> > l_;
        Array slice(const Array& x, Size i) const;

      private:
        typedef
            std::vector<ext::shared_ptr<StochasticProcess> >::const_iterator
            const_iterator;

        typedef std::vector<ext::shared_ptr<StochasticProcess> >::iterator
            iterator;

        Size size_ = 0, factors_, modelFactors_ = 0;
        std::vector<Size> vsize_, vfactors_;

        struct CachingKey {
            CachingKey(const Time t0, const Time dt)
                : t0_(t0), dt_(dt) {}

            bool operator<(const CachingKey& key) const {
                return   t0_ < key.t0_ 
                    || ( t0_ == key.t0_ && dt_ < key.dt_); 
            }
            Time t0_;
            Time dt_;
        };

        mutable std::map<CachingKey, Matrix> correlationCache_;
    };

}


#endif
