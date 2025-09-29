// Copyright sirjofri. All rights reserved. See License file for more info.

#pragma once

#include "CoreMinimal.h"

class SWidget;

class FScreenshotWidgetUtils
{
public:
	static TSharedPtr<SWidget> FindWidgetByPath(TSharedPtr<SWidget> Root, const TArray<FString>& Path);
	static TSharedPtr<SWidget> FindWidgetDepth(TSharedPtr<SWidget> Root, const FString& Type);
};
