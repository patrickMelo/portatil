//
// Runtime/States/Error.c
//
// This file is part of Portatil source code.
// Copyright 2025 Patrick L. Melo <patrick@patrickmelo.com.br>
//

#include "../States.h"

static string         errorMessage             = NULL;
static Rectangle2D    backgroundRectangle      = {.X = 0, .Y = 0, .Width = ScreenWidth, .Height = 0};
static u8             backgroundRectangleColor = 0;
static Rectangle2D    rectangle                = {.X = 0, .Y = 0, .Width = ScreenWidth, .Height = 0};
static u8             rectangleColor           = 0;
static KernelFunction nextState                = NULL;
static BitmapFont*    defaultFont              = NULL;

void InitializeError(void) {
    defaultFont      = GetDefaultFont();
    rectangleColor   = GetNearestColorIndex(220, 0, 0);
    rectangle.Height = defaultFont->CharHeight * 5;
    rectangle.Y      = (ScreenHeight - rectangle.Height) / 2;

    backgroundRectangleColor   = GetNearestColorIndex(255, 255, 255);
    backgroundRectangle.Y      = rectangle.Y - 2;
    backgroundRectangle.Height = rectangle.Height + 4;
}

void ShowError(KernelFunction nextStateFunction, const string message) {
    errorMessage = message;
    nextState    = nextStateFunction;

    ChangeState(ErrorState);
}

void ErrorState(const u64 frameTime) {
    if (IsButtonJustPressed(ButtonY) && (nextState)) {
        ChangeState(nextState);
        return;
    }

    ResetDrawState();
    DrawRectangle(&backgroundRectangle, backgroundRectangleColor);
    DrawRectangle(&rectangle, rectangleColor);

    SetDrawAnchor(AnchorMiddle | AnchorCenter);
    SetTransparentColor(0);

    DrawText(defaultFont, ScreenWidth / 2, ScreenHeight / 2 - defaultFont->CharHeight, errorMessage);
    DrawText(defaultFont, ScreenWidth / 2, (ScreenHeight / 2) + defaultFont->CharHeight, "Press Y to Continue");
}