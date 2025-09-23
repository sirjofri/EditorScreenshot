// Copyright sirjofri. All rights reserved. See License file for more info.

#include "ScreenshotPainter.h"

#include "EditorScreenshot.h"
#include "ScreenshotWidgetUtils.h"
#include "Widgets/SWindow.h"

void SScreenshotPainterWidget::Construct(const FArguments& InArgs)
{
	HighlightWidgets = InArgs._HighlightWidgets;
}

FVector2D SScreenshotPainterWidget::ComputeDesiredSize(float LayoutScaleMultiplier) const
{
	return GetParentWidget()->GetDesiredSize();
}

int32 SScreenshotPainterWidget::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
                                        const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId,
                                        const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	const FSlateBrush* brush = FCoreStyle::Get().GetBrush(TEXT("PlainBorder"));
	LayerId++;
	for (TSharedPtr<SWidget> Widget : HighlightWidgets) {
		FGeometry Geo = Widget->GetPaintSpaceGeometry();
		FPaintGeometry PaintGeo(Geo.GetAccumulatedLayoutTransform(), Geo.GetAccumulatedRenderTransform(), Geo.GetLocalSize(), Geo.HasRenderTransform());
		FSlateDrawElement::MakeBox(OutDrawElements, LayerId, PaintGeo, brush, ESlateDrawEffect::None, FLinearColor::Red);
	}
	return LayerId;
}

bool FScreenshotPainter::HasPainting(FConfigFile* Input, FString Section)
{
	return Input->FindSection(Section)->Contains(TEXT("Highlight"));
}

TSharedPtr<SWidget> FScreenshotPainter::GetPainting(FConfigFile* Input, FString Section, TSharedPtr<SWindow> Window)
{
	TArray<FString> Highlights;
	Input->GetArray(*Section, TEXT("Highlight"), Highlights);

	TArray<TSharedPtr<SWidget>> HighlightWidgets;
	HighlightWidgets.Reserve(Highlights.Num());

	for (FString& h : Highlights) {
		TArray<FString> Path;
		h.ParseIntoArray(Path, TEXT("."));
		
		TSharedPtr<SWidget> Widget = FScreenshotWidgetUtils::FindWidgetByPath(Window->GetContent(), Path);
		if (!Widget.IsValid()) {
			UE_LOG(LogEditorScreenshot, Warning, TEXT("Widget not found: %s"), *h);
			continue;
		}
		HighlightWidgets.Add(Widget);
	}
	if (HighlightWidgets.IsEmpty())
		return nullptr;
	return SNew(SScreenshotPainterWidget).HighlightWidgets(HighlightWidgets);
}
