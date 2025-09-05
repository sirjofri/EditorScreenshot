// Copyright sirjofri. All rights reserved. See License file for more info.

#include "EditorScreenshot.h"

#include "Screenshotter.h"
#include "HAL/IConsoleManager.h"

DEFINE_LOG_CATEGORY(LogEditorScreenshot);

#define LOCTEXT_NAMESPACE "FEditorScreenshotModule"

void FEditorScreenshotModule::StartupModule()
{
	Screenshotter = MakeShared<FScreenshotter>();
	
	IConsoleManager::Get().RegisterConsoleCommand(TEXT("EditorScreenshot.Capture"), TEXT("Capture screenshots from a file"),
		FConsoleCommandWithArgsDelegate::CreateLambda([&](const TArray<FString>& Args)
		{
			if (Args.IsEmpty())
				return;

			Screenshotter->CaptureScreenshots(Args[0]);
		}));
}

void FEditorScreenshotModule::ShutdownModule()
{
	IConsoleManager::Get().UnregisterConsoleObject(TEXT("EditorScreenshot.Capture"));
	Screenshotter.Reset();
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FEditorScreenshotModule, EditorScreenshot)