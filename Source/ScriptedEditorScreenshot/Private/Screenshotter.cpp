// Copyright sirjofri. All rights reserved. See License file for more info.

#include "Screenshotter.h"

#include "DesktopPlatformModule.h"
#include "Editor.h"
#include "ScriptedEditorScreenshot.h"
#include "IDesktopPlatform.h"
#include "ScreenshotPainter.h"
#include "ImageUtils.h"
#include "ScreenshotCropper.h"
#include "Components/Viewport.h"
#include "Engine/GameViewportClient.h"
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
		UE_LOG(LogScriptedEditorScreenshot, Error, TEXT("File does not exist: %s"), *File);
		return;
	}

	Input.Empty();
	Input.Read(File);

	Sections.Empty();
	if (Input.GetKeys(Sections) <= 0) {
		UE_LOG(LogScriptedEditorScreenshot, Log, TEXT("Input file is empty"));
		return;
	}

	NextSection = 0;
	Stage = PreCapture;
}

void FScreenshotter::CaptureFileDialog()
{
	if (!FSlateApplication::IsInitialized())
		return;
	FSlateApplication& SlateApp = FSlateApplication::Get();
	if (!SlateApp.GetActiveTopLevelWindow())
		return;
	void* WindowHandle = SlateApp.GetActiveTopLevelWindow()->GetNativeWindow()->GetOSWindowHandle();
	if (!WindowHandle)
		return;
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	if (!DesktopPlatform)
		return;

	TArray<FString> Files;
	bool success = DesktopPlatform->OpenFileDialog(WindowHandle, TEXT("Open screenshot description file"),
		FPaths::ProjectDir(), TEXT(""), TEXT("INI Files|*.ini|All Files|*"), EFileDialogFlags::None, Files);

	if (!success || Files.IsEmpty()) {
		UE_LOG(LogScriptedEditorScreenshot, Log, TEXT("Cancelled"));
		return;
	}

	CaptureScreenshots(Files[0]);
}

bool FScreenshotter::CaptureInProgress()
{
	return Stage != None;
}

void FScreenshotter::Tick(float DeltaTime)
{
	switch (Stage) {
	case PreCapture:
		CaptureNumber();
		break;
	case Warmup:
		WarmupFrames--;
		if (WarmupFrames <= 0)
			Stage = Capture;
		break;
	case Capture:
		TakeCurrentScreenshot();
		break;
	}
}

bool FScreenshotter::IsTickable() const
{
	return Stage != None;
}

void FScreenshotter::TakeScreenshot(FString Target, FString Folder, TSharedPtr<SWidget> InWidget, FIntRect CropRect)
{
	if (!FSlateApplication::IsInitialized()) {
		UE_LOG(LogScriptedEditorScreenshot, Error, TEXT("Slate is not initialized"));
		return;
	}
	UE_LOG(LogScriptedEditorScreenshot, Log, TEXT("Taking screenshot %s"), *Target);
	TArray<FColor> OutImageData;
	FIntVector OutImageSize;
	if (FSlateApplication::Get().TakeScreenshot(InWidget.ToSharedRef(), CropRect, OutImageData, OutImageSize)) {
		FString FileName;
		const FString BaseFileName = Folder / Target + TEXT(".png");
		FImageView ImageView(OutImageData.GetData(), OutImageSize.X, OutImageSize.Y);
		FImageUtils::SaveImageByExtension(*BaseFileName, ImageView);
	}
}

void FScreenshotter::TakeCurrentScreenshot()
{
	FIntRect CropRect;
	if (CurrentScreenshotData.CropWidget.IsValid()) {
		FSlateRect Rect = CurrentScreenshotData.CropWidget->GetPaintSpaceGeometry().GetLayoutBoundingRect();
		CropRect = FIntRect(
			Rect.Left - CurrentScreenshotData.CropMargin.Left,
			Rect.Top - CurrentScreenshotData.CropMargin.Top,
			Rect.Right + CurrentScreenshotData.CropMargin.Right,
			Rect.Bottom + CurrentScreenshotData.CropMargin.Bottom);
	}
	TakeScreenshot(CurrentScreenshotData.Target, CurrentScreenshotFolder, CurrentScreenshotData.Widget, CropRect);
	CurrentScreenshotData.Window->RequestDestroyWindow();

	for (int i = TabsToClose.Num() - 1; i >= 0; i--) {
		TSharedPtr<SDockTab> tab = TabsToClose[i];
		if (tab.IsValid())
			tab->RequestCloseTab();
	}
	TabsToClose.Empty();
	
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

	UE_LOG(LogScriptedEditorScreenshot, Log, TEXT("Preparing %s"), *Section);

	FString SizeString;
	if (!Input.GetString(*Section, TEXT("Size"), SizeString)) {
		UE_LOG(LogScriptedEditorScreenshot, Error, TEXT("Missing Size in section %s"), *Section);
		return;
	}
	TArray<FString> Parts;
	SizeString.ParseIntoArray(Parts, TEXT("x"));

	if (Parts.Num() != 2) {
		UE_LOG(LogScriptedEditorScreenshot, Error, TEXT("Bad Size: %s in Section %s"), *SizeString, *Section);
		QueueNextSection();
		return;
	}

	FVector2D Size(FCString::Atoi(*Parts[0]), FCString::Atoi(*Parts[1]));

	FString TabPath;
	if (!Input.GetString(*Section, TEXT("Tab"), TabPath)) {
		UE_LOG(LogScriptedEditorScreenshot, Error, TEXT("Missing Tab in section %s"), *Section);
		QueueNextSection();
		return;
	}

	TArray<FString> TabList;
	TabPath.ParseIntoArray(TabList, TEXT("/"));

	TSharedRef<FGlobalTabmanager> tmgr = FGlobalTabmanager::Get();
	TSharedPtr<SDockTab> relevanttab;

	TabsToClose.Empty();
	TSharedPtr<FTabManager> TempTabManager = tmgr;
	for (FString ts : TabList) {
		FName tn = FName(ts);
		TSharedPtr<SDockTab> tab = TempTabManager->FindExistingLiveTab(tn);
		if (!tab.IsValid()) {
			tab = TempTabManager->TryInvokeTab(tn);
			TabsToClose.Add(tab);
		}
		if (!tab.IsValid()) {
			UE_LOG(LogScriptedEditorScreenshot, Error, TEXT("Tab not found: %s"), *ts);
			QueueNextSection();
			return;
		}
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
	Window->SetTitle(relevanttab->GetTabLabel());

	if (FScreenshotPainter::HasPainting(&Input, Section)) {
		Window->SetFullWindowOverlayContent(FScreenshotPainter::GetPainting(&Input, Section, Window));
		Window->BeginFullWindowOverlayTransition();
	}

	CurrentScreenshotData.Target = Section;
	CurrentScreenshotData.Widget = Window;
	CurrentScreenshotData.Window = Window;
	CurrentScreenshotData.CropWidget = nullptr;

	FScreenshotCropper::GetCropData(&Input, Section, Window, CurrentScreenshotData.CropWidget, CurrentScreenshotData.CropMargin);

	NextSection++;

	WarmupFrames = 0;
	Input.GetInt(*Section, TEXT("Warmup"), WarmupFrames);
	
	Stage = WarmupFrames > 0 ? Warmup : Capture;
}
