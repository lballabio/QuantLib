/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Roland Lichters

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file onefactorstudentcopula.hpp
    \brief One-factor Student-t copula
*/

#ifndef quantlib_one_factor_student_copula_hpp
#define quantlib_one_factor_student_copula_hpp

#include <ql/experimental/credit/onefactorcopula.hpp>
#include <ql/math/distributions/studenttdistribution.hpp>
#include <ql/math/distributions/normaldistribution.hpp>

namespace QuantLib {

    //! One-factor Double Student t-Copula
    /*! The copula model
        \f[ Y_i = a_i\,M+\sqrt{1-a_i^2}\:Z_i \f]

        is specified here by setting the probability density functions
        for \f$ Z_i \f$ (\f$ D_Z \f$) and \f$ M \f$ (\f$ D_M \f$) to
        Student t-distributions with \f$ N_z \f$ and \f$ N_m \f$
        degrees of freedom, respectively.

        The variance of the Student t-distribution with \f$ \nu \f$
        degrees of freedom is \f$ \nu / (\nu - 2) \f$. Since the
        copula approach requires zero mean and unit variance
        distributions, variables \f$ Z \f$ and \f$ M \f$ are scaled by
        \f$ \sqrt{(N_z - 2) / N_z} \f$ and \f$ \sqrt{(N_m - 2) / N_m}, \f$
        respectively.

        \todo Improve performance/accuracy of the calculation of
              inverse cumulative Y. Tabulate and store it for selected
              correlations?
    */
    class OneFactorStudentCopula : public OneFactorCopula {
      public:
        OneFactorStudentCopula (const Handle<Quote>& correlation,
                                int nz, int nm,
                                Real maximum = 10, Size integrationSteps = 200);

        Real density(Real m) const override;
        Real cumulativeZ(Real z) const override;

      private:
        //! Observer interface
        void performCalculations() const override;

        StudentDistribution density_;              // density of M
        CumulativeStudentDistribution cumulative_; // cumulated density of Z
        int nz_;                                   // degrees of freedom of Z
        int nm_;                                   // degrees of freedom of M

        Real scaleM_; // scaling for m to ensure unit variance
        Real scaleZ_; // scaling for z to ensure unit variance

        // This function is used to update the table of the cumulative
        // distribution of Y. It is invoked by performCalculations() when the
        // correlation handle is amended.
        Real cumulativeYintegral (Real y) const;
    };

    inline Real OneFactorStudentCopula::density (Real m) const {
        return density_(m / scaleM_) / scaleM_;
    }

    inline Real OneFactorStudentCopula::cumulativeZ (Real z) const {
        return cumulative_(z / scaleZ_);
    }


    //! One-factor Gaussian-Student t-Copula
    /*! The copula model
        \f[ Y_i = a_i\,M+\sqrt{1-a_i^2}\:Z_i \f]

        is specified here by setting the probability density functions
        for \f$ Z_i \f$ (\f$ D_Z \f$) to a Student t-distributions
        with \f$ N_z \f$ degrees of freedom, and for \f$ M \f$
        (\f$ D_M \f$) to a Gaussian.

        The variance of the Student t-distribution with \f$ \nu \f$
        degrees of freedom is \f$ \nu / (\nu - 2) \f$. Since the
        copula approach requires zero mean and unit variance
        distributions, \f$ Z \f$ is scaled by \f$ \sqrt{(N_z - 2) /
        N_z}.\f$

        \todo Improve performance/accuracy of the calculation of
              inverse cumulative Y. Tabulate and store it for selected
              correlations?
    */
    class OneFactorGaussianStudentCopula : public OneFactorCopula {
      public:
        OneFactorGaussianStudentCopula (const Handle<Quote>& correlation,
                                        int nz,
                                        Real maximum = 10,
                                        Size integrationSteps = 200);

        Real density(Real m) const override;
        Real cumulativeZ(Real z) const override;

      private:
        //! Observer interface
        void performCalculations() const override;

        NormalDistribution density_;               // density of M
        CumulativeStudentDistribution cumulative_; // cumulated density of Z
        int nz_;                                   // degrees of freedom of Z

        Real scaleZ_; // scaling for z to ensure unit variance

        // This function is used to update the table of the cumulative
        // distribution of Y. It is invoked by performCalculations() when the
        // correlation handle is amended.
        Real cumulativeYintegral (Real y) const;
    };

    inline Real OneFactorGaussianStudentCopula::density (Real m) const {
        return density_(m);
    }

    inline Real OneFactorGaussianStudentCopula::cumulativeZ (Real z) const {
        return cumulative_(z / scaleZ_);
    }


    //! One-factor Student t - Gaussian Copula
    /*! The copula model
        \f[ Y_i = a_i\,M+\sqrt{1-a_i^2}\:Z_i \f]
        is specified here by setting the probability density functions
        for \f$ Z_i \f$ (\f$ D_Z \f$) to a Gaussian and for \f$ M \f$
        (\f$ D_M \f$) to a Student t-distribution with \f$ N_m \f$
        degrees of freedom.

        The variance of the Student t-distribution with \f$ \nu \f$
        degrees of freedom is \f$ \nu / (\nu - 2) \f$. Since the
        copula approach requires zero mean and unit variance
        distributions, \f$ M \f$ is scaled by \f$ \sqrt{(N_m - 2) /
        N_m}. \f$

        \todo Improve performance/accuracy of the calculation of
              inverse cumulative Y. Tabulate and store it for selected
              correlations?
    */
    class OneFactorStudentGaussianCopula : public OneFactorCopula {
      public:
        OneFactorStudentGaussianCopula (const Handle<Quote>& correlation,
                                        int nm,
                                        Real maximum = 10,
                                        Size integrationSteps = 200);

        Real density(Real m) const override;
        Real cumulativeZ(Real z) const override;

      private:
        //! Observer interface
        void performCalculations() const override;

        StudentDistribution density_;              // density of M
        CumulativeNormalDistribution cumulative_;  // cumulated density of Z
        int nm_;                                   // degrees of freedom of M

        Real scaleM_; // scaling for m to ensure unit variance

        // This function is used to update the table of the cumulative
        // distribution of Y. It is invoked by performCalculations() when the
        // correlation handle is amended.
        Real cumulativeYintegral (Real y) const;
    };

    inline Real OneFactorStudentGaussianCopula::density (Real m) const {
        return density_(m / scaleM_) / scaleM_;
    }

    inline Real OneFactorStudentGaussianCopula::cumulativeZ (Real z) const {
        return cumulative_(z);
    }

}


#endif
