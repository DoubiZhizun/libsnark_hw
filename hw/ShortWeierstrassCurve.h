#pragma once
#include "Zp.h"
#include <thread>

template <uint32_t L>
struct ShortWeierstrassPoint {
    BigInt<L> X;
    BigInt<L> Y;
    BigInt<L> Z;
};

template<uint32_t L, uint32_t L_order, uint32_t _a, uint32_t _b>
class ShortWeierstrassCurve {
    public:
        ShortWeierstrassCurve(Zp<L> _zp, Zp<L_order> _orderZp): zp(_zp), orderZp(_orderZp) {}

        ShortWeierstrassPoint<L> pointAdd(const ShortWeierstrassPoint<L> &a, const ShortWeierstrassPoint<L> &b) const {
            ShortWeierstrassPoint<L> output;

            BigInt<L> t0 = zp.mul(a.X, b.X);
            BigInt<L> t1 = zp.mul(a.Y, b.Y);
            BigInt<L> t2 = zp.mul(a.Z, b.Z);
            BigInt<L> t3 = zp.add(a.X, a.Y);
            BigInt<L> t4 = zp.add(b.X, b.Y);
            t3 = zp.mul(t3, t4);
            t4 = zp.add(t0, t1);
            t3 = zp.sub(t3, t4);
            t4 = zp.add(a.X, a.Z);
            BigInt<L> t5 = zp.add(b.X, b.Z);
            t4 = zp.mul(t4, t5);
            t5 = zp.add(t0, t2);
            t4 = zp.sub(t4, t5);
            t5 = zp.add(a.Y, a.Z);
            output.X = zp.add(b.Y, b.Z);
            t5 = zp.mul(t5, output.X);
            output.X = zp.add(t1, t2);
            t5 = zp.sub(t5, output.X);
            if (_a > 0) output.Z = zp.template cmul<_a>(t4);
            if (_b > 0) output.X = zp.template cmul<3 * _b>(t2);
            if (_b > 0) {if(_a > 0) output.Z = zp.add(output.Z, output.X); else output.Z = output.X;}
            if (_a > 0 || _b > 0) output.X = zp.sub(t1, output.Z); else output.X = t1;
            if (_a > 0 || _b > 0) output.Z = zp.add(t1, output.Z); else output.Z = t1;
            output.Y = zp.mul(output.X, output.Z);
            t1 = zp.template cmul<3>(t0);
            if (_a > 0) t2 = zp.template cmul<_a>(t2);
            if (_b > 0) t4 = zp.template cmul<3 * _b>(t4);
            if (_a > 0) t1 = zp.add(t1, t2);
            if (_a > 0) t2 = zp.sub(t0, t2); else t2 = t0;
            if (_a > 0) t2 = zp.template cmul<_a>(t2);
            if (_a > 0) {if (_b > 0) t4 = zp.add(t4, t2); else t4 = t2;}
            if (_a > 0 || _b > 0) t0 = zp.mul(t1, t4);
            if (_a > 0 || _b > 0) output.Y = zp.add(output.Y, t0);
            if (_a > 0 || _b > 0) t0 = zp.mul(t5, t4);
            output.X = zp.mul(t3, output.X);
            if (_a > 0 || _b > 0) output.X = zp.sub(output.X, t0);
            t0 = zp.mul(t3, t1);
            output.Z = zp.mul(t5, output.Z);
            output.Z = zp.add(output.Z, t0);
            
            return output;
        }

        ShortWeierstrassPoint<L> uniformPoint(const ShortWeierstrassPoint<L> &a) const {
            BigInt<L> ZInv = zp.inv(a.Z);
            ShortWeierstrassPoint<L> output;
            output.X = zp.mul(a.X, ZInv);
            output.Y = zp.mul(a.Y, ZInv);
            output.Z = 1u;
            return output;
        }

        ShortWeierstrassPoint<L> getZeroPoint() const {
            ShortWeierstrassPoint<L> output;
            output.X = 0u;
            output.Y = 1u;
            output.Z = 0u;
            return output;
        }

        ShortWeierstrassPoint<L> pointMul(const ShortWeierstrassPoint<L> &a, const BigInt<L_order> &b) const {
            ShortWeierstrassPoint<L> sum = getZeroPoint();
            ShortWeierstrassPoint<L> a2 = a;
            for (uint32_t i = 0; i < L_order; i++) {
                if (b.testBit(i)) sum = pointAdd(sum, a2);
                a2 = pointAdd(a2, a2);
            }
            return sum;
        }

    private:
        Zp<L> zp;
        Zp<L_order> orderZp;
};