// Copyright sirjofri. All rights reserved. See License file for more info.

#include "ScriptedEditorScreenshot.h"

#include "Screenshotter.h"
#include "HAL/IConsoleManager.h"

DEFINE_LOG_CATEGORY(LogScriptedEditorScreenshot);

#define LOCTEXT_NAMESPACE "ScriptedEditorScreenshot"

void FScriptedEditorScreenshotModule::StartupModule()
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

	IConsoleManager::Get().RegisterConsoleCommand(TEXT("ScriptedEditorScreenshot.Capture"),
		TEXT("Open File picker to capture file"),
		FConsoleCommandDelegate::CreateLambda([&]()
		{
			Screenshotter->CaptureFileDialog();
		}));
}

void FScriptedEditorScreenshotModule::ShutdownModule()
{
	IConsoleManager::Get().UnregisterConsoleObject(TEXT("ScriptedEditorScreenshot.Capture"));
	Screenshotter.Reset();
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FScriptedEditorScreenshotModule, ScriptedEditorScreenshot)