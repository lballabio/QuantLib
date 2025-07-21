/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003, 2004 Roman Gitlin

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

/*! \file multicubicspline.hpp
    \brief N-dimensional cubic spline interpolation between discrete points
*/

#ifndef quantlib_multi_cubic_spline_hpp
#define quantlib_multi_cubic_spline_hpp

#include <ql/errors.hpp>
#include <ql/types.hpp>
#include <algorithm>
#include <functional>
#include <utility>
#include <vector>

namespace QuantLib {

    namespace detail {

        // data structures

        typedef std::vector<std::vector<Real> > SplineGrid;

        // different termination markers are necessary
        // to maintain separation of possibly overlapping types
        struct EmptyArg {};  // arg_type termination marker
        struct EmptyRes {};  // res_type termination marker
        struct EmptyDim {};  // size_t termination marker

        template<class X> struct DataTable {
            DataTable(const std::vector<Size>::const_iterator &i) {
                std::vector<X> temp(*i, X(i + 1));
                data_table_.swap(temp);
            }
            DataTable(const SplineGrid::const_iterator &i) {
                std::vector<X> temp(i->size(), X(i + 1));
                data_table_.swap(temp);
            }
            template<class U> DataTable(const std::vector<U> &v) {
                DataTable temp(v.begin());
                data_table_.swap(temp.data_table_);
            }
            Size size() const {
                return data_table_.size();
            }
            const X &operator[](Size n) const {return data_table_[n];}
            X &operator[](Size n) {return data_table_[n];}
            std::vector<X> data_table_;
        };

        template<> struct DataTable<Real> {
            DataTable(Size n) : data_table_(n) {}
            DataTable(const std::vector<Size>::const_iterator& i)
            : data_table_(*i) {}
            DataTable(const SplineGrid::const_iterator &i)
            : data_table_(i->size()) {}
            template<class U> DataTable(const std::vector<U> &v) {
                DataTable temp(v.begin());
                data_table_.swap(temp.data_table_);
            }
            Size size() const {
                return data_table_.size();
            }
            Real operator[](Size n) const {return data_table_[n];}
            Real& operator[](Size n) {return data_table_[n];}
            std::vector<Real> data_table_;
        };

        typedef DataTable<Real> base_data_table;

        template<class X, class Y> struct Data {
            Data()
            : first(X()), second(Y()) {}
            Data(const SplineGrid::const_iterator &i)
            : first(*i), second(i + 1) {}
            Data(const SplineGrid &v)
            : first(v[0]), second(v.begin()+1) {}
            void swap(Data<X, Y> &d) noexcept {
                first.swap(d.first);
                second.swap(d.second);
            }
            X first;
            Y second;
        };

        template<> struct Data<std::vector<Real>, EmptyArg> {
            Data() = default;
            Data(const SplineGrid::const_iterator &i)
            : first(*i) {}
            Data(const SplineGrid &v)
            : first(v[0]) {}
            Data(std::vector<Real> v) : first(std::move(v)) {}
            void swap(Data<std::vector<Real>, EmptyArg> &d) noexcept {
                first.swap(d.first);
            }
            Real operator[](Size n) const {return first[n];}
            Real& operator[](Size n) {return first[n];}
            std::vector<Real> first;
            EmptyArg second;
        };

        typedef Data<std::vector<Real>, EmptyArg> base_data;

        template<class X, class Y> struct Point {
            typedef X data_type;
            Point()
            : first(data_type()), second(Y()) {}
            Point(const std::vector<Real>::const_iterator &i)
            : first(*i), second(i + 1) {}
            Point(const std::vector<Real> &v)
            : first(v[0]), second(v.begin()+1) {}
            Point(const SplineGrid::const_iterator &i)
            : first(i->size()), second(i + 1) {}
            Point(const SplineGrid &grid)
            : first(grid[0].size()), second(grid.begin()+1) {}
            operator data_type() const {
                return first;
            }
            data_type operator[](Size n) const { return n != 0U ? second[n - 1] : first; }
            data_type& operator[](Size n) { return n != 0U ? second[n - 1] : first; }
            data_type first;
            Y second;
        };

