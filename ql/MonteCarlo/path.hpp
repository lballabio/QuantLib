
/*
 Copyright (C) 2000, 2001, 2002 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software developed by the QuantLib Group; you can
 redistribute it and/or modify it under the terms of the QuantLib License;
 either version 1.0, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 QuantLib License for more details.

 You should have received a copy of the QuantLib License along with this
 program; if not, please email ferdinando@ametrano.net

 The QuantLib License is also available at http://quantlib.org/license.html
 The members of the QuantLib Group are listed in the QuantLib License
*/

// $Id$

#ifndef quantlib_montecarlo_path_h
#define quantlib_montecarlo_path_h

#include <ql/array.hpp>
#include <ql/handle.hpp>
#include <vector>

namespace QuantLib {

    namespace MonteCarlo {

        //! single factor random walk
        /*! \todo make it time-aware
        */
        class Path {
          public:
            Path(Size size);
            Path(const std::vector<Time>& times,
                 const Array& drift,
                 const Array& diffusion);
            //! \name inspectors
            //@{
            double operator[](int i) const;
            Size size() const;
            //@}
            //! \name read/write access to components
            //@{
            const std::vector<Time>& times() const;
            std::vector<Time>& times();
            const Array& drift() const;
            Array& drift();
            const Array& diffusion() const;
            Array& diffusion();
            //@}
          private:
            std::vector<Time> times_;
            Array drift_;
            Array diffusion_;
        };

        // inline definitions

        inline Path::Path(Size size)
        : times_(size), drift_(size), diffusion_(size) {}

        inline Path::Path(const std::vector<Time>& times, const Array& drift,
            const Array& diffusion)
        : times_(times), drift_(drift), diffusion_(diffusion) {
            QL_REQUIRE(drift_.size() == diffusion_.size(),
                "Path: drift and diffusion have different size");
            QL_REQUIRE(times_.size() == drift_.size(),
                "Path: times and drift have different size");
        }

        inline double Path::operator[](int i) const {
            return drift_[i] + diffusion_[i];
        }

        inline Size Path::size() const {
            return drift_.size();
        }

        inline const std::vector<Time>& Path::times() const {
            return times_;
        }

        inline std::vector<Time>& Path::times() {
            return times_;
        }

        inline const Array& Path::drift() const {
            return drift_;
        }

        inline Array& Path::drift() {
            return drift_;
        }

        inline const Array& Path::diffusion() const {
            return diffusion_;
        }

        inline Array& Path::diffusion() {
            return diffusion_;
        }

    }

}


#endif
