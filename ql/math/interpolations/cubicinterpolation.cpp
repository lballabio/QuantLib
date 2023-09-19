/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2001, 2002, 2003 Nicolas Di Césaré
 Copyright (C) 2004, 2008, 2009, 2011 Ferdinando Ametrano
 Copyright (C) 2009 Sylvain Bertrand
 Copyright (C) 2013 Peter Caspers
 Copyright (C) 2016 Nicholas Bertocchi

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

#include <ql/math/interpolations/cubicinterpolation.hpp>

namespace QuantLib {

    namespace detail {

        void cubicInterpolationSplineOM1(Array& tmp_, std::vector<Real>& dx_, Array& Y_, Size n_) {
            Matrix T_(n_ - 2, n_, 0.0);
            for (Size i = 0; i < n_ - 2; ++i) {
                T_[i][i] = dx_[i] / 6.0;
                T_[i][i + 1] = (dx_[i + 1] + dx_[i]) / 3.0;
                T_[i][i + 2] = dx_[i + 1] / 6.0;
            }
            Matrix S_(n_ - 2, n_, 0.0);
            for (Size i = 0; i < n_ - 2; ++i) {
                S_[i][i] = 1.0 / dx_[i];
                S_[i][i + 1] = -(1.0 / dx_[i + 1] + 1.0 / dx_[i]);
                S_[i][i + 2] = 1.0 / dx_[i + 1];
            }
            Matrix Up_(n_, 2, 0.0);
            Up_[0][0] = 1;
            Up_[n_ - 1][1] = 1;
            Matrix Us_(n_, n_ - 2, 0.0);
            for (Size i = 0; i < n_ - 2; ++i)
                Us_[i + 1][i] = 1;
            Matrix Z_ = Us_ * inverse(T_ * Us_);
            Matrix I_(n_, n_, 0.0);
            for (Size i = 0; i < n_; ++i)
                I_[i][i] = 1;
            Matrix V_ = (I_ - Z_ * T_) * Up_;
            Matrix W_ = Z_ * S_;
            Matrix Q_(n_, n_, 0.0);
            Q_[0][0] = 1.0 / (n_ - 1) * dx_[0] * dx_[0] * dx_[0];
            Q_[0][1] = 7.0 / 8 * 1.0 / (n_ - 1) * dx_[0] * dx_[0] * dx_[0];
            for (Size i = 1; i < n_ - 1; ++i) {
                Q_[i][i - 1] = 7.0 / 8 * 1.0 / (n_ - 1) * dx_[i - 1] * dx_[i - 1] * dx_[i - 1];
                Q_[i][i] = 1.0 / (n_ - 1) * dx_[i] * dx_[i] * dx_[i] +
                           1.0 / (n_ - 1) * dx_[i - 1] * dx_[i - 1] * dx_[i - 1];
                Q_[i][i + 1] = 7.0 / 8 * 1.0 / (n_ - 1) * dx_[i] * dx_[i] * dx_[i];
            }
            Q_[n_ - 1][n_ - 2] = 7.0 / 8 * 1.0 / (n_ - 1) * dx_[n_ - 2] * dx_[n_ - 2] * dx_[n_ - 2];
            Q_[n_ - 1][n_ - 1] = 1.0 / (n_ - 1) * dx_[n_ - 2] * dx_[n_ - 2] * dx_[n_ - 2];
            Matrix J_ = (I_ - V_ * inverse(transpose(V_) * Q_ * V_) * transpose(V_) * Q_) * W_;
            Array D_ = J_ * Y_;
            for (Size i = 0; i < n_ - 1; ++i)
                tmp_[i] = (Y_[i + 1] - Y_[i]) / dx_[i] - (2.0 * D_[i] + D_[i + 1]) * dx_[i] / 6.0;
            tmp_[n_ - 1] = tmp_[n_ - 2] + D_[n_ - 2] * dx_[n_ - 2] +
                           (D_[n_ - 1] - D_[n_ - 2]) * dx_[n_ - 2] / 2.0;
        }

