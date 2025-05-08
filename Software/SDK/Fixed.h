//
// SDK/Fixed.h
//
// This file is part of Portatil source code.
// Copyright 2025 Patrick L. Melo <patrick@patrickmelo.com.br>
//

#ifndef PORTATIL_FIXED_H
#define PORTATIL_FIXED_H

typedef int f16;

#define F16Half    0x00008000
#define F16One     0x00010000
#define F16Maximum 0x7FFFFFFF
#define F16Minimum 0x80000000

#define F16(intValue)     ((f16) ((intValue) << 16))
#define FromF16(f16Value) ((int) (f16Value & 0x80000000) | (f16Value >> 16))

static inline f16 F16Abs(f16 fixedValue) {
    return (f16) (fixedValue < 0 ? -(int) fixedValue : (int) fixedValue);
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
    long long int f16Product = (long long int) aValue * (long long int) bValue;
    return (f16) (f16Product >> 16);
}

static inline f16 F16Div(const f16 aValue, const f16 bValue) {
    long long int aExtended = aValue >> 31 ? 0xFFFFFFFFFFFFFFFF : 0x0000000000000000;
    aExtended |= ((long long int) aValue & 0x7FFFFFFF) << 16;
    return (f16) (aExtended / (long long int) bValue);
}

static inline f16 F16Mod(const f16 aValue, const f16 bValue) {
    return aValue % bValue;
}

#endif    // PORTATIL_FIXED_H