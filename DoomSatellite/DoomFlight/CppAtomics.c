// ======================================================================
// \title  CppAtomics.c
// \brief  GCC __atomic libcalls for Cortex-M0+ (RP2040), which lacks LDREX/STREX
//
// libstdc++ std::atomic lowers to these out-of-line calls on ARMv6-M. Zephyr provides
// only a subset, so the remainder are implemented here with an IRQ lock, which is
// sufficient on a single-core Zephyr build.
// ======================================================================
#include <stdint.h>
#include <zephyr/kernel.h>

#if defined(CONFIG_ATOMIC_OPERATIONS_C)

#define DEFINE_ATOMIC_EXCHANGE(n, type)                                        \
    type __atomic_exchange_##n(volatile void* ptr, type value, int memorder) { \
        (void)memorder;                                                        \
        unsigned int key = irq_lock();                                         \
        volatile type* p = ptr;                                                \
        type previous = *p;                                                    \
        *p = value;                                                            \
        irq_unlock(key);                                                       \
        return previous;                                                       \
    }

#define DEFINE_ATOMIC_FETCH_OP(name, op, n, type)                                    \
    type __atomic_fetch_##name##_##n(volatile void* ptr, type value, int memorder) { \
        (void)memorder;                                                              \
        unsigned int key = irq_lock();                                               \
        volatile type* p = ptr;                                                      \
        type previous = *p;                                                          \
        *p = previous op value;                                                      \
        irq_unlock(key);                                                             \
        return previous;                                                             \
    }

#define DEFINE_ATOMIC_OPERATIONS(n, type)   \
    DEFINE_ATOMIC_EXCHANGE(n, type)         \
    DEFINE_ATOMIC_FETCH_OP(add, +, n, type) \
    DEFINE_ATOMIC_FETCH_OP(sub, -, n, type)

DEFINE_ATOMIC_OPERATIONS(1, uint8_t)
DEFINE_ATOMIC_OPERATIONS(2, uint16_t)
DEFINE_ATOMIC_OPERATIONS(4, uint32_t)

#endif