        template<> struct Point<Real, EmptyArg> {
            typedef Real data_type;
            Point(data_type s)
            : first(s) {}
            Point(const std::vector<Real>::const_iterator &i)
            : first(*i) {}
            Point(const std::vector<Real> &v)
            : first(v[0]) {}
            operator data_type() const {return first;}
            data_type operator[](Size n) const {
                QL_REQUIRE(n == 0, "operator[] : access violation");
                return first;
            }
            data_type& operator[](Size n) {
                QL_REQUIRE(n == 0, "operator[] : access violation");
                return first;
            }
            data_type first;
            EmptyArg second;
        };

        typedef Point<Real, EmptyArg> base_arg_type;

        template<> struct Point<Real, EmptyRes> {
            typedef Real data_type;
            Point()
            : first(data_type()) {}
            Point(data_type s)
            : first(s) {}
            operator data_type() const {return first;}
            const data_type &operator[](Size n) const {
                QL_REQUIRE(n == 0, "operator[] : access violation");
                return first;
            }
            data_type &operator[](Size n) {
                QL_REQUIRE(n == 0, "operator[] : access violation");
                return first;
            }
            data_type first;
            EmptyRes second;
        };

        typedef Point<Real, EmptyRes> base_return_type;

        template<> struct Point<Size, EmptyDim> {
            typedef Size data_type;
            Point()
            : first(data_type()) {}
            Point(data_type s)
            : first(s) {}
            operator data_type() const {return first;}
            data_type operator[](Size n) const {
                QL_REQUIRE(n == 0, "operator[] : access violation");
                return first;
            }
            data_type& operator[](Size n) {
                QL_REQUIRE(n == 0, "operator[] : access violation");
                return first;
            }
            data_type first;
            EmptyDim second;
        };

        typedef Point<Size, EmptyDim> base_dimensions;

        template<> struct Point<base_data_table, EmptyRes> {
            typedef base_data_table data_type;
            Point(data_type s) : first(std::move(s)) {}
            Point(const SplineGrid::const_iterator &i)
            : first(i->size()) {}
            Point(const SplineGrid &grid)
            : first(grid[0].size()) {}
            Real operator[](Size n) const {return first[n];}
            Real& operator[](Size n) {return first[n];}
            data_type first;
            EmptyRes second;
        };

        typedef Point<base_data_table, EmptyRes> base_output_data;


        // cubic spline iplementations

        // no heap memory is allocated
        // in any of the recursive calls
        class base_cubic_spline {
          public:
            typedef Real argument_type;
            typedef Real result_type;
            typedef base_data data;
            typedef base_data_table data_table;
            typedef base_output_data output_data;
            base_cubic_spline(const data &d, const data &d2,
                              const data_table& y, data_table &y2,
                              output_data &v) {
                Size dim = d.first.size();
                Size j = 1, k = 2, l = 3;
                result_type  &w = ((y2[dim] = y[1]) -= y[0]) /= d[0],
                    &u = ((y2[0] = y[2]) -= y[1]) /= d[1], &t = v[dim];
                y2[1] = -d[1] / d2[0], v[1] = 6.0 * (u - w) / d2[0];
                for(; k < dim; u = w, j = k, k = l, ++l) {
                    w = (y[l]-y[k])/d[k];
                    u = (u-w)*6.0;
                    (y2[k] = d[k]) /= ((t = -y2[j]) *= d[j]) -= d2[j];
                    (v[k] = (u += d[j] * v[j])) /= t;
                }
                y2[0] = y2[dim] = 0.0;
                while (k != 0U) {
                    (y2[k-1] *= y2[l-1]) += v[k-1];
                    --k; --l;
                }
            }
        };

        template<class X>
        class n_cubic_spline {
          public:
            typedef Data<base_data, typename X::data> data;
            typedef DataTable<typename X::data_table> data_table;
            typedef Point<base_output_data, typename X::output_data> output_data;
            n_cubic_spline(const data &d, const data &d2,
                           const data_table &y, data_table &y2, output_data &v)
            :  d_(d), d2_(d2), y_(y), y2_(y2), v_(v) {
                for(Size j = 0, dim = y_.size();  j < dim; ++j)
                    X(d_.second, d2_.second, y_[j], y2_[j], v_.second);
            }
            ~n_cubic_spline() = default;

