
/*
 Copyright (C) 2003 Neil Firth

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.

        Adapted from the TNT project
        http://math.nist.gov/tnt/download.html

        This software was developed at the National Institute of Standards 
        and Technology (NIST) by employees of the Federal Government in the 
        course of their official duties. Pursuant to title 17 Section 105 
        of the United States Code this software is not subject to copyright 
        protection and is in the public domain. NIST assumes no responsibility 
        whatsoever for its use by other parties, and makes no guarantees, 
        expressed or implied, about its quality, reliability, or any other 
        characteristic. 

        We would appreciate acknowledgement if the software is incorporated in 
        redistributable libraries or applications. 
*/

/*! \file svd.cpp
    \brief singular value decomposition
*/


#include <ql/Math/svd.hpp>

namespace QuantLib {

    //! Singular Value Decomposition
    /*! Refer to Golub and Van Loan: Matrix computation,
        The Johns Hopkins University Press
    */

    /*  returns hypotenuse of real (non-complex) scalars a and b by 
        avoiding underflow/overflow
        using (a * sqrt( 1 + (b/a) * (b/a))), rather than
        sqrt(a*a + b*b).
    */
    double hypot(const double &a, const double &b) {	
        if (a == 0) {
            return QL_FABS(b);
        } else {
            double c = b/a;
            return QL_FABS(a) * sqrt(1 + c*c);
        }
    }

