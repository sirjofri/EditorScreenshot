// Copyright sirjofri. All rights reserved. See License file for more info.

#pragma once

#include "CoreMinimal.h"
#include "Misc/ConfigCacheIni.h"
#include "Widgets/SLeafWidget.h"

class SWidget;
class SWindow;

class SScreenshotPainterWidget : public SLeafWidget
{
public:
	SLATE_BEGIN_ARGS(SScreenshotPainterWidget) {}
	SLATE_ARGUMENT(TArray<TSharedPtr<SWidget>>, HighlightWidgets)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	virtual FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const override;
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

private:
	TArray<TSharedPtr<SWidget>> HighlightWidgets;
};

class FScreenshotPainter
{
public:
	static bool HasPainting(FConfigFile* Input, FString Section);
	static TSharedPtr<SWidget> GetPainting(FConfigFile* Input, FString Section, TSharedPtr<SWindow> Window);

private:
	static TSharedPtr<SWidget> FindWidgetByPath(TSharedPtr<SWidget> Root, const TArray<FString>& Path);
	static TSharedPtr<SWidget> FindWidgetDepth(TSharedPtr<SWidget> Root, const FString& Type);
};
