#pragma once

#include "CoreMinimal.h"
#include "TickableEditorObject.h"

class SWidget;
class SWindow;

class FScreenshotter : public TSharedFromThis<FScreenshotter>, public FTickableEditorObject
{
public:
	void CaptureScreenshots(FString File);

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

	TArray<FString> Lines;
	int NextLine = 0;

	struct FCurrentScreenshotData
	{
		FString Target;
		TSharedPtr<SWidget> Widget;
		TSharedPtr<SWindow> Window;
	};

	FCurrentScreenshotData CurrentScreenshotData;
	FString CurrentScreenshotFolder;
};