    SVD::SVD(const Matrix &Arg) {

        m = Arg.rows();
        n = Arg.columns();
        int nu = QL_MIN(m,n);
        s = Array(QL_MIN(m+1,n)); 
        U = Matrix(m, nu, 0.0);
        V = Matrix(n,n);
        Array e(n);
        Array work(m);
        Matrix A(Arg);
        int wantu = 1;  					/* boolean */
        int wantv = 1;  					/* boolean */
        int i, j, k;

        // Reduce A to bidiagonal form, storing the diagonal elements
        // in s and the super-diagonal elements in e.

        int nct = QL_MIN(m-1,n);
        int nrt = QL_MAX(0,QL_MIN(n-2,m));
        for (k = 0; k < QL_MAX(nct,nrt); k++) {
            if (k < nct) {

                // Compute the transformation for the k-th column and
                // place the k-th diagonal in s[k].
                // Compute 2-norm of k-th column without under/overflow.
                s[k] = 0;
                for (i = k; i < m; i++) {
                    s[k] = hypot(s[k],A[i][k]);
                }
                if (s[k] != 0.0) {
                    if (A[k][k] < 0.0) {
                        s[k] = -s[k];
                    }
                    for (i = k; i < m; i++) {
                        A[i][k] /= s[k];
                    }
                    A[k][k] += 1.0;
                }
                s[k] = -s[k];
            }
            for (j = k+1; j < n; j++) {
                if ((k < nct) && (s[k] != 0.0))  {

                    // Apply the transformation.

                    double t = 0;
                    for (i = k; i < m; i++) {
                        t += A[i][k]*A[i][j];
                    }
                    t = -t/A[k][k];
                    for (i = k; i < m; i++) {
                        A[i][j] += t*A[i][k];
                    }
                }

                // Place the k-th row of A into e for the
                // subsequent calculation of the row transformation.

                e[j] = A[k][j];
            }
            if (wantu & (k < nct)) {

                // Place the transformation in U for subsequent back
                // multiplication.

                for (i = k; i < m; i++) {
                    U[i][k] = A[i][k];
                }
            }
            if (k < nrt) {

                // Compute the k-th row transformation and place the
                // k-th super-diagonal in e[k].
                // Compute 2-norm without under/overflow.
                e[k] = 0;
                for (i = k+1; i < n; i++) {
                    e[k] = hypot(e[k],e[i]);
                }
                if (e[k] != 0.0) {
                    if (e[k+1] < 0.0) {
                        e[k] = -e[k];
                    }
                    for (i = k+1; i < n; i++) {
                        e[i] /= e[k];
                    }
                    e[k+1] += 1.0;
                }
                e[k] = -e[k];
                if ((k+1 < m) & (e[k] != 0.0)) {

                    // Apply the transformation.

                    for (i = k+1; i < m; i++) {
                        work[i] = 0.0;
                    }
                    for (j = k+1; j < n; j++) {
                        for (i = k+1; i < m; i++) {
                            work[i] += e[j]*A[i][j];
                        }
                    }
                    for (j = k+1; j < n; j++) {
                        double t = -e[j]/e[k+1];
                        for (i = k+1; i < m; i++) {
                            A[i][j] += t*work[i];
                        }
                    }
                }
                if (wantv) {

                    // Place the transformation in V for subsequent
                    // back multiplication.

                    for (i = k+1; i < n; i++) {
                        V[i][k] = e[i];
                    }
                }
            }
        }

        // Set up the final bidiagonal matrix or order p.

        int p = QL_MIN(n,m+1);
        if (nct < n) {
            s[nct] = A[nct][nct];
        }
        if (m < p) {
            s[p-1] = 0.0;
        }
        if (nrt+1 < p) {
            e[nrt] = A[nrt][p-1];
        }
        e[p-1] = 0.0;

        // If required, generate U.

        if (wantu) {
            for (j = nct; j < nu; j++) {
                for (i = 0; i < m; i++) {
                    U[i][j] = 0.0;
                }
                U[j][j] = 1.0;
            }
            for (k = nct-1; k >= 0; k--) {
                if (s[k] != 0.0) {
                    for (j = k+1; j < nu; j++) {
                        double t = 0;
                        for (i = k; i < m; i++) {
                            t += U[i][k]*U[i][j];
                        }
                        t = -t/U[k][k];
                        for (i = k; i < m; i++) {
                            U[i][j] += t*U[i][k];
                        }
                    }
                    for (i = k; i < m; i++ ) {
                        U[i][k] = -U[i][k];
                    }
                    U[k][k] = 1.0 + U[k][k];
                    for (i = 0; i < k-1; i++) {
                        U[i][k] = 0.0;
                    }
                } else {
                    for (i = 0; i < m; i++) {
                        U[i][k] = 0.0;
                    }
                    U[k][k] = 1.0;
                }
            }
        }

        // If required, generate V.

        if (wantv) {
            for (k = n-1; k >= 0; k--) {
                if ((k < nrt) & (e[k] != 0.0)) {
                    for (j = k+1; j < nu; j++) {
                        double t = 0;
                        for (i = k+1; i < n; i++) {
                            t += V[i][k]*V[i][j];
                        }
                        t = -t/V[k+1][k];
                        for (i = k+1; i < n; i++) {
                            V[i][j] += t*V[i][k];
                        }
                    }
                }
                for (i = 0; i < n; i++) {
                    V[i][k] = 0.0;
                }
                V[k][k] = 1.0;
            }
        }

        // Main iteration loop for the singular values.

        int pp = p-1;
        int iter = 0;
        double eps = pow(2.0,-52.0);
        while (p > 0) {
            int k;
            int kase;

            // Here is where a test for too many iterations would go.

            // This section of the program inspects for
            // negligible elements in the s and e arrays.  On
            // completion the variables kase and k are set as follows.

            // kase = 1     if s(p) and e[k-1] are negligible and k<p
            // kase = 2     if s(k) is negligible and k<p
            // kase = 3     if e[k-1] is negligible, k<p, and
            //              s(k), ..., s(p) are not negligible (qr step).
            // kase = 4     if e(p-1) is negligible (convergence).

            for (k = p-2; k >= -1; k--) {
                if (k == -1) {
                    break;
                }
                if (QL_FABS(e[k]) <= eps*(QL_FABS(s[k]) + QL_FABS(s[k+1]))) {
                    e[k] = 0.0;
                    break;
                }
            }
            if (k == p-2) {
                kase = 4;
            } else {
                int ks;
                for (ks = p-1; ks >= k; ks--) {
                    if (ks == k) {
                        break;
                    }
                    double t = (ks != p ? QL_FABS(e[ks]) : 0.) + 
                        (ks != k+1 ? QL_FABS(e[ks-1]) : 0.);
                    if (QL_FABS(s[ks]) <= eps*t)  {
                        s[ks] = 0.0;
                        break;
                    }
                }
                if (ks == k) {
                    kase = 3;
                } else if (ks == p-1) {
                    kase = 1;
                } else {
                    kase = 2;
                    k = ks;
                }
            }
            k++;

            // Perform the task indicated by kase.

            switch (kase) {

                // Deflate negligible s(p).

              case 1: {
                  double f = e[p-2];
                  e[p-2] = 0.0;
                  for (j = p-2; j >= k; j--) {
                      double t = hypot(s[j],f);
                      double cs = s[j]/t;
                      double sn = f/t;
                      s[j] = t;
                      if (j != k) {
                          f = -sn*e[j-1];
                          e[j-1] = cs*e[j-1];
                      }
                      if (wantv) {
                          for (i = 0; i < n; i++) {
                              t = cs*V[i][j] + sn*V[i][p-1];
                              V[i][p-1] = -sn*V[i][j] + cs*V[i][p-1];
                              V[i][j] = t;
                          }
                      }
                  }
              }
                break;

                // Split at negligible s(k).

              case 2: {
                  double f = e[k-1];
                  e[k-1] = 0.0;
                  for (j = k; j < p; j++) {
                      double t = hypot(s[j],f);
                      double cs = s[j]/t;
                      double sn = f/t;
                      s[j] = t;
                      f = -sn*e[j];
                      e[j] = cs*e[j];
                      if (wantu) {
                          for (i = 0; i < m; i++) {
                              t = cs*U[i][j] + sn*U[i][k-1];
                              U[i][k-1] = -sn*U[i][j] + cs*U[i][k-1];
                              U[i][j] = t;
                          }
                      }
                  }
              }
                break;

                // Perform one qr step.

              case 3: {

                  // Calculate the shift.
                  //                        double asdfasdf = QL_MAX(1.12, 1.342);
                  double scale = QL_MAX(QL_MAX(QL_MAX(QL_MAX(
                                                             QL_FABS(s[p-1]),QL_FABS(s[p-2])),QL_FABS(e[p-2])), 
                                               QL_FABS(s[k])),QL_FABS(e[k]));
                  double sp = s[p-1]/scale;
                  double spm1 = s[p-2]/scale;
                  double epm1 = e[p-2]/scale;
                  double sk = s[k]/scale;
                  double ek = e[k]/scale;
                  double b = ((spm1 + sp)*(spm1 - sp) + epm1*epm1)/2.0;
                  double c = (sp*epm1)*(sp*epm1);
                  double shift = 0.0;
                  if ((b != 0.0) | (c != 0.0)) {
                      shift = sqrt(b*b + c);
                      if (b < 0.0) {
                          shift = -shift;
                      }
                      shift = c/(b + shift);
                  }
                  double f = (sk + sp)*(sk - sp) + shift;
                  double g = sk*ek;

                  // Chase zeros.

                  for (j = k; j < p-1; j++) {
                      double t = hypot(f,g);
                      double cs = f/t;
                      double sn = g/t;
                      if (j != k) {
                          e[j-1] = t;
                      }
                      f = cs*s[j] + sn*e[j];
                      e[j] = cs*e[j] - sn*s[j];
                      g = sn*s[j+1];
                      s[j+1] = cs*s[j+1];
                      if (wantv) {
                          for (i = 0; i < n; i++) {
                              t = cs*V[i][j] + sn*V[i][j+1];
                              V[i][j+1] = -sn*V[i][j] + cs*V[i][j+1];
                              V[i][j] = t;
                          }
                      }
                      t = hypot(f,g);
                      cs = f/t;
                      sn = g/t;
                      s[j] = t;
                      f = cs*e[j] + sn*s[j+1];
                      s[j+1] = -sn*e[j] + cs*s[j+1];
                      g = sn*e[j+1];
                      e[j+1] = cs*e[j+1];
                      if (wantu && (j < m-1)) {
                          for (i = 0; i < m; i++) {
                              t = cs*U[i][j] + sn*U[i][j+1];
                              U[i][j+1] = -sn*U[i][j] + cs*U[i][j+1];
                              U[i][j] = t;
                          }
                      }
                  }
                  e[p-2] = f;
                  iter = iter + 1;
              }
                break;

                // Convergence.

              case 4: {

                  // Make the singular values positive.

                  if (s[k] <= 0.0) {
                      s[k] = (s[k] < 0.0 ? -s[k] : 0.0);
                      if (wantv) {
                          for (i = 0; i <= pp; i++) {
                              V[i][k] = -V[i][k];
                          }
                      }
                  }

                  // Order the singular values.

                  while (k < pp) {
                      if (s[k] >= s[k+1]) {
                          break;
                      }
                      double t = s[k];
                      s[k] = s[k+1];
                      s[k+1] = t;
                      if (wantv && (k < n-1)) {
                          for (i = 0; i < n; i++) {
                              t = V[i][k+1]; V[i][k+1] = V[i][k]; V[i][k] = t;
                          }
                      }
                      if (wantu && (k < m-1)) {
                          for (i = 0; i < m; i++) {
                              t = U[i][k+1]; U[i][k+1] = U[i][k]; U[i][k] = t;
                          }
                      }
                      k++;
                  }
                  iter = 0;
                  p--;
              }
                break;
            }
        }
    }

    void SVD::getU(Matrix &A) const {
        int minm = QL_MIN(m+1,n);

        A = Matrix(m, minm);

        for (int i=0; i<m; i++)
            for (int j=0; j<minm; j++)
                A[i][j] = U[i][j];

    }

    void SVD::getV(Matrix &A) const {
        A = V;
    }

    void SVD::getSingularValues(Array &x) const {
        x = s;
    }

    void SVD::getS(Matrix &S) const {
        S = Matrix(n,n);
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                S[i][j] = 0.0;
            }
            S[i][i] = s[i];
        }
    }

    double SVD::norm2() {
        return s[0];
    }

    double SVD::cond() {
        return s[0]/s[QL_MIN(m,n)-1];
    }

    int SVD::rank() {
        double eps = pow(2.0,-52.0);
        double tol = QL_MAX(m,n)*s[0]*eps;
        int r = 0;
        for (Size i = 0; i < s.size(); i++) {
            if (s[i] > tol) {
                r++;
            }
        }
        return r;
    }

}

