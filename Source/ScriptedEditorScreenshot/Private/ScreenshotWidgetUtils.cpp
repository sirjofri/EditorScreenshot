// Copyright sirjofri. All rights reserved. See License file for more info.

#include "ScreenshotWidgetUtils.h"

#include "ScriptedEditorScreenshot.h"
#include "Layout/ChildrenBase.h"
#include "Widgets/SWidget.h"

TSharedPtr<SWidget> FScreenshotWidgetUtils::FindWidgetByPath(TSharedPtr<SWidget> Root, const TArray<FString>& Path)
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
				UE_LOG(LogScriptedEditorScreenshot, Warning, TEXT("Widget not found: %s"), *p);
				return nullptr;
			}
		}
		if (p.StartsWith(TEXT("[")) && p.EndsWith(TEXT("]"))) {
			child = FindWidgetDepth(child, p.Mid(1, p.Len() - 2));
			if (!child.IsValid()) {
				UE_LOG(LogScriptedEditorScreenshot, Warning, TEXT("Widget not found with depth search: %s"), *p);
				return nullptr;
			}
			if (j == Path.Num() - 1)
				return child;
			children = child->GetChildren();
			continue;
		}
		child = findnth(children, p, 0);
		if (!child.IsValid()) {
			UE_LOG(LogScriptedEditorScreenshot, Warning, TEXT("Widget not Found: %s"), *p);
			return nullptr;
		}
		if (j == Path.Num() - 1)
			return child;
		children = child->GetChildren();
	}
	return nullptr;
}

TSharedPtr<SWidget> FScreenshotWidgetUtils::FindWidgetDepth(TSharedPtr<SWidget> Root, const FString& Type)
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