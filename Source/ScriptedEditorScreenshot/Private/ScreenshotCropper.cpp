// Copyright sirjofri. All rights reserved. See License file for more info.

#include "ScreenshotCropper.h"

#include "ScriptedEditorScreenshot.h"
#include "ScreenshotWidgetUtils.h"
#include "Misc/ConfigCacheIni.h"
#include "Widgets/SWindow.h"
#include "Widgets/Images/SImage.h"

void FScreenshotCropper::GetCropData(FConfigFile* Input, FString Section, TSharedPtr<SWindow> Window,
	TSharedPtr<SWidget>& CropWidget, FMargin& CropMargin)
{
	FString CropTarget;
	Input->GetString(*Section, TEXT("Crop"), CropTarget);

	if (CropTarget.IsEmpty())
		return;

	TArray<FString> Path;
	CropTarget.ParseIntoArray(Path, TEXT("."));

	TSharedPtr<SWidget> TargetWidget = FScreenshotWidgetUtils::FindWidgetByPath(Window->GetContent(), Path);
	if (!TargetWidget.IsValid()) {
		UE_LOG(LogScriptedEditorScreenshot, Error, TEXT("Widget %s not found for cropping"), *CropTarget);
		return;
	}

	CropWidget = TargetWidget;

	FString MarginString;
	Input->GetString(*Section, TEXT("CropMargin"), MarginString);
	TArray<FString> MarginParts;
	MarginString.ParseIntoArrayWS(MarginParts);

	FMargin Margin;

	switch (MarginParts.Num()) {
	case 1:
		Margin = FMargin(FCString::Atoi(*MarginParts[0]));
		break;
	case 2:
		Margin = FMargin(FCString::Atoi(*MarginParts[0]), FCString::Atoi(*MarginParts[1]));
		break;
	case 4:
		Margin = FMargin(FCString::Atoi(*MarginParts[0]), FCString::Atoi(*MarginParts[1]), FCString::Atoi(*MarginParts[2]), FCString::Atoi(*MarginParts[3]));
		break;
	default:
		UE_LOG(LogScriptedEditorScreenshot, Error, TEXT("Bad CropMargin: %s"), *MarginString);
		Margin = FMargin(0);
	}

	CropMargin = Margin;
}