        void cubicInterpolationSplineOM2(Array& tmp_, std::vector<Real>& dx_, Array& Y_, Size n_) {
            Matrix T_(n_ - 2, n_, 0.0);
            for (Size i = 0; i < n_ - 2; ++i) {
                T_[i][i] = dx_[i] / 6.0;
                T_[i][i + 1] = (dx_[i] + dx_[i + 1]) / 3.0;
                T_[i][i + 2] = dx_[i + 1] / 6.0;
            }
            Matrix S_(n_ - 2, n_, 0.0);
            for (Size i = 0; i < n_ - 2; ++i) {
                S_[i][i] = 1.0 / dx_[i];
                S_[i][i + 1] = -(1.0 / dx_[i + 1] + 1.0 / dx_[i]);
                S_[i][i + 2] = 1.0 / dx_[i + 1];
            }
            Matrix Up_(n_, 2, 0.0);
            Up_[0][0] = 1;
            Up_[n_ - 1][1] = 1;
            Matrix Us_(n_, n_ - 2, 0.0);
            for (Size i = 0; i < n_ - 2; ++i)
                Us_[i + 1][i] = 1;
            Matrix Z_ = Us_ * inverse(T_ * Us_);
            Matrix I_(n_, n_, 0.0);
            for (Size i = 0; i < n_; ++i)
                I_[i][i] = 1;
            Matrix V_ = (I_ - Z_ * T_) * Up_;
            Matrix W_ = Z_ * S_;
            Matrix Q_(n_, n_, 0.0);
            Q_[0][0] = 1.0 / (n_ - 1) * dx_[0];
            Q_[0][1] = 1.0 / 2 * 1.0 / (n_ - 1) * dx_[0];
            for (Size i = 1; i < n_ - 1; ++i) {
                Q_[i][i - 1] = 1.0 / 2 * 1.0 / (n_ - 1) * dx_[i - 1];
                Q_[i][i] = 1.0 / (n_ - 1) * dx_[i] + 1.0 / (n_ - 1) * dx_[i - 1];
                Q_[i][i + 1] = 1.0 / 2 * 1.0 / (n_ - 1) * dx_[i];
            }
            Q_[n_ - 1][n_ - 2] = 1.0 / 2 * 1.0 / (n_ - 1) * dx_[n_ - 2];
            Q_[n_ - 1][n_ - 1] = 1.0 / (n_ - 1) * dx_[n_ - 2];
            Matrix J_ = (I_ - V_ * inverse(transpose(V_) * Q_ * V_) * transpose(V_) * Q_) * W_;
            Array D_ = J_ * Y_;
            for (Size i = 0; i < n_ - 1; ++i)
                tmp_[i] = (Y_[i + 1] - Y_[i]) / dx_[i] - (2.0 * D_[i] + D_[i + 1]) * dx_[i] / 6.0;
            tmp_[n_ - 1] = tmp_[n_ - 2] + D_[n_ - 2] * dx_[n_ - 2] +
                           (D_[n_ - 1] - D_[n_ - 2]) * dx_[n_ - 2] / 2.0;
        }

        void cubicInterpolationSplineParabolic(Array& tmp_,
                                               std::vector<Real>& dx_,
                                               std::vector<Real>& S_,
                                               Size n_) {
            // intermediate points
            for (Size i = 1; i < n_ - 1; ++i)
                tmp_[i] = (dx_[i - 1] * S_[i] + dx_[i] * S_[i - 1]) / (dx_[i] + dx_[i - 1]);
            // end points
            tmp_[0] = ((2.0 * dx_[0] + dx_[1]) * S_[0] - dx_[0] * S_[1]) / (dx_[0] + dx_[1]);
            tmp_[n_ - 1] =
                ((2.0 * dx_[n_ - 2] + dx_[n_ - 3]) * S_[n_ - 2] - dx_[n_ - 2] * S_[n_ - 3]) /
                (dx_[n_ - 2] + dx_[n_ - 3]);
        }

        void cubicInterpolationSplineFritschButland(Array& tmp_,
                                                    std::vector<Real>& dx_,
                                                    std::vector<Real>& S_,
                                                    Size n_) {
            // intermediate points
            for (Size i = 1; i < n_ - 1; ++i) {
                Real Smin = std::min(S_[i - 1], S_[i]);
                Real Smax = std::max(S_[i - 1], S_[i]);
                if (Smax + 2.0 * Smin == 0) {
                    if (Smin * Smax < 0)
                        tmp_[i] = QL_MIN_REAL;
                    else if (Smin * Smax == 0)
                        tmp_[i] = 0;
                    else
                        tmp_[i] = QL_MAX_REAL;
                } else
                    tmp_[i] = 3.0 * Smin * Smax / (Smax + 2.0 * Smin);
            }
            // end points
            tmp_[0] = ((2.0 * dx_[0] + dx_[1]) * S_[0] - dx_[0] * S_[1]) / (dx_[0] + dx_[1]);
            tmp_[n_ - 1] =
                ((2.0 * dx_[n_ - 2] + dx_[n_ - 3]) * S_[n_ - 2] - dx_[n_ - 2] * S_[n_ - 3]) /
                (dx_[n_ - 2] + dx_[n_ - 3]);
        }

