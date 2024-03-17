#ifndef _HW_H_
#define _HW_H_

#include <stdint.h>

class hw_interface {
public:
    virtual void hw_init() = 0;
    virtual void *hw_ntt_i() = 0;
    virtual const void *hw_ntt_o() const = 0;
    virtual void hw_ntt(uint32_t log2N, bool is_inv) = 0;
    virtual void *hw_msm_scalar_i() = 0;
    virtual void *hw_msm_point_i() = 0;
    virtual const void *hw_msm_o() const = 0;
    virtual void hw_msm(uint64_t N) = 0;
};

hw_interface *hw_get_instance();

#endif