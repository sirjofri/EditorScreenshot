// Copyright Epic Games, Inc. All Rights Reserved.

#include "EditorWindowScreenshot.h"

#include "Screenshotter.h"
#include "HAL/IConsoleManager.h"

#define LOCTEXT_NAMESPACE "FEditorWindowScreenshotModule"

void FEditorWindowScreenshotModule::StartupModule()
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

void FEditorWindowScreenshotModule::ShutdownModule()
{
	IConsoleManager::Get().UnregisterConsoleObject(TEXT("EditorScreenshot.Capture"));
	Screenshotter.Reset();
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FEditorWindowScreenshotModule, EditorWindowScreenshot)