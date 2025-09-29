// Copyright sirjofri. All rights reserved. See License file for more info.

#pragma once

#include "CoreMinimal.h"
#include "Layout/Margin.h"

class SWidget;
class SWindow;
class SCanvas;

class FScreenshotCropper
{
public:
	static void GetCropData(FConfigFile* Input, FString Section, TSharedPtr<SWindow> Window, TSharedPtr<SWidget>& CropWidget, FMargin &CropMargin);
};