        void cubicInterpolationSplineAkima(Array& tmp_,
                                           std::vector<Real>& dx_,
                                           std::vector<Real>& S_,
                                           Size n_) {
            tmp_[0] =
                (std::abs(S_[1] - S_[0]) * 2 * S_[0] * S_[1] +
                 std::abs(2 * S_[0] * S_[1] - 4 * S_[0] * S_[0] * S_[1]) * S_[0]) /
                (std::abs(S_[1] - S_[0]) + std::abs(2 * S_[0] * S_[1] - 4 * S_[0] * S_[0] * S_[1]));
            tmp_[1] =
                (std::abs(S_[2] - S_[1]) * S_[0] + std::abs(S_[0] - 2 * S_[0] * S_[1]) * S_[1]) /
                (std::abs(S_[2] - S_[1]) + std::abs(S_[0] - 2 * S_[0] * S_[1]));
            for (Size i = 2; i < n_ - 2; ++i) {
                if ((S_[i - 2] == S_[i - 1]) && (S_[i] != S_[i + 1]))
                    tmp_[i] = S_[i - 1];
                else if ((S_[i - 2] != S_[i - 1]) && (S_[i] == S_[i + 1]))
                    tmp_[i] = S_[i];
                else if (S_[i] == S_[i - 1])
                    tmp_[i] = S_[i];
                else if ((S_[i - 2] == S_[i - 1]) && (S_[i - 1] != S_[i]) && (S_[i] == S_[i + 1]))
                    tmp_[i] = (S_[i - 1] + S_[i]) / 2.0;
                else
                    tmp_[i] = (std::abs(S_[i + 1] - S_[i]) * S_[i - 1] +
                               std::abs(S_[i - 1] - S_[i - 2]) * S_[i]) /
                              (std::abs(S_[i + 1] - S_[i]) + std::abs(S_[i - 1] - S_[i - 2]));
            }
            tmp_[n_ - 2] = (std::abs(2 * S_[n_ - 2] * S_[n_ - 3] - S_[n_ - 2]) * S_[n_ - 3] +
                            std::abs(S_[n_ - 3] - S_[n_ - 4]) * S_[n_ - 2]) /
                           (std::abs(2 * S_[n_ - 2] * S_[n_ - 3] - S_[n_ - 2]) +
                            std::abs(S_[n_ - 3] - S_[n_ - 4]));
            tmp_[n_ - 1] =
                (std::abs(4 * S_[n_ - 2] * S_[n_ - 2] * S_[n_ - 3] - 2 * S_[n_ - 2] * S_[n_ - 3]) *
                     S_[n_ - 2] +
                 std::abs(S_[n_ - 2] - S_[n_ - 3]) * 2 * S_[n_ - 2] * S_[n_ - 3]) /
                (std::abs(4 * S_[n_ - 2] * S_[n_ - 2] * S_[n_ - 3] - 2 * S_[n_ - 2] * S_[n_ - 3]) +
                 std::abs(S_[n_ - 2] - S_[n_ - 3]));
        }

        void cubicInterpolationSplineKruger(Array& tmp_,
                                            std::vector<Real>& dx_,
                                            std::vector<Real>& S_,
                                            Size n_) {
            // intermediate points
            for (Size i = 1; i < n_ - 1; ++i) {
                if (S_[i - 1] * S_[i] < 0.0)
                    // slope changes sign at point
                    tmp_[i] = 0.0;
                else
                    // slope will be between the slopes of the adjacent
                    // straight lines and should approach zero if the
                    // slope of either line approaches zero
                    tmp_[i] = 2.0 / (1.0 / S_[i - 1] + 1.0 / S_[i]);
            }
            // end points
            tmp_[0] = (3.0 * S_[0] - tmp_[1]) / 2.0;
            tmp_[n_ - 1] = (3.0 * S_[n_ - 2] - tmp_[n_ - 2]) / 2.0;
        }

        void cubicInterpolationSplineHarmonic(Array& tmp_,
                                              std::vector<Real>& dx_,
                                              std::vector<Real>& S_,
                                              Size n_) {
            // intermediate points
            for (Size i = 1; i < n_ - 1; ++i) {
                Real w1 = 2 * dx_[i] + dx_[i - 1];
                Real w2 = dx_[i] + 2 * dx_[i - 1];
                if (S_[i - 1] * S_[i] <= 0.0)
                    // slope changes sign at point
                    tmp_[i] = 0.0;
                else
                    // weighted harmonic mean of S_[i] and S_[i-1] if they
                    // have the same sign; otherwise 0
                    tmp_[i] = (w1 + w2) / (w1 / S_[i - 1] + w2 / S_[i]);
            }
            // end points [0]
            tmp_[0] = ((2 * dx_[0] + dx_[1]) * S_[0] - dx_[0] * S_[1]) / (dx_[1] + dx_[0]);
            if (tmp_[0] * S_[0] < 0.0) {
                tmp_[0] = 0;
            } else if (S_[0] * S_[1] < 0) {
                if (std::fabs(tmp_[0]) > std::fabs(3 * S_[0])) {
                    tmp_[0] = 3 * S_[0];
                }
            }
            // end points [n-1]
            tmp_[n_ - 1] =
                ((2 * dx_[n_ - 2] + dx_[n_ - 3]) * S_[n_ - 2] - dx_[n_ - 2] * S_[n_ - 3]) /
                (dx_[n_ - 3] + dx_[n_ - 2]);
            if (tmp_[n_ - 1] * S_[n_ - 2] < 0.0) {
                tmp_[n_ - 1] = 0;
            } else if (S_[n_ - 2] * S_[n_ - 3] < 0) {
                if (std::fabs(tmp_[n_ - 1]) > std::fabs(3 * S_[n_ - 2])) {
                    tmp_[n_ - 1] = 3 * S_[n_ - 2];
                }
            }
        }

