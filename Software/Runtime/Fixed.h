//
// Runtime/Fixed.h
//
// This file is part of Portatil source code.
// Copyright 2025 Patrick L. Melo <patrick@patrickmelo.com.br>
//

#ifndef PORTATIL_FIXED_H
#define PORTATIL_FIXED_H

#ifndef _DEFAULT_SOURCE
    #define _DEFAULT_SOURCE
#endif

#ifndef _POSIX_C_SOURCE
    #define _POSIX_C_SOURCE 200809L
#endif

#include <stdint.h>

typedef int32_t f16;

#define F16One     0x00010000
#define F16Maximum 0x7FFFFFFF
#define F16Minimum 0x80000000

#define F16(constValue)  F16FromInt(constValue)
#define F16F(constValue) F16FromFloat(constValue)

#define F16FromInt(intValue)       ((f16) ((intValue) << 16))
#define F16FromFloat(floatValue)   ((f16) ((floatValue) * F16One))
#define F16FromDouble(doubleValue) ((f16) ((doubleValue) * F16One))

#define F16ToInt(f16Value)    ((int32_t) (f16Value >> 16))
#define F16ToFloat(f16Value)  ((float) f16Value / F16One)
#define F16ToDouble(f16Value) ((double) f16Value / F16One)

static inline f16 F16Abs(f16 fixedValue) {
    return (f16) (fixedValue < 0 ? -(uint32_t) fixedValue : (uint32_t) fixedValue);
}

static inline f16 F16Floor(f16 fixedValue) {
    return (fixedValue & 0xFFFF0000);
}

static inline f16 F16Ceil(f16 fixedValue) {
    return (fixedValue & 0xFFFF0000) + (fixedValue & 0x0000FFFF ? F16One : 0);
}

static inline f16 F16Min(f16 aValue, f16 bValue) {
    return (aValue < bValue ? aValue : bValue);
}

static inline f16 F16Max(f16 aValue, f16 bValue) {
    return (aValue > bValue ? aValue : bValue);
}

static inline f16 F16Clamp(f16 fixedValue, f16 minValue, f16 maxValue) {
    return F16Min(F16Max(fixedValue, minValue), maxValue);
}

static inline f16 F16Mult(const f16 aValue, const f16 bValue) {
    int64_t f16Product = (int64_t) aValue * (int64_t) bValue;
    return (f16) (f16Product >> 16);
}

static inline f16 F16Div(const f16 aValue, const f16 bValue) {
    int64_t aExtended = aValue >> 31 ? 0xFFFFFFFFFFFFFFFF : 0x0000000000000000;
    aExtended |= ((int64_t) aValue & 0x7FFFFFFF) << 16;
    return (f16) (aExtended / (int64_t) bValue);
}

static inline f16 F16Mod(const f16 aValue, const f16 bValue) {
    return aValue % bValue;
}

#endif    // PORTATIL_FIXED_H