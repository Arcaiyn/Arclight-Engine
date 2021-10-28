#pragma once

#include "math.h"
#include "vector.h"
#include "rectangle.h"

#include <array>
#include <vector>
#include <span>


template<u32 Degree, Float F>
class Bezier {

public:

    static_assert(Degree, "Degree must be at least 1");

    using Type = F;
    constexpr static u32 Order = Degree;


    constexpr Bezier() : Bezier(Vec2<F>(0, 0)) {}

    template<Vector... V>
    constexpr Bezier(const V&... cps) : controlPoints {cps...} {}

    template<Vector V>
    constexpr Bezier(const std::span<V>& cps) {
        std::copy(cps.begin(), cps.end(), controlPoints);
        std::fill_n(&controlPoints[cps.size()], Math::max<i32>(Degree + 1 - cps.size(), 0), Vec2<F>(0, 0));
    }


    constexpr Vec2<F> evaluate(double t) const {

        if constexpr (Degree == 1) {
            return Math::lerp(controlPoints[0], controlPoints[1], t);
        } else {
            return evaluateHelper(t, std::make_index_sequence<Degree>{});
        }

    }


    constexpr Bezier<Degree - 1, F> derivative() const {

        static_assert(Degree >= 2, "Derivative is not a bezier curve");

        std::array<Vec2<F>, Degree> a;

        for(u32 i = 0; i < Degree; i++) {
            a[i] = Degree * (controlPoints[i + 1] - controlPoints[i]);
        }

        return Bezier<Degree - 1, F>(std::span{a.data(), a.size()});

    }

    constexpr Bezier<Degree - 2, F> secondDerivative() const {

        static_assert(Degree >= 3, "Second derivative is not a bezier curve");

        std::array<Vec2<F>, Degree - 1> a;

        Vec2<F> first = Degree * (controlPoints[1] - controlPoints[0]);

        for(u32 i = 0; i < Degree; i++) {

            Vec2<F> second = Degree * (controlPoints[i + 2] - controlPoints[i + 1]);
            a[i] = (Degree - 1) * (second - first);
            first = second;

        }

        return Bezier<Degree - 2, F>(std::span{a.data(), a.size()});

    }


    constexpr Rectangle<F> boundingBox() const {

        if constexpr (Degree == 1) {

            //Trivial case, simply enclose line by rect
            return Rectangle<F>::fromPoints(controlPoints[0], controlPoints[1]);

        } else if constexpr (Degree <= 5) {

            //Linear derivative
            Bezier<Degree - 1, F> b = derivative();

            const Vec2<F>& d0 = b.getStartPoint();
            const Vec2<F>& d1 = b.getEndPoint();

            F lx = getStartPoint().x;
            F hx = getEndPoint().x;
            F ly = getStartPoint().y;
            F hy = getEndPoint().y;

            Math::ascOrder(lx, hx);
            Math::ascOrder(ly, hy);

            if constexpr (Degree == 2) {

                if(!Math::isEqual(d1.x, d0.x)) {

                    F t = -d0.x / (d1.x - d0.x);

                    if(t >= 0.0 && t <= 1.0) {

                        Vec2<F> p = evaluate(t);
                        lx = Math::min(lx, p.x);
                        hx = Math::max(hx, p.x);

                    }

                }

                if(!Math::isEqual(d1.y, d0.y)) {

                    F t = -d0.y / (d1.y - d0.y);
                                    
                    if(t >= 0.0 && t <= 1.0) {

                        Vec2<F> p = evaluate(t);
                        ly = Math::min(ly, p.y);
                        hy = Math::max(hy, p.y);

                    }

                }

            } else if constexpr (Degree == 3) {


                
            }

            return Rectangle<F>::fromPoints(Vec2<F>(lx, ly), Vec2<F>(hx, hy));

        } else {

            static_assert("Bezier bounding boxes above degree 5 are unsupported");

        }

    }


    constexpr Vec2<F> getStartPoint() const {
        return getControlPoint<0>();
    }

    constexpr Vec2<F> getEndPoint() const {
        return getControlPoint<Degree>();
    }

    template<u32 I>
    constexpr Vec2<F> getControlPoint() const requires (I < Degree + 1) {
        return controlPoints[I];
    }

    constexpr Vec2<F> getControlPoint(u32 i) const {
        return controlPoints[i];
    }


    constexpr void setStartPoint(const Vec2<F>& start) {
        setControlPoint<0>(start);
    }

    constexpr void setEndPoint(const Vec2<F>& end) {
        setControlPoint<Degree>(end);
    }

    template<u32 I>
    constexpr void setControlPoint(const Vec2<F>& point) requires (I < Degree + 1) {
        this->controlPoints[I] = point;
    }

    constexpr void setControlPoint(u32 i, const Vec2<F>& point) {
        this->controlPoints[i] = point;
    }


    Vec2<F> controlPoints[Degree + 1];

private:

    template<SizeT... Pack>
    constexpr auto evaluateHelper(double t, std::index_sequence<Pack...>) const {

        constexpr static bool useHeap = Degree > 128;
        constexpr static SizeT Count = sizeof...(Pack);
        using Container = std::conditional_t<useHeap, std::vector<Vec2<F>>, std::array<Vec2<F>, Count>>;

        Container c;

        if constexpr (useHeap) {
            c.resize(Count);
        }

        for(SizeT i = 0; i < c.size(); i++) {
            c[i] = Math::lerp(controlPoints[i], controlPoints[i + 1], t);
        }

        return Bezier<Degree - 1, F>(std::span{c.data(), c.size()}).evaluate(t);

    }

};



#define BEZIER_DEFINE_NDTS(name, degree, type, suffix) typedef Bezier<degree, type> name##degree##suffix;

#define BEZIER_DEFINE_ND(name, degree) \
	BEZIER_DEFINE_NDTS(name, degree, float, f) \
	BEZIER_DEFINE_NDTS(name, degree, double, d) \
	BEZIER_DEFINE_NDTS(name, degree, long double, ld) \
	BEZIER_DEFINE_NDTS(name, degree, ARC_STD_FLOAT_TYPE, x)

#define BEZIER_DEFINE_N(name) \
    BEZIER_DEFINE_ND(name, 1) \
    BEZIER_DEFINE_ND(name, 2) \
    BEZIER_DEFINE_ND(name, 3) \
    BEZIER_DEFINE_ND(name, 4) \
    BEZIER_DEFINE_ND(name, 5)

#define BEZIER_DEFINE \
	BEZIER_DEFINE_N(Bezier) \
	BEZIER_DEFINE_N(BezierCurve)

BEZIER_DEFINE

#undef BEZIER_DEFINE
#undef BEZIER_DEFINE_N
#undef BEZIER_DEFINE_ND
#undef BEZIER_DEFINE_NDTS
