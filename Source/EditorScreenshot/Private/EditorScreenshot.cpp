// Copyright sirjofri. All rights reserved. See License file for more info.

#include "EditorScreenshot.h"

#include "Screenshotter.h"
#include "HAL/IConsoleManager.h"

DEFINE_LOG_CATEGORY(LogEditorScreenshot);

#define LOCTEXT_NAMESPACE "FEditorScreenshotModule"

void FEditorScreenshotModule::StartupModule()
{
	Screenshotter = MakeShared<FScreenshotter>();
	
	IConsoleManager::Get().RegisterConsoleCommand(TEXT("EditorScreenshot.CaptureFile"),
		TEXT("Capture screenshots from a file specified as an argument"),
		FConsoleCommandWithArgsDelegate::CreateLambda([&](const TArray<FString>& Args)
		{
			if (Args.IsEmpty())
				return;

			FString File = FString::Join(Args, TEXT(" "));
			Screenshotter->CaptureScreenshots(File);
		}));

	IConsoleManager::Get().RegisterConsoleCommand(TEXT("EditorScreenshot.Capture"),
		TEXT("Open File picker to capture file"),
		FConsoleCommandDelegate::CreateLambda([&]()
		{
			Screenshotter->CaptureFileDialog();
		}));
}

void FEditorScreenshotModule::ShutdownModule()
{
	IConsoleManager::Get().UnregisterConsoleObject(TEXT("EditorScreenshot.Capture"));
	Screenshotter.Reset();
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FEditorScreenshotModule, EditorScreenshot)