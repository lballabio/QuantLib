/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003 Ferdinando Ametrano
 Copyright (C) 2003, 2004, 2005 StatPro Italia srl
 Copyright (C) 2005 Klaus Spanderen

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

/*! \file multipathgenerator.hpp
    \brief Generates a multi path from a random-array generator
*/

#ifndef quantlib_multi_path_generator_hpp
#define quantlib_multi_path_generator_hpp

#include <ql/methods/montecarlo/multipath.hpp>
#include <ql/methods/montecarlo/sample.hpp>
#include <ql/stochasticprocess.hpp>
#include <utility>

namespace QuantLib {

    //! Generates a multipath from a random number generator.
    /*! RSG is a sample generator which returns a random sequence.
        It must have the minimal interface:
        \code
        RSG {
            Sample<Array> next();
        };
        \endcode

        \ingroup mcarlo

        \test the generated paths are checked against cached results
    */
    template <class GSG>
    class MultiPathGenerator {
      public:
        typedef Sample<MultiPath> sample_type;
        MultiPathGenerator(const std::shared_ptr<StochasticProcess>&,
                           const TimeGrid&,
                           GSG generator,
                           bool brownianBridge = false);
        const sample_type& next() const;
        const sample_type& antithetic() const;
      private:
        const sample_type& next(bool antithetic) const;
        bool brownianBridge_;
        std::shared_ptr<StochasticProcess> process_;
        GSG generator_;
        mutable sample_type next_;
    };


    // template definitions

    template <class GSG>
    MultiPathGenerator<GSG>::MultiPathGenerator(const std::shared_ptr<StochasticProcess>& process,
                                                const TimeGrid& times,
                                                GSG generator,
                                                bool brownianBridge)
    : brownianBridge_(brownianBridge), process_(process), generator_(std::move(generator)),
      next_(MultiPath(process->size(), times), 1.0) {

        QL_REQUIRE(generator_.dimension() ==
                   process->factors()*(times.size()-1),
                   "dimension (" << generator_.dimension()
                   << ") is not equal to ("
                   << process->factors() << " * " << times.size()-1
                   << ") the number of factors "
                   << "times the number of time steps");
        QL_REQUIRE(times.size() > 1,
                   "no times given");
    }

    template <class GSG>
    inline const typename MultiPathGenerator<GSG>::sample_type&
    MultiPathGenerator<GSG>::next() const {
        return next(false);
    }

    template <class GSG>
    inline const typename MultiPathGenerator<GSG>::sample_type&
    MultiPathGenerator<GSG>::antithetic() const {
        return next(true);
    }

    template <class GSG>
    const typename MultiPathGenerator<GSG>::sample_type&
    MultiPathGenerator<GSG>::next(bool antithetic) const {

        if (brownianBridge_) {

            QL_FAIL("Brownian bridge not supported");

        } else {

            typedef typename GSG::sample_type sequence_type;
            const sequence_type& sequence_ =
                antithetic ? generator_.lastSequence()
                           : generator_.nextSequence();

            Size m = process_->size();
            Size n = process_->factors();

            MultiPath& path = next_.value;

            Array asset = process_->initialValues();
            for (Size j=0; j<m; j++)
                path[j].front() = asset[j];

            Array temp(n);
            next_.weight = sequence_.weight;

            const TimeGrid& timeGrid = path[0].timeGrid();
            Time t, dt;
            for (Size i = 1; i < path.pathSize(); i++) {
                Size offset = (i-1)*n;
                t = timeGrid[i-1];
                dt = timeGrid.dt(i-1);
                if (antithetic)
                    std::transform(sequence_.value.begin()+offset,
                                   sequence_.value.begin()+offset+n,
                                   temp.begin(),
                                   std::negate<>());
                else
                    std::copy(sequence_.value.begin()+offset,
                              sequence_.value.begin()+offset+n,
                              temp.begin());

                asset = process_->evolve(t, asset, dt, temp);
                for (Size j=0; j<m; j++)
                    path[j][i] = asset[j];
            }
            return next_;
        }
    }

}

#endif