        void cubicInterpolationSplineMonotonicFilter(Array& tmp_,
                                                     std::vector<Real>& dx_,
                                                     std::vector<Real>& S_,
                                                     Size n_,
                                                     std::vector<bool>& monotonicityAdjustments_) {
            Real correction;
            Real pm, pu, pd, M;
            for (Size i = 0; i < n_; ++i) {
                if (i == 0) {
                    if (tmp_[i] * S_[0] > 0.0) {
                        correction = tmp_[i] / std::fabs(tmp_[i]) *
                                     std::min<Real>(std::fabs(tmp_[i]), std::fabs(3.0 * S_[0]));
                    } else {
                        correction = 0.0;
                    }
                    if (correction != tmp_[i]) {
                        tmp_[i] = correction;
                        monotonicityAdjustments_[i] = true;
                    }
                } else if (i == n_ - 1) {
                    if (tmp_[i] * S_[n_ - 2] > 0.0) {
                        correction =
                            tmp_[i] / std::fabs(tmp_[i]) *
                            std::min<Real>(std::fabs(tmp_[i]), std::fabs(3.0 * S_[n_ - 2]));
                    } else {
                        correction = 0.0;
                    }
                    if (correction != tmp_[i]) {
                        tmp_[i] = correction;
                        monotonicityAdjustments_[i] = true;
                    }
                } else {
                    pm = (S_[i - 1] * dx_[i] + S_[i] * dx_[i - 1]) / (dx_[i - 1] + dx_[i]);
                    M = 3.0 *
                        std::min(std::min(std::fabs(S_[i - 1]), std::fabs(S_[i])), std::fabs(pm));
                    if (i > 1) {
                        if ((S_[i - 1] - S_[i - 2]) * (S_[i] - S_[i - 1]) > 0.0) {
                            pd = (S_[i - 1] * (2.0 * dx_[i - 1] + dx_[i - 2]) -
                                  S_[i - 2] * dx_[i - 1]) /
                                 (dx_[i - 2] + dx_[i - 1]);
                            if (pm * pd > 0.0 && pm * (S_[i - 1] - S_[i - 2]) > 0.0) {
                                M = std::max<Real>(M, 1.5 * std::min(std::fabs(pm), std::fabs(pd)));
                            }
                        }
                    }
                    if (i < n_ - 2) {
                        if ((S_[i] - S_[i - 1]) * (S_[i + 1] - S_[i]) > 0.0) {
                            pu = (S_[i] * (2.0 * dx_[i] + dx_[i + 1]) - S_[i + 1] * dx_[i]) /
                                 (dx_[i] + dx_[i + 1]);
                            if (pm * pu > 0.0 && -pm * (S_[i] - S_[i - 1]) > 0.0) {
                                M = std::max<Real>(M, 1.5 * std::min(std::fabs(pm), std::fabs(pu)));
                            }
                        }
                    }
                    if (tmp_[i] * pm > 0.0) {
                        correction = tmp_[i] / std::fabs(tmp_[i]) * std::min(std::fabs(tmp_[i]), M);
                    } else {
                        correction = 0.0;
                    }
                    if (correction != tmp_[i]) {
                        tmp_[i] = correction;
                        monotonicityAdjustments_[i] = true;
                    }
                }
            }
        }

        Real cubicInterpolatingPolynomialDerivative(
            Real a, Real b, Real c, Real d, Real u, Real v, Real w, Real z, Real x) {
            return (-((((a - c) * (b - c) * (c - x) * z - (a - d) * (b - d) * (d - x) * w) *
                           (a - x + b - x) +
                       ((a - c) * (b - c) * z - (a - d) * (b - d) * w) * (a - x) * (b - x)) *
                          (a - b) +
                      ((a - c) * (a - d) * v - (b - c) * (b - d) * u) * (c - d) * (c - x) *
                          (d - x) +
                      ((a - c) * (a - d) * (a - x) * v - (b - c) * (b - d) * (b - x) * u) *
                          (c - x + d - x) * (c - d))) /
                   ((a - b) * (a - c) * (a - d) * (b - c) * (b - d) * (c - d));
        }

    } // namespace detail

} // namespace QuantLib
