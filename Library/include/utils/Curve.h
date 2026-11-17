/*
    This file is a part of Stonefish.

    Stonefish is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Stonefish is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

//
//  Curve.h
//  Stonefish
//
//  Piecewise-linear curve defined by data points, used to pass measured or
//  tabulated characteristics (propeller kT(J)/kQ(J),
//  foil CL/CD(alpha)) from the scenario instead of model-specific parameters.
//

#pragma once

#include "StonefishCommon.h"
#include <algorithm>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace sf
{
    //! A piecewise-linear curve y(x) through a set of data points.
    class Curve
    {
    public:
        //! Behaviour outside the range of the data points.
        enum class Ends {CLAMP, EXTRAPOLATE};

        //! An empty curve.
        Curve() : ends_(Ends::CLAMP) {}

        //! A constructor.
        /*!
         \param x strictly increasing abscissae (at least two points)
         \param y ordinates, same size as x
         \param ends behaviour outside [x.front(), x.back()]
         */
        Curve(std::vector<Scalar> x, std::vector<Scalar> y, Ends ends = Ends::CLAMP)
            : x_(std::move(x)), y_(std::move(y)), ends_(ends)
        {
            if (x_.size() < 2 || x_.size() != y_.size())
                throw std::runtime_error("Curve: need at least two points and as many x as y values!");
            for (size_t i = 1; i < x_.size(); ++i)
                if (!(x_[i] > x_[i-1]))
                    throw std::runtime_error("Curve: x values must be strictly increasing!");
        }

        //! A method building a curve from two whitespace-separated lists of numbers.
        /*!
         \param x list of abscissae, e.g. "0 0.1 0.2"
         \param y list of ordinates
         \param ends behaviour outside the data range
         \return the curve
         */
        static Curve Parse(const std::string& x, const std::string& y, Ends ends = Ends::CLAMP)
        {
            return Curve(ToVector(x), ToVector(y), ends);
        }

        //! A method returning true if the curve has no points.
        bool IsEmpty() const { return x_.empty(); }

        //! A method evaluating the curve.
        /*!
         \param x the abscissa
         \return the interpolated ordinate
         */
        Scalar operator()(Scalar x) const
        {
            size_t i;
            if (x <= x_.front())
            {
                if (ends_ == Ends::CLAMP) return y_.front();
                i = 1;
            }
            else if (x >= x_.back())
            {
                if (ends_ == Ends::CLAMP) return y_.back();
                i = x_.size() - 1;
            }
            else
                i = std::upper_bound(x_.begin(), x_.end(), x) - x_.begin();
            return y_[i-1] + (x - x_[i-1]) * (y_[i] - y_[i-1]) / (x_[i] - x_[i-1]);
        }

        //! A method returning the abscissae.
        const std::vector<Scalar>& GetX() const { return x_; }

        //! A method returning the ordinates.
        const std::vector<Scalar>& GetY() const { return y_; }

    private:
        static std::vector<Scalar> ToVector(const std::string& str)
        {
            std::vector<Scalar> v;
            std::istringstream ss(str);
            Scalar s;
            while (ss >> s) v.push_back(s);
            if (!ss.eof())
                throw std::runtime_error("Curve: cannot parse number list '" + str + "'!");
            return v;
        }

        std::vector<Scalar> x_;
        std::vector<Scalar> y_;
        Ends ends_;
    };
}
