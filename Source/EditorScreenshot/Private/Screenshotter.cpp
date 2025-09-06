// Copyright sirjofri. All rights reserved. See License file for more info.

#include "Screenshotter.h"

#include "Editor.h"
#include "FScreenshotPainter.h"
#include "ImageUtils.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Docking/TabManager.h"
#include "HAL/FileManager.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/FileHelper.h"
#include "Tests/AutomationEditorCommon.h"
#include "Tests/AutomationEditorPromotionCommon.h"
#include "Widgets/Docking/SDockTab.h"

void FScreenshotter::CaptureScreenshots(FString File)
{
	IFileManager& fmgr = IFileManager::Get();

	if (FPaths::IsRelative(File))
		File = FPaths::ProjectDir() + "/" + File;

	CurrentScreenshotFolder = FPaths::ProjectSavedDir() / TEXT("EditorScreenshots") / FPaths::GetBaseFilename(File);
	
	if (!fmgr.FileExists(*File)) {
		UE_LOG(LogTemp, Error, TEXT("File does not exist: %s"), *File);
		return;
	}

	Input.Empty();
	Input.Read(File);

	Sections.Empty();
	if (Input.GetKeys(Sections) <= 0) {
		UE_LOG(LogTemp, Log, TEXT("Input file is empty"));
		return;
	}

	NextSection = 0;
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
	if (!Sections.IsValidIndex(NextSection)) {
		Stage = None;
		return;
	}

	FString Section = Sections[NextSection];

	auto QueueNextSection = [&]()
	{
		NextSection++;
		Stage = PreCapture;
	};

	UE_LOG(LogTemp, Log, TEXT("Preparing %s"), *Section);

	FString SizeString;
	if (!Input.GetString(*Section, TEXT("Size"), SizeString)) {
		UE_LOG(LogTemp, Error, TEXT("Missing Size in section %s"), *Section);
		return;
	}
	TArray<FString> Parts;
	SizeString.ParseIntoArray(Parts, TEXT("x"));

	if (Parts.Num() != 2) {
		UE_LOG(LogTemp, Error, TEXT("Bad Size: %s in Section %s"), *SizeString, *Section);
		QueueNextSection();
		return;
	}

	FVector2D Size(FCString::Atoi(*Parts[0]), FCString::Atoi(*Parts[1]));

	TArray<FString> TabList;
	Section.ParseIntoArray(TabList, TEXT("/"));

	TSharedRef<FGlobalTabmanager> tmgr = FGlobalTabmanager::Get();
	TSharedPtr<SDockTab> relevanttab;

	TSharedPtr<FTabManager> TempTabManager = tmgr;
	for (FString ts : TabList) {
		TSharedPtr<SDockTab> tab = TempTabManager->TryInvokeTab(FName(ts));
		TempTabManager = tmgr->GetTabManagerForMajorTab(tab);
		relevanttab = tab;
	}
	FString Name = TabList.Last();
	{
		FString tn;
		if (Input.GetString(*Section, TEXT("Name"), tn))
			Name = tn;
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
	if (FScreenshotPainter::HasPainting(&Input, Section)) {
		Window->SetFullWindowOverlayContent(FScreenshotPainter::GetPainting(&Input, Section, Window));
		Window->BeginFullWindowOverlayTransition();
	}

	CurrentScreenshotData.Target = Name;
	CurrentScreenshotData.Widget = Window;
	CurrentScreenshotData.Window = Window;

	NextSection++;
	Stage = Capture;
}
