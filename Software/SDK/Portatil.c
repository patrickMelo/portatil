//
// SDK/Portatil.c
//
// This file is part of Portatil source code.
// Copyright 2025 Patrick L. Melo <patrick@patrickmelo.com.br>
//

#include "Portatil.h"

extern bool AppSetup(void);
extern void AppSync(const f16 speedMultiplier);

void main(void) {
    if (!AppSetup()) {
        Exit(1);
    }

    f16 speedMultiplier = Sync();

    for (;;) {
        AppSync(speedMultiplier);
        speedMultiplier = Sync();
    }
}
