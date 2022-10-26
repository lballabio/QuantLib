/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
Copyright (C) 2007, 2008 Mark Joshi

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

#include <ql/math/matrixutilities/basisincompleteordered.hpp>
#include <algorithm>

namespace QuantLib {

    BasisIncompleteOrdered::BasisIncompleteOrdered(Size euclideanDimension)
        : euclideanDimension_(euclideanDimension) {}

    bool BasisIncompleteOrdered::addVector(const Array& newVector1) {

        QL_REQUIRE(newVector1.size() == euclideanDimension_,
            "missized vector passed to "
            "BasisIncompleteOrdered::addVector");

        newVector_ = newVector1;

        if (currentBasis_.size()==euclideanDimension_)
            return false;

        for (auto& currentBasi : currentBasis_) {
            Real innerProd =
                std::inner_product(newVector_.begin(), newVector_.end(), currentBasi.begin(), Real(0.0));

            for (Size k=0; k<euclideanDimension_; ++k)
                newVector_[k] -= innerProd * currentBasi[k];
        }

        Real norm = std::sqrt(std::inner_product(newVector_.begin(),
            newVector_.end(),
            newVector_.begin(), Real(0.0)));

        if (norm<1e-12) // maybe this should be a tolerance
            return false;

        for (Size l=0; l<euclideanDimension_; ++l)
            newVector_[l]/=norm;

        currentBasis_.push_back(newVector_);

        return true;
    }

    Size BasisIncompleteOrdered::basisSize() const {
        return currentBasis_.size();
    }

    Size BasisIncompleteOrdered::euclideanDimension() const {
        return euclideanDimension_;
    }


    Matrix BasisIncompleteOrdered::getBasisAsRowsInMatrix() const {
        Matrix basis(currentBasis_.size(), euclideanDimension_);
        for (Size i=0; i<basis.rows(); ++i)
            for (Size j=0; j<basis.columns(); ++j)
                basis[i][j] = currentBasis_[i][j];

        return basis;
    }

    namespace
    {
        Real normSquared(const Matrix& v, Size row)
        {
            Real x=0.0;
            for (Size i=0; i < v.columns(); ++i)
                x += v[row][i]*v[row][i];

            return x;
        }


        Real norm(const Matrix& v, Size row)
        {
            return std::sqrt(normSquared( v,  row));
        }

        Real innerProduct(const Matrix& v, Size row1, const Matrix& w, Size row2)
        {

            Real x=0.0;
            for (Size i=0; i < v.columns(); ++i)
                x += v[row1][i]*w[row2][i];

            return x;
        }

    }



    OrthogonalProjections::OrthogonalProjections(const Matrix& originalVectors,
                                                 Real multiplierCutoff,
                                                 Real tolerance)
    : originalVectors_(originalVectors),
      multiplierCutoff_(multiplierCutoff),
      numberVectors_(originalVectors.rows()),
      dimension_(originalVectors.columns()),
      validVectors_(true,originalVectors.rows()), // opposite way round from vector constructor
      orthoNormalizedVectors_(originalVectors.rows(),
                              originalVectors.columns())
    {
        std::vector<Real> currentVector(dimension_);
        for (Size j=0; j < numberVectors_; ++j)
        {

            if (validVectors_[j])
            {
                for (Size k=0; k< numberVectors_; ++k) // create an orthormal basis not containing j
                {
                    for (Size m=0; m < dimension_; ++m)
                        orthoNormalizedVectors_[k][m] = originalVectors_[k][m];

                    if ( k !=j && validVectors_[k])
                    {


                        for (Size l=0; l < k; ++l)
                        {
                            if (validVectors_[l] && l !=j)
                            {
                                Real dotProduct = innerProduct(orthoNormalizedVectors_, k, orthoNormalizedVectors_,l);
                                for (Size n=0; n < dimension_; ++n)
                                    orthoNormalizedVectors_[k][n] -=  dotProduct*orthoNormalizedVectors_[l][n];
                            }

                        }

                        Real normBeforeScaling= norm(orthoNormalizedVectors_,k);

                        if (normBeforeScaling < tolerance)
                        {
                            validVectors_[k] = false;
                        }
                        else
                        {
                            Real normBeforeScalingRecip = 1.0/normBeforeScaling;
                            for (Size m=0; m < dimension_; ++m)
                                orthoNormalizedVectors_[k][m] *= normBeforeScalingRecip;

                        } // end of else (norm < tolerance)

                    } // end of if k !=j && validVectors_[k])

                }// end of  for (Size k=0; k< numberVectors_; ++k)

                // we now have an o.n. basis for everything except  j

                Real prevNormSquared = normSquared(originalVectors_, j);


                for (Size r=0; r < numberVectors_; ++r)
                    if (validVectors_[r] && r != j)
                    {
                        Real dotProduct = innerProduct(orthoNormalizedVectors_, j, orthoNormalizedVectors_,r);

                        for (Size s=0; s < dimension_; ++s)
                           orthoNormalizedVectors_[j][s] -= dotProduct*orthoNormalizedVectors_[r][s];

                    }

               Real projectionOnOriginalDirection = innerProduct(originalVectors_,j,orthoNormalizedVectors_,j);
               Real sizeMultiplier = prevNormSquared/projectionOnOriginalDirection;

               if (std::fabs(sizeMultiplier) < multiplierCutoff_)
               {
                    for (Size t=0; t < dimension_; ++t)
                        currentVector[t] = orthoNormalizedVectors_[j][t]*sizeMultiplier;

               }
               else
                   validVectors_[j] = false;


            } // end of  if (validVectors_[j])

            projectedVectors_.push_back(currentVector);


        } //end of j loop

        numberValidVectors_ =0;
        for (Size i=0; i < numberVectors_; ++i)
            numberValidVectors_ +=  validVectors_[i] ? 1 : 0;


    } // end of constructor

    const std::valarray<bool>& OrthogonalProjections::validVectors() const
    {
        return validVectors_;

    }

    const std::vector<Real>& OrthogonalProjections::GetVector(Size index) const
    {
        return projectedVectors_[index];
    }


  Size OrthogonalProjections::numberValidVectors() const
  {
        return numberValidVectors_;
  }




}
