#include "hw.h"

#include "experimental/xrt_bo.h"
#include "experimental/xrt_device.h"
#include "experimental/xrt_kernel.h"
#include "experimental/xrt_ip.h"

#include "BigInt.h"
#include "Zp.h"
#include "ShortWeierstrassCurve.h"

static uint32_t _m[8] = {
    0xd87cfd47, 0x3c208c16, 0x6871ca8d, 0x97816a91,
    0x8181585d, 0xb85045b6, 0xe131a029, 0x30644e72,
};
static uint32_t _order[8] = {
    0xf0000001, 0x43e1f593, 0x79b97091, 0x2833e848,
    0x8181585d, 0xb85045b6, 0xe131a029, 0x30644e72,
};
static uint32_t _R[8] = {
    0xc58f0d9d, 0xd35d438d, 0xf5c70b3d, 0x0a78eb28,
    0x7879462c, 0x666ea36f, 0x9a07df2f, 0x0e0a77c1,
};
BigInt<254> R = _R;
static uint32_t _RInv[8] = {
    0x6db1194e, 0xdc5ba005, 0xe111ec87, 0x090ef5a9,
    0xaeb85d5d, 0xc8260de4, 0x82c5551c, 0x15ebf951,
};
BigInt<254> RInv = _RInv;

class hw_device: public hw_interface {
public:
    hw_device() {}

    ~hw_device() {
        if (!initialized) return;

        delete zp;
        delete curve;
        delete device;
        delete ntt_ip;
        delete msm_ip;
        delete ntt_i;
        delete ntt_o;
        delete msm_scalar_i;
        delete msm_point_i;
        delete msm_o;
    }

    virtual void hw_init() {
        if (initialized) return;
        initialized = true;

        printf("Initializing ...\n");

        BigInt<254> m = _m;
        BigInt<254> order = _order;

        zp = new Zp<254>(m);
        Zp<254> order_zp = Zp<254>(order);
        curve = new ShortWeierstrassCurve<254, 254, 0, 3>(*zp, order_zp);

        uint32_t device_index = 0;
        device = new xrt::device(device_index);
        auto uuid = device->load_xclbin("../zk_SNARK.xclbin");

        ntt_ip = new xrt::ip(*device, uuid, "NTT6StepU250");
        msm_ip = new xrt::ip(*device, uuid, "MSMU250");

        ntt_i = new xrt::bo(*device, sizeof(BigInt<254>) << 27, xrt::bo::flags::normal, 0);
        ntt_o = new xrt::bo(*device, sizeof(BigInt<254>) << 27, xrt::bo::flags::normal, 1);
        msm_scalar_i = new xrt::bo(*device, sizeof(BigInt<254>) << 27, xrt::bo::flags::normal, 2);
        msm_point_i = new xrt::bo(*device, (sizeof(BigInt<254>) * 2) << 27, xrt::bo::flags::normal, 3);
        msm_o = new xrt::bo(*device, sizeof(BigInt<254>) * 3, xrt::bo::flags::normal, 2);

        ntt_ip->write_register(0x20, ntt_i->address());
        ntt_ip->write_register(0x24, ntt_i->address() >> 32);
        ntt_ip->write_register(0x28, ntt_o->address());
        ntt_ip->write_register(0x2c, ntt_o->address() >> 32);

        msm_ip->write_register(0x20, msm_scalar_i->address());
        msm_ip->write_register(0x24, msm_scalar_i->address() >> 32);
        msm_ip->write_register(0x28, msm_point_i->address());
        msm_ip->write_register(0x2c, msm_point_i->address() >> 32);
        msm_ip->write_register(0x30, msm_o->address());
        msm_ip->write_register(0x34, msm_o->address() >> 32);

        printf("Initialized.\n");
    }
    virtual void *hw_ntt_i() {
        assert(initialized);
        return ntt_i->map<void *>();
    }
    virtual const void *hw_ntt_o() const {
        assert(initialized);
        return ntt_o->map<void *>();
    }
    virtual void hw_ntt(uint32_t log2N, bool is_inv) {
        assert(initialized);
        ntt_ip->write_register(0x18, log2N - 18);
        ntt_i->sync(XCL_BO_SYNC_BO_TO_DEVICE, sizeof(BigInt<254>) << log2N, 0);

        ntt_ip->write_register(0x10, (is_inv << 1) | 0x1);
        while (ntt_ip->read_register(0x10) & 0x1)
            ;

        ntt_o->sync(XCL_BO_SYNC_BO_FROM_DEVICE, sizeof(BigInt<254>) << log2N, 0);
    }
    virtual void *hw_msm_scalar_i() {
        assert(initialized);
        return msm_scalar_i->map<void *>();
    }
    virtual void *hw_msm_point_i() {
        assert(initialized);
        return msm_point_i->map<void *>();
    }
    virtual const void *hw_msm_o() const {
        assert(initialized);
        return msm_o->map<void *>();
    }
    virtual void hw_msm(uint64_t N) {
        assert(initialized);
        ShortWeierstrassPoint<254> *sum = msm_o->map<ShortWeierstrassPoint<254> *>();
        if (N == 0) {
            sum->X = 1u;
            sum->Y = 1u;
            sum->Z = 0u;
            return;
        }

        uint64_t groupNum = (N + 255) / 256;
        uint64_t really_N = groupNum * 256;
        for (uint64_t i = N; i < really_N; i++) {
            (msm_scalar_i->map<BigInt<254> *>())[i] = 0u;
        }

        msm_ip->write_register(0x18, really_N - 1);
        msm_scalar_i->sync(XCL_BO_SYNC_BO_TO_DEVICE, sizeof(BigInt<254>) * really_N, 0);
        msm_point_i->sync(XCL_BO_SYNC_BO_TO_DEVICE, sizeof(BigInt<254>) * 2 * really_N, 0);

        msm_ip->write_register(0x10, 0x1);
        while (msm_ip->read_register(0x10) & 0x1)
            ;

        msm_o->sync(XCL_BO_SYNC_BO_FROM_DEVICE, sizeof(BigInt<254>) * 3, 0);
        *sum = curve->pointMul(*sum, RInv);

        if (sum->Z == 0u) {
            sum->X = 1u;
            sum->Y = 1u;
            sum->Z = 0u;
            return;
        }

        BigInt<254> zInv = zp->mul(R, zp->inv(sum->Z));
        sum->X = zp->mul(sum->X, zInv);
        sum->Y = zp->mul(sum->Y, zInv);
        sum->Z = R;
    }
private:
    Zp<254> *zp;
    ShortWeierstrassCurve<254, 254, 0, 3> *curve;
    xrt::device *device;
    xrt::ip *ntt_ip;
    xrt::ip *msm_ip;

    xrt::bo *ntt_i;
    xrt::bo *ntt_o;
    xrt::bo *msm_scalar_i;
    xrt::bo *msm_point_i;
    xrt::bo *msm_o;

    bool initialized = false;
};

hw_interface *hw_get_instance() {
    static hw_device device;
    return &device;
}