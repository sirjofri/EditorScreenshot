#include "Screenshotter.h"

#include "Editor.h"
#include "ImageUtils.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Docking/TabManager.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Tests/AutomationEditorCommon.h"
#include "Tests/AutomationEditorPromotionCommon.h"
#include "Widgets/Docking/SDockTab.h"

void FScreenshotter::CaptureScreenshots(FString File)
{
	IFileManager& fmgr = IFileManager::Get();

	if (FPaths::IsRelative(File))
		File = FPaths::ProjectDir() + "/" + File;

	CurrentScreenshotFolder = FPaths::ProjectSavedDir() / TEXT("EditorScreenshots") / FPaths::GetBaseFilename(FPaths::GetPath(File));
	
	if (!fmgr.FileExists(*File)) {
		UE_LOG(LogTemp, Error, TEXT("File does not exist: %s"), *File);
		return;
	}

	Lines.Empty();
	if (!FFileHelper::LoadFileToStringArray(Lines, *File)) {
		UE_LOG(LogTemp, Error, TEXT("Cannot read file: %s"), *File);
		return;
	}

	NextLine = 0;
	Stage = PreCapture;
}

void FScreenshotter::Tick(float DeltaTime)
{
	switch (Stage) {
	case PreCapture:
		CaptureNumber();
		break;
	case Capture:
		TakeCurrentScreenshot();
		break;
	}
}

bool FScreenshotter::IsTickable() const
{
	return Stage == PreCapture || Stage == Capture;
}

void FScreenshotter::TakeScreenshot(FString Target, FString Folder, TSharedPtr<SWidget> InWidget)
{
	if (!FSlateApplication::IsInitialized()) {
		UE_LOG(LogTemp, Error, TEXT("Slate is not initialized"));
		return;
	}
	UE_LOG(LogTemp, Log, TEXT("Taking screenshot %s"), *Target);
	TArray<FColor> OutImageData;
	FIntVector OutImageSize;
	if (FSlateApplication::Get().TakeScreenshot(InWidget.ToSharedRef(), OutImageData, OutImageSize)) {
		FString FileName;
		const FString BaseFileName = Folder / Target + TEXT(".png");
		FImageView ImageView(OutImageData.GetData(), OutImageSize.X, OutImageSize.Y);
		FImageUtils::SaveImageByExtension(*BaseFileName, ImageView);
	}
}

void FScreenshotter::TakeCurrentScreenshot()
{
	TakeScreenshot(CurrentScreenshotData.Target, CurrentScreenshotFolder, CurrentScreenshotData.Widget);
	CurrentScreenshotData.Window->RequestDestroyWindow();
	Stage = PreCapture;
}

void FScreenshotter::CaptureNumber()
{
	if (!Lines.IsValidIndex(NextLine)) {
		Stage = None;
		return;
	}

	FString Line = Lines[NextLine];

	auto QueueNextLine = [&]()
	{
		NextLine++;
		Stage = PreCapture;
	};

	UE_LOG(LogTemp, Log, TEXT("Screenshot %s"), *Line);

	if (Line.StartsWith("#")) {
		QueueNextLine();
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Preparing %s"), *Line);

	TArray<FString> Parts;
	Line.ParseIntoArrayWS(Parts);

	if (Parts.Num() < 3) {
		UE_LOG(LogTemp, Error, TEXT("Bad line: %s"), *Line);
		QueueNextLine();
		return;
	}

	FVector2D Size(FCString::Atoi(*Parts[1]), FCString::Atoi(*Parts[2]));

	TArray<FString> TabList;
	Parts[0].ParseIntoArray(TabList, TEXT("/"));

	TSharedRef<FGlobalTabmanager> tmgr = FGlobalTabmanager::Get();
	TSharedPtr<SDockTab> relevanttab;

	TSharedPtr<FTabManager> TempTabManager = tmgr;
	for (FString ts : TabList) {
		TSharedPtr<SDockTab> tab = TempTabManager->TryInvokeTab(FName(ts));
		TempTabManager = tmgr->GetTabManagerForMajorTab(tab);
		relevanttab = tab;
	}

	TSharedRef<SWindow> Window = SNew(SWindow)
		.ClientSize(Size);
	FSlateApplication::Get().AddWindow(Window);
	FOptionalSize Title = Window->GetTitleBarSize();
	FMargin borders = Window->GetWindowBorderSize(false);
	Size.X -= borders.Left + borders.Right;
	Size.Y -= borders.Top + borders.Bottom + (Title.IsSet() ? Title.Get() : 0);
	Window->Resize(Size);
	Window->SetContent(relevanttab->GetContent());

	CurrentScreenshotData.Target = TabList.Last();
	CurrentScreenshotData.Widget = Window;
	CurrentScreenshotData.Window = Window;

	NextLine++;
	Stage = Capture;
}
