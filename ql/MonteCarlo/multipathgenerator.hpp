
/*
 Copyright (C) 2003 Ferdinando Ametrano
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

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

/*! \file multipathgenerator.hpp
    \brief Generates a multi path from a random-array generator
*/

#ifndef quantlib_montecarlo_multi_path_generator_h
#define quantlib_montecarlo_multi_path_generator_h

#include <ql/stochasticprocess.hpp>
#include <ql/MonteCarlo/multipath.hpp>
#include <ql/MonteCarlo/sample.hpp>
#include <ql/Math/pseudosqrt.hpp>

namespace QuantLib {

    //! Generates a multipath from a random number generator.
    /*! RSG is a sample generator which returns a random sequence.
        It must have the minimal interface:
        \code
        RSG {
            Sample<Array> next();
        };
        \endcode

        \todo why store correlation matrix rather than covariance?

        \ingroup mcarlo
    */
    template <class GSG>
    class MultiPathGenerator {
      public:
        typedef Sample<MultiPath> sample_type;
        MultiPathGenerator(
                     const std::vector<boost::shared_ptr<StochasticProcess> >&
                                                               diffusionProcs,
                     const Matrix& correlation,
                     const TimeGrid& timeGrid,
                     GSG generator,
                     bool brownianBridge);
        const sample_type& next() const;
        const sample_type& antithetic() const;
      private:
        bool brownianBridge_;
        std::vector<boost::shared_ptr<StochasticProcess> > diffusionProcs_;
        Size numAssets_;
        Matrix sqrtCorrelation_;
        GSG generator_;
        mutable sample_type next_;
    };


    // template definitions

    template <class GSG>
    MultiPathGenerator<GSG>::MultiPathGenerator(
                     const std::vector<boost::shared_ptr<StochasticProcess> >&
                                                               diffusionProcs,
                     const Matrix& correlation,
                     const TimeGrid& times,
                     GSG generator,
                     bool brownianBridge)
    :   brownianBridge_(brownianBridge),
        diffusionProcs_(diffusionProcs),
        numAssets_(correlation.rows()),
        sqrtCorrelation_(pseudoSqrt(correlation,SalvagingAlgorithm::Spectral)),
        generator_(generator),
        next_(MultiPath(correlation.rows(), times), 1.0) {

        QL_REQUIRE(generator_.dimension() == numAssets_*(times.size()-1),
                   "dimension (" +
                   SizeFormatter::toString(generator_.dimension()) +
                   ") is not equal to (" +
                   SizeFormatter::toString(numAssets_) +
                   " * " +
                   SizeFormatter::toString(times.size()-1) +
                   ") the number of assets times the number of time steps");
        QL_REQUIRE(sqrtCorrelation_.columns() == numAssets_,
                   "correlation is not a square matrix");
        QL_REQUIRE(times.size() > 1,
                   "no times given");
    }


    template <class GSG>
    const typename MultiPathGenerator<GSG>::sample_type&
    MultiPathGenerator<GSG>::next() const {

        if (brownianBridge_) {
            /*typedef typename BrownianBridge<GSG>::sample_type sequence_type;
            const sequence_type& stdDev_ = bb_.next();

            next_.weight = stdDev_.weight;

            Time t = timeGrid_[1];
            Time dt= timeGrid_.dt(0);
            next_.value.drift()[0] = dt *
                diffProcess_->drift(t, asset_);
            next_.value.diffusion()[0] = stdDev_.value[0];
            for (Size i=1; i<next_.value.size(); i++) {
                t = timeGrid_[i+1];
                dt = timeGrid_.dt(i);
                next_.value.drift()[i] = dt *
                    diffProcess_->drift(t, asset_);
                next_.value.diffusion()[i] =
                    stdDev_.value[i] - stdDev_.value[i-1];
            }
            */
            return next_;

        } else {
            typedef typename GSG::sample_type sequence_type;
            const sequence_type& sequence_ = generator_.nextSequence();

            Array asset(numAssets_);
            Array temp(numAssets_);
            next_.weight = sequence_.weight;

            for (Size j = 0; j < numAssets_; j++) {
                asset[j] = diffusionProcs_[j]->x0();
            }

            TimeGrid timeGrid = next_.value[0].timeGrid();
            Time t, dt;
            for (Size i = 0; i < next_.value[0].size(); i++) {
                Size offset = i*numAssets_;
                t = timeGrid[i+1];
                dt = timeGrid.dt(i);
                std::copy(sequence_.value.begin()+offset,
                        sequence_.value.begin()+offset+numAssets_,
                        temp.begin());

                temp = sqrtCorrelation_ * temp;

                for (Size j=0; j<numAssets_; j++) {
                    next_.value[j].drift()[i] =
                        dt * diffusionProcs_[j]->drift(t, asset[j]);
                    next_.value[j].diffusion()[i] =
                        temp[j] *
                        QL_SQRT(diffusionProcs_[j]->variance(t, asset[j], dt));
                    asset[j] *=
                        QL_EXP(next_.value[j].drift()[i] +
                               next_.value[j].diffusion()[i]);
                }
            }

            return next_;
        }
    }


    template <class GSG>
    inline const typename MultiPathGenerator<GSG>::sample_type&
    MultiPathGenerator<GSG>::antithetic() const {

        // brownian bridge?

        return next();

    }

}


#endif
