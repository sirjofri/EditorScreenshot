// Copyright sirjofri. All rights reserved. See License file for more info.

#include "ScriptedEditorScreenshot.h"

#include "Screenshotter.h"
#include "Async/Async.h"
#include "Engine/Engine.h"
#include "Framework/Application/SlateApplication.h"
#include "HAL/IConsoleManager.h"

DEFINE_LOG_CATEGORY(LogScriptedEditorScreenshot);

#define LOCTEXT_NAMESPACE "ScriptedEditorScreenshot"

void FScriptedEditorScreenshotModule::StartupModule()
{
	Screenshotter = MakeShared<FScreenshotter>();
	
	IConsoleManager::Get().RegisterConsoleCommand(TEXT("ScriptedEditorScreenshot.CaptureFile"),
		TEXT("Capture screenshots from a file specified as an argument"),
		FConsoleCommandWithArgsDelegate::CreateLambda([&](const TArray<FString>& Args)
		{
			if (Args.IsEmpty())
				return;

			FString File = FString::Join(Args, TEXT(" "));
			Screenshotter->CaptureScreenshots(File);
		}));

	IConsoleManager::Get().RegisterConsoleCommand(TEXT("ScriptedEditorScreenshot.CaptureThenQuit"),
		TEXT("Capture screenshots from a file specified as an argument, then quit the editor"),
		FConsoleCommandWithArgsDelegate::CreateLambda([&](const TArray<FString>& Args)
		{
			if (Args.IsEmpty())
				return;

			UE_LOG(LogScriptedEditorScreenshot, Log, TEXT("Capturing screenshots from file %s"), *FString::Join(Args, TEXT(" ")));
			FString File = FString::Join(Args, TEXT(" "));
			Screenshotter->CaptureScreenshots(File);

			AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [&]()
			{
				while (Screenshotter->CaptureInProgress()) {
					FPlatformProcess::Sleep(0.5);
				}
				UE_LOG(LogScriptedEditorScreenshot, Log, TEXT("Capturing done, waiting for shutdown"));
				FPlatformProcess::Sleep(1.);
				UE_LOG(LogScriptedEditorScreenshot, Log, TEXT("Shutting down"));
				FPlatformMisc::RequestExit(false);
			});
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
	IConsoleManager& mgr = IConsoleManager::Get();
	mgr.UnregisterConsoleObject(TEXT("ScriptedEditorScreenshot.Capture"));
	mgr.UnregisterConsoleObject(TEXT("ScriptedEditorScreenshot.CaptureFile"));
	mgr.UnregisterConsoleObject(TEXT("ScriptedEditorScreenshot.CaptureThenQuit"));
	Screenshotter.Reset();
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FScriptedEditorScreenshotModule, ScriptedEditorScreenshot)