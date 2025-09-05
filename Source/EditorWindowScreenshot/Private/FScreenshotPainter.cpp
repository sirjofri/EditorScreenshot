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
		
		TSharedPtr<SWidget> Widget = FindWidgetByPath(Window->GetContent(), Path);
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

TSharedPtr<SWidget> FScreenshotPainter::FindWidgetByPath(TSharedPtr<SWidget> Root, const TArray<FString>& Path)
{
	FChildren* children = Root->GetChildren();
	TSharedPtr<SWidget> child = Root;

	auto findnth = [&](FChildren* cn, FString t, int n) -> TSharedPtr<SWidget>
	{
		for (int i = 0; i < cn->Num(); i++) {
			child = cn->GetChildAt(i);
			if (t.Equals(child->GetTypeAsString())) {
				n--;
				if (n < 0)
					return child;
			}
		}
		return nullptr;
	};

	for (int j = 0; j < Path.Num(); j++) {
		FString p = Path[j];
		if (Path.IsValidIndex(j+1)) {
			FString pt = Path[j+1];
			if (pt.IsNumeric()) {
				int n = FCString::Atoi(*pt);
				child = findnth(children, p, n);
				if (child.IsValid()) {
					if (j+1 == Path.Num() - 1)
						return child;
					children = child->GetChildren();
					j++;
					continue;
				}
				UE_LOG(LogTemp, Warning, TEXT("Widget not found: %s"), *p);
				return nullptr;
			}
		}
		if (p.StartsWith(TEXT("[")) && p.EndsWith(TEXT("]"))) {
			child = FindWidgetDepth(child, p.Mid(1, p.Len() - 2));
			if (!child.IsValid()) {
				UE_LOG(LogTemp, Warning, TEXT("Widget not found with depth search: %s"), *p);
				return nullptr;
			}
			if (j == Path.Num() - 1)
				return child;
			children = child->GetChildren();
			continue;
		}
		child = findnth(children, p, 0);
		if (!child.IsValid()) {
			UE_LOG(LogTemp, Warning, TEXT("Widget not Found: %s"), *p);
			return nullptr;
		}
		if (j == Path.Num() - 1)
			return child;
		children = child->GetChildren();
	}
	return nullptr;
}

TSharedPtr<SWidget> FScreenshotPainter::FindWidgetDepth(TSharedPtr<SWidget> Root, const FString& Type)
{
	if (!Root.IsValid())
		return nullptr;
	if (Root->GetTypeAsString().Equals(Type))
		return Root;
	
	FChildren* children = Root->GetChildren();

	for (int i = 0; i < children->Num(); i++) {
		TSharedPtr<SWidget> child = children->GetChildAt(i);
		TSharedPtr<SWidget> found = FindWidgetDepth(child, Type);
		if (found.IsValid())
			return found;
	}
	return nullptr;
}
