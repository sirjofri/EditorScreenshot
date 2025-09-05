#include "FScreenshotPainter.h"

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
		
		int nth = 0;
		if (Path.Last().IsNumeric()) {
			nth = FCString::Atoi(*Path.Last());
			Path.RemoveAt(Path.Num() - 1);
		}
		TSharedPtr<SWidget> Widget = FindWidgetByPath(Window->GetContent(), Path, nth);
		if (!Widget.IsValid()) {
			UE_LOG(LogTemp, Warning, TEXT("Widget not found: %s"), *h);
			continue;
		}
		HighlightWidgets.Add(Widget);
	}
	if (HighlightWidgets.IsEmpty())
		return nullptr;
	return SNew(SScreenshotPainterWidget).HighlightWidgets(HighlightWidgets);
}

TSharedPtr<SWidget> FScreenshotPainter::FindWidgetByPath(TSharedPtr<SWidget> Root, const TArray<FString>& Path, int Nth)
{
	FChildren* children = Root->GetChildren();
	TSharedPtr<SWidget> child = nullptr;

	for (int j = 0; j < Path.Num(); j++) {
		FString p = Path[j];
		for (int i = 0; i < children->Num(); i++) {
			child = children->GetChildAt(i);
			if (p.Equals(child->GetTypeAsString())) {
				if (j == Path.Num() - 1) {
					Nth--;
					if (Nth < 0)
						return child;
					continue;
				}
				children = child->GetChildren();
				break;
			}
		}
	}
	return nullptr;
}
