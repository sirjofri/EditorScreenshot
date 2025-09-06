// Copyright sirjofri. All rights reserved. See License file for more info.

#pragma once

#include "CoreMinimal.h"
#include "TickableEditorObject.h"
#include "Misc/ConfigCacheIni.h"

class SWidget;
class SWindow;

class FScreenshotter : public TSharedFromThis<FScreenshotter>, public FTickableEditorObject
{
public:
	void CaptureScreenshots(FString File);
	bool CaptureInProgress();

	virtual TStatId GetStatId() const override { return TStatId(); }
	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override;
	virtual ETickableTickType GetTickableTickType() const override { return ETickableTickType::Conditional; };

private:
	void TakeScreenshot(FString Target, FString Folder, TSharedPtr<SWidget> InWidget);

	void TakeCurrentScreenshot();
	void CaptureNumber();

	enum TickStage
	{
		None, PreCapture, Capture
	};

	TickStage Stage = TickStage::None;

	FConfigFile Input;
	TArray<FString> Sections;
	int NextSection = 0;

	struct FCurrentScreenshotData
	{
		FString Target;
		TSharedPtr<SWidget> Widget;
		TSharedPtr<SWindow> Window;
	};

	FCurrentScreenshotData CurrentScreenshotData;
	FString CurrentScreenshotFolder;
};
