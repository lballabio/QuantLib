/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
Copyright (C) 2010 Adrian O' Neill

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

/*! \file variancegammamodel.hpp
    \brief Variance Gamma model
*/

#ifndef quantlib_variance_gamma_model_hpp
#define quantlib_variance_gamma_model_hpp

#include <ql/models/model.hpp>
#include <ql/experimental/variancegamma/variancegammaprocess.hpp>

namespace QuantLib {

    //! Variance Gamma model
    /*! References:

        Dilip B. Madan, Peter Carr, Eric C. Chang (1998)
        "The variance gamma process and option pricing,"
        European Finance Review, 2, 79-105

        \warning calibration is not implemented for VG
    */
    class VarianceGammaModel : public CalibratedModel {
      public:
        explicit VarianceGammaModel(
                      const std::shared_ptr<VarianceGammaProcess>& process);

        // sigma
        Real sigma() const { return arguments_[0](0.0); }
        // nu
        Real nu() const { return arguments_[1](0.0); }
        // theta
        Real theta() const { return arguments_[2](0.0); }

        // underlying process
        std::shared_ptr<VarianceGammaProcess> process() const { return process_; }

    protected:
      void generateArguments() override;
      std::shared_ptr<VarianceGammaProcess> process_;
    };

}

#endif

