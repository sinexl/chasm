#ifndef REGISTER_H_
#define REGISTER_H_

#include "util.hpp"

enum class Reg : u8 {
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

#endif // REGISTER_H_
