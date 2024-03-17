#pragma once
#include <stdlib.h>
#include "BigInt.h"

template <uint32_t L>
class Zp{
    public:
        Zp(const BigInt<L> &_m) {
            assert(L > 0);
            assert(_m.lsb());

            m = _m;
            
            u = 0u;
            BigInt<L + 1> temp = 0u;
            temp.setBit(L);

            u.setBit(L);
            temp = temp - _m;

            for (uint32_t i = 0; i < L; i++) {
                temp = temp << 1;
                BigInt<L + 2> temp2 = temp - _m;
                if (!temp2.msb()) {
                    u.setBit(L - 1 - i);
                    temp = temp2;
                }

            }
        }

        Zp(const Zp<L> &that) {
            m = that.m;
            u = that.u;
        }

        Zp<L> &operator=(const Zp<L> &that) {
            *this = Zp(that);
            return *this;
        }

        BigInt<L> add(const BigInt<L> &a, const BigInt<L> &b) const {
            BigInt<L + 1> sum = a + b;
            BigInt<L + 1> temp = sum - m;
            if (temp.msb()) return sum;
            return temp;
        }

        BigInt<L> sub(const BigInt<L> &a, const BigInt<L> &b) const {
            BigInt<L + 1> sum = a - b;
            if (sum.msb()) return sum + m;
            return sum;
        }

        BigInt<L> barrett(const BigInt<L + L> &a) const {
            BigInt<L + 1> b = a >> (L - 1);
            BigInt<L + 1 + L + 1> c = b * u;
            BigInt<L + 1> d = c >> (L + 1);
            BigInt<L + 2> e = d * m;
            BigInt<L + 2> product = a - e;

            BigInt<L + 2> f = product - m;
            if (f.msb()) return product;
            BigInt<L + 1> g = f - m;
            if (g.msb()) return f;
            return g;
        }

        BigInt<L> mul(const BigInt<L> &a, const BigInt<L> &b) const {
            return barrett(a * b);
        }

        template<uint32_t c>
        BigInt<L> cmul(const BigInt<L> &a) const {
            BigInt<L> sum = 0u;
            for (int i = 31; i >= 0; i--) {
                sum = add(sum, sum);
                if ((c & (1u << i)) != 0) sum = add(sum, a);
            }
            return sum;
        }

        template<uint32_t L2>
        BigInt<L> pow(const BigInt<L> &a, const BigInt<L2> &e) const {
            BigInt<L> product;
            product[0] = 1;
            for (uint32_t i = 1; i < product.wordNum(); i++) {
                product[i] = 0;
            }

            uint32_t i;
            for (i = 0; i < L2; i++) {
                if (e.testBit(L2 - 1 - i)) {
                    goto calc;
                }
            }
            for (; i < L2; i++) {
                product = mul(product, product);
                if (e.testBit(L2 - 1 - i)) {
                    calc: product = mul(product, a);
                }
            }
            return product;
        }

        BigInt<L> inv(const BigInt<L> &a) const {
            return pow<L>(a, m - 2);
        }

        uint32_t getPrimitiveRoot() {
            uint32_t g = 2;
            while (g < 100) {
                if (pow(g, BigInt<L - 1>(m >> 1)) == 1) {
                    g++;
                } else {
                    return g;
                }
            }
            return 0;
        }

        const BigInt<L> &getModular() const {
            return m;
        }

    private:
        BigInt<L> m;
        BigInt<L + 1> u;
};
