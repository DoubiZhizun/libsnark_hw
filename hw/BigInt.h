#pragma once
#include <stdio.h>
#include <stdint.h>
#include <assert.h>

template<uint32_t L>
class BigInt{
    public:
    BigInt() {
        assert(L > 0);
    }

    BigInt(const uint32_t &that) {
        if (L <= 32) {
            num[0] = that & ((1u << (L % 32)) - 1u);
        } else {
            num[0] = that;
            for (uint32_t i = 1; i < wordNum(); i++) {
                num[i] = 0;
            }
        }
    }

    BigInt(const uint32_t that[(L + 31) / 32]) {
        for (uint32_t i = 0; i < wordNum(); i++) {
            if (i == wordNum() - 1 && L % 32 > 0) {
                num[i] = that[i] & ((1u << (L % 32)) - 1u);
            } else {
                num[i] = that[i];
            }
        }
    }

    template<uint32_t L2>
    BigInt(const BigInt<L2> &that) {
        for (uint32_t i = 0; i < wordNum(); i++) {
            if (i < that.wordNum()) {
                if (i == wordNum() - 1 && L % 32 > 0) {
                    num[i] = that[i] & ((1u << (L % 32)) - 1u);
                } else {
                    num[i] = that[i];
                }
            } else {
                num[i] = 0;
            }
        }
    }

    uint32_t wordNum() const {
        return (L + 31) / 32;
    }

    bool testBit(uint32_t n) const {
        return num[n / 32] & (1u << (n % 32));
    }

    bool msb() const {
        return testBit(L - 1);
    }

    bool lsb() const {
        return testBit(0);
    }

    void setBit(uint32_t n) {
        num[n / 32] |= 1u << (n % 32);
    }

    uint32_t &operator[](uint32_t n) {
        return num[n];
    }

    const uint32_t &operator[](uint32_t n) const {
        return num[n];
    }

    BigInt<L + 1> operator+(const BigInt<L> &that) const {
        BigInt<L + 1> sum;
        uint64_t carry = 0;
        for (uint32_t i = 0; i < sum.wordNum(); i++) {
            if (i < wordNum()) {
                carry += (uint64_t)(num[i]) + (uint64_t)(that[i]);
                sum[i] = (uint32_t)carry;
                carry >>= 32;
            } else {
                sum[i] = (uint32_t)carry;
            }
        }
        return sum;
    }

    BigInt<L + 1> operator-(const BigInt<L> &that) const {
        BigInt<L + 1> sum;
        int64_t carry = 0;
        for (uint32_t i = 0; i < sum.wordNum(); i++) {
            if (i < wordNum()) {
                carry += (uint64_t)(num[i]) - (uint64_t)(that[i]);
                sum[i] = (uint32_t)carry;
                carry >>= 32;
            } else {
                sum[i] = (uint32_t)carry;
            }
        }
        return sum;
    }

    BigInt<L + L> operator*(const BigInt<L> &that) const {
        BigInt<L + L> product;
        for (uint32_t i = 0; i < wordNum(); i++) {
            product[i] = 0;
        }
        uint64_t temp;
        for (uint32_t i = 0; i < wordNum(); i++) {
            temp = 0;
            for (uint32_t j = 0; j < wordNum(); j++) {
                temp += (uint64_t)num[i] * (uint64_t)that[j] + product[i + j];
                product[i + j] = (uint32_t)temp;
                temp >>= 32;
            }

            if (wordNum() + i < product.wordNum()) {
                product[wordNum() + i] = (uint32_t)temp;
            }
        }
        return product;
    }

    template<uint32_t L2>
    BigInt<L> &operator=(const BigInt<L2> &that) {
        *this = BigInt<L>(that);
        return *this;
    }

    bool operator==(const BigInt<L> &that) const {
        for (uint32_t i = 0; i < wordNum(); i++) {
            if (num[i] != that[i]) {
                return false;
            }
        }
        return true;
    }

    BigInt<L> operator<<(uint32_t n) const {
        if (n >= L) return 0u;

        BigInt<L> shift;
        uint32_t i;
        for (i = 0; i < n / 32; i++) {
            shift[i] = 0;
        }
        shift[i] = num[0] << (n % 32);
        for (i++; i < wordNum(); i++) {
            shift[i] = (num[i - n / 32] << (n % 32)) | (num[i - n / 32 - 1] >> (32 - n % 32));
        }
        if (L % 32 > 0) {
            shift[(L - 1) / 32] &= (1u << (L % 32)) - 1u;
        }
        return shift;
    }

    BigInt<L> operator>>(uint32_t n) const {
        if (n >= L) return 0u;

        BigInt<L> shift;
        uint32_t i;
        for (i = 0; i < wordNum() - 1 - n / 32; i++) {
            shift[i] = (num[n /32 + i] >> (n % 32)) | (num[n / 32 + i + 1] << (32 - n % 32));
        }
        shift[i] = num[n / 32 + i] >> (n % 32);
        for (i++; i < wordNum(); i++) {
            shift[i] = 0;
        }
        return shift;
    }

    void print(const char *endWith = "") const {
        for (uint32_t i = 0; i < (L + 31) / 32; i++) {
            printf("%08x", num[(L + 31) / 32 - 1 - i]);
        }
        printf("%s", endWith);
    }

    private:
    uint32_t num[(L + 31) / 32];
};