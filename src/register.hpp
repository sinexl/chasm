#ifndef CHASM_REGISTER_
#define CHASM_REGISTER_

#include <unordered_map>

#include "util.hpp"

namespace reg
{
    enum class Reg : u8
    {
        zero = 0,
        at = 1,
        // function return (2 - 3)
        v0, v1,
        // arguments (4-7)
        a0, a1, a2, a3,
        // temporaries (8-15)
        t0, t1, t2, t3, t4, t5, t6, t7,
        // saved temporaries (16-23)
        s0, s1, s2, s3, s4, s5, s6, s7,
        // more temporaries (24-25),
        t8, t9,
        // OS reserved
        k0, k1,
        // global pointer
        gp,
        // stack pointer
        sp,
        // frame pointer
        fp,
        // return address
        ra
    };

    static const std::unordered_map<std::string_view, Reg> strings = {
        {"$zero", Reg::zero}, {"$0", Reg::zero},
        {"$at", Reg::at}, {"$1", Reg::at},

        {"$v0", Reg::v0}, {"$2", Reg::v0},
        {"$v1", Reg::v1}, {"$3", Reg::v1},

        {"$a0", Reg::a0}, {"$4", Reg::a0},
        {"$a1", Reg::a1}, {"$5", Reg::a1},
        {"$a2", Reg::a2}, {"$6", Reg::a2},
        {"$a3", Reg::a3}, {"$7", Reg::a3},

        {"$t0", Reg::t0}, {"$8", Reg::t0},
        {"$t1", Reg::t1}, {"$9", Reg::t1},
        {"$t2", Reg::t2}, {"$10", Reg::t2},
        {"$t3", Reg::t3}, {"$11", Reg::t3},
        {"$t4", Reg::t4}, {"$12", Reg::t4},
        {"$t5", Reg::t5}, {"$13", Reg::t5},
        {"$t6", Reg::t6}, {"$14", Reg::t6},
        {"$t7", Reg::t7}, {"$15", Reg::t7},

        {"$s0", Reg::s0}, {"$16", Reg::s0},
        {"$s1", Reg::s1}, {"$17", Reg::s1},
        {"$s2", Reg::s2}, {"$18", Reg::s2},
        {"$s3", Reg::s3}, {"$19", Reg::s3},
        {"$s4", Reg::s4}, {"$20", Reg::s4},
        {"$s5", Reg::s5}, {"$21", Reg::s5},
        {"$s6", Reg::s6}, {"$22", Reg::s6},
        {"$s7", Reg::s7}, {"$23", Reg::s7},

        {"$t8", Reg::t8}, {"$24", Reg::t8},
        {"$t9", Reg::t9}, {"$25", Reg::t9},

        {"$k0", Reg::k0}, {"$26", Reg::k0},
        {"$k1", Reg::k1}, {"$27", Reg::k1},

        {"$gp", Reg::gp}, {"$28", Reg::gp},
        {"$sp", Reg::sp}, {"$29", Reg::sp},
        {"$fp", Reg::fp}, {"$30", Reg::fp},
        {"$ra", Reg::ra}, {"$31", Reg::ra}
    };

    inline u8 reg_u8(Reg r)
    {
        return static_cast<u8>(r);
    }

}

#endif