          private:
            const data &d_, &d2_;
            const data_table &y_;
            data_table &y2_;
            output_data &v_;
        };

        class base_cubic_splint {
          public:
            typedef base_arg_type argument_type;
            typedef Real result_type;
            typedef base_data data;
            typedef base_data_table data_table;
            typedef base_dimensions dimensions;
            typedef base_output_data output_data;
            typedef base_return_type return_type;
            base_cubic_splint(const return_type &a, const return_type &b,
                              const return_type &a2, const return_type &b2,
                              const dimensions &i,
                              const data&, const data&,
                              const data_table &y, data_table &y2,
                              output_data&,
                              output_data&, output_data&,
                              result_type &res) {
                res = a * y[i] + b * y[i + 1] + a2 * y2[i] + b2 * y2[i + 1];
            }
        };

        template<class X>
        class n_cubic_splint {
          public:
            typedef Point<Real, typename X::argument_type> argument_type;
            typedef Real result_type;
            typedef Data<base_data, typename X::data> data;
            typedef DataTable<typename X::data_table> data_table;
            typedef Point<Size, typename X::dimensions> dimensions;
            typedef Point<base_output_data, typename X::output_data> output_data;
            typedef Point<result_type,
                          typename X::return_type> return_type;
            n_cubic_splint(const return_type &a, const return_type &b,
                           const return_type &a2, const return_type &b2,
                           const dimensions &i, const data &d, const data &d2,
                           const data_table &y, data_table &y2, output_data &v,
                           output_data &v1, output_data &v2,
                           result_type& r)
            :  a_(a), b_(b), a2_(a2), b2_(b2), i_(i), d_(d), d2_(d2),
               y_(y), y2_(y2), v_(v), v1_(v1), v2_(v2) {
                for(Size j = 0, dim = y_.size(); j < dim; ++j)
                    X(a_.second, b_.second, a2_.second, b2_.second, i_.second,
                      d_.second, d2_.second, y_[j], y2_[j], v_.second,
                      v1_.second, v2_.second, v1_.first[j]);
                base_cubic_spline(d_.first, d2_.first,
                                  v1_.first.first, v2_.first.first, v_.first);
                base_cubic_splint(a_.first, b_.first, a2_.first, b2_.first,
                                  i_.first, d_.first, d2_.first,
                                  v1_.first.first, v2_.first.first,
                                  v_.first, v1_.first, v2_.first, r);
            }
            ~n_cubic_splint() = default;

          private:
            const return_type &a_, &b_, &a2_, &b2_;
            const dimensions &i_;
            const data &d_, &d2_;
            const data_table &y_;
            data_table &y2_;
            output_data &v_, &v1_, &v2_;
        };

        typedef base_cubic_spline               cubic_spline_01;
        typedef n_cubic_spline<cubic_spline_01> cubic_spline_02;
        typedef n_cubic_spline<cubic_spline_02> cubic_spline_03;
        typedef n_cubic_spline<cubic_spline_03> cubic_spline_04;
        typedef n_cubic_spline<cubic_spline_04> cubic_spline_05;
        typedef n_cubic_spline<cubic_spline_05> cubic_spline_06;
        typedef n_cubic_spline<cubic_spline_06> cubic_spline_07;
        typedef n_cubic_spline<cubic_spline_07> cubic_spline_08;
        typedef n_cubic_spline<cubic_spline_08> cubic_spline_09;
        typedef n_cubic_spline<cubic_spline_09> cubic_spline_10;
        typedef n_cubic_spline<cubic_spline_10> cubic_spline_11;
        typedef n_cubic_spline<cubic_spline_11> cubic_spline_12;
        typedef n_cubic_spline<cubic_spline_12> cubic_spline_13;
        typedef n_cubic_spline<cubic_spline_13> cubic_spline_14;
        typedef n_cubic_spline<cubic_spline_14> cubic_spline_15;

        typedef base_cubic_splint               cubic_splint_01;
        typedef n_cubic_splint<cubic_splint_01> cubic_splint_02;
        typedef n_cubic_splint<cubic_splint_02> cubic_splint_03;
        typedef n_cubic_splint<cubic_splint_03> cubic_splint_04;
        typedef n_cubic_splint<cubic_splint_04> cubic_splint_05;
        typedef n_cubic_splint<cubic_splint_05> cubic_splint_06;
        typedef n_cubic_splint<cubic_splint_06> cubic_splint_07;
        typedef n_cubic_splint<cubic_splint_07> cubic_splint_08;
        typedef n_cubic_splint<cubic_splint_08> cubic_splint_09;
        typedef n_cubic_splint<cubic_splint_09> cubic_splint_10;
        typedef n_cubic_splint<cubic_splint_10> cubic_splint_11;
        typedef n_cubic_splint<cubic_splint_11> cubic_splint_12;
        typedef n_cubic_splint<cubic_splint_12> cubic_splint_13;
        typedef n_cubic_splint<cubic_splint_13> cubic_splint_14;
        typedef n_cubic_splint<cubic_splint_14> cubic_splint_15;

        template<Size i> struct Int2Type {
            typedef cubic_spline_01 c_spline;
            typedef cubic_splint_01 c_splint;
        };

        template<> struct Int2Type<2> {
            typedef cubic_spline_02 c_spline;
            typedef cubic_splint_02 c_splint;
        };

        template<> struct Int2Type<3> {
            typedef cubic_spline_03 c_spline;
            typedef cubic_splint_03 c_splint;
        };

        template<> struct Int2Type<4> {
            typedef cubic_spline_04 c_spline;
            typedef cubic_splint_04 c_splint;
        };

        template<> struct Int2Type<5> {
            typedef cubic_spline_05 c_spline;
            typedef cubic_splint_05 c_splint;
        };

        template<> struct Int2Type<6> {
            typedef cubic_splint_06 c_splint;
            typedef cubic_spline_06 c_spline;
        };

        template<> struct Int2Type<7> {
            typedef cubic_spline_07 c_spline;
            typedef cubic_splint_07 c_splint;
        };

        template<> struct Int2Type<8> {
            typedef cubic_spline_08 c_spline;
            typedef cubic_splint_08 c_splint;
        };

        template<> struct Int2Type<9> {
            typedef cubic_spline_09 c_spline;
            typedef cubic_splint_09 c_splint;
        };

        template<> struct Int2Type<10> {
            typedef cubic_spline_10 c_spline;
            typedef cubic_splint_10 c_splint;
        };

        template<> struct Int2Type<11> {
            typedef cubic_splint_11 c_splint;
            typedef cubic_spline_11 c_spline;
        };

        template<> struct Int2Type<12> {
            typedef cubic_spline_12 c_spline;
            typedef cubic_splint_12 c_splint;
        };

        template<> struct Int2Type<13> {
            typedef cubic_spline_13 c_spline;
            typedef cubic_splint_13 c_splint;
        };

        template<> struct Int2Type<14> {
            typedef cubic_spline_14 c_spline;
            typedef cubic_splint_14 c_splint;
        };

        template<> struct Int2Type<15> {
            typedef cubic_spline_15 c_spline;
            typedef cubic_splint_15 c_splint;
        };

    }


    // Multi-cubic spline

    typedef detail::SplineGrid SplineGrid;

    //! N-dimensional cubic spline interpolation between discrete points
    /*! \test interpolated values are checked against the original
              function.

        \todo
        - allow extrapolation as for the other interpolations
        - investigate if and how to implement Hyman filters and
          different boundary conditions

        \bug cannot interpolate at the grid points on the boundary
             surface of the N-dimensional region
    */
    template <Size i> class MultiCubicSpline {
        typedef typename detail::Int2Type<i>::c_spline c_spline;
        typedef typename detail::Int2Type<i>::c_splint c_splint;
      public:
        typedef typename c_splint::argument_type argument_type;
        typedef typename c_splint::result_type result_type;
        typedef typename c_splint::data_table data_table;
        typedef typename c_splint::return_type return_type;
        typedef typename c_splint::output_data output_data;
        typedef typename c_splint::dimensions dimensions;
        typedef typename c_splint::data data;
        MultiCubicSpline(const SplineGrid& grid, const data_table &y,
                         const std::vector<bool>& ae =
                                             std::vector<bool>(20, false))
        : grid_(grid), y_(y), ae_(ae), v_(grid), v1_(grid),
          v2_(grid), y2_(grid) {
            set_shared_increments();
            c_spline(d_, d2_, y_, y2_, v_);
        }
        result_type operator()(const argument_type& x) const {
            set_shared_coefficients(x);
            c_splint(a_, b_, a2_, b2_, i_, d_, d2_, y_, y2_,
                     v_, v1_, v2_, res_);
            return res_;
        }
        void set_shared_increments() const;
        void set_shared_coefficients(const argument_type &x) const;
      private:
        const SplineGrid &grid_;
        const data_table &y_;
        const std::vector<bool> &ae_;
        mutable return_type a_, b_, a2_, b2_;
        mutable output_data v_, v1_, v2_;
        mutable result_type res_;
        mutable dimensions i_;
        mutable data d_, d2_;
        mutable data_table y2_;
    };

    // the data is checked and, in case of insufficient number of points,
    // exception is thrown BEFORE the main body of interpolation begins
    template <Size i>
    void MultiCubicSpline<i>::set_shared_increments() const {
        SplineGrid x(i), y(i);
        Size k = 0, dim = 0;
        for(Size j = 0; j < i; k = 0, ++j) {
            const std::vector<Real> &v = grid_[j];
            if((dim = v.size() - 1) > 2) {
                std::vector<Real> tmp1(dim);
                x[j].swap(tmp1);
                std::vector<Real> tmp2(dim - 1);
                y[j].swap(tmp2);
                for(; k < dim; ++k) {
                    if((x[j][k] = v[k + 1] - v[k]) <= 0.0) break;
                    if (k != 0U)
                        y[j][k - 1] = 2.0 * (v[k + 1] - v[k - 1]);
                }
            }
            QL_REQUIRE(dim >= 3,
                       "Dimension " << j
                       << " : not enough points for interpolation");
            QL_REQUIRE(k >= dim,
                       "Dimension " << j << " : invalid data");
        }

        typename c_splint::data tmp1(x), tmp2(y);
        d_.swap(tmp1);
        d2_.swap(tmp2);
    }

    #ifndef __DOXYGEN__
    // the argument value is checked and, in out of boundaries case,
    // exception is thrown BEFORE the main body of interpolation begins
    template <Size i>
    void MultiCubicSpline<i>::set_shared_coefficients(
                 const typename MultiCubicSpline<i>::argument_type &x) const {
        for(Size j = 0; j < i; ++j) {
            Size &k = i_[j], sz = grid_[j].size() - 1;
            const std::vector<Real> &v = grid_[j];
            if(x[j] < v[0] || x[j] >= v[sz]) {
                QL_REQUIRE(ae_[j],
                           "Dimension " << j
                           << ": extrapolation is not allowed.");
                a_[j] = 1.0, a2_[j] = b_[j] = b2_[j] = 0.0;
                k =  x[j] < v[0] ? 0 : sz;
            }
            else {
                k = v[k] <= x[j] && x[j] < v[k + 1] ? k :
                    std::upper_bound(v.begin(),v.end(),x[j])-v.begin()-1;
                Real h = v[k + 1] - v[k];
                a_[j] = (v[k + 1] - x[j]) / h, b_[j] = (x[j] - v[k]) / h;
                a2_[j] = (a_[j] * a_[j] * a_[j] - a_[j]) * h * h / 6.0,
                    b2_[j] = (b_[j] * b_[j] * b_[j] - b_[j]) * h * h / 6.0;
            }
        }
    }
    #endif

}


#endif

