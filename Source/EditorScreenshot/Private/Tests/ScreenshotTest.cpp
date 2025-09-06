// Copyright sirjofri. All rights reserved. See License file for more info.

#include "EditorScreenshot.h"
#include "Screenshotter.h"
#include "HAL/FileManager.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/AutomationTest.h"
#include "Modules/ModuleManager.h"
#include "Tests/AutomationCommon.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FScreenshotTest, "EditorScreenshot.ScreenshotTest",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter | EAutomationTestFlags::NonNullRHI)

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(FWaitForCaptureDone, FScreenshotTest*, Test, TSharedPtr<FScreenshotter>, Screenshotter);

bool FWaitForCaptureDone::Update()
{
	bool IsRunning = GetCurrentRunTime() > 0.1;
	
	if (GetCurrentRunTime() > 5.) {
		Test->AddError(TEXT("ScreenshotTest timed out"));
		return true;
	}
	if (IsRunning && !Screenshotter->CaptureInProgress())
		return true;
	
	if (Screenshotter->CaptureInProgress())
		return false;
	FPlatformProcess::Sleep(0.1);
	return false;
}

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(FCaptureScreenshotVerifyTests, FString, File, FScreenshotTest*, Test);

bool FCaptureScreenshotVerifyTests::Update()
{
	FString Path = FPaths::ProjectSavedDir() / TEXT("EditorScreenshots") / FPaths::GetBaseFilename(File);
	TArray<FString> Files = {
		TEXT("DebugTools_DebugPanel.png"),
		TEXT("LocalizationTargets.png"),
	};

	for (FString& f : Files) {
		FString FullFile = Path / f;
		if (!FPaths::FileExists(FullFile)) {
			Test->AddError(TEXT("File not found: ") + FullFile);
		}
		// TODO: Ideally, we can see if the image file makes sense (e. g. by looking at the file size or the contents)
	}

	IFileManager& fmgr = IFileManager::Get();
	fmgr.DeleteDirectory(*Path, false, true);
	return true;
}


bool FScreenshotTest::RunTest(const FString& Parameters)
{
	IPluginManager& PluginManager = IPluginManager::Get();
	FString File = PluginManager.FindPlugin(TEXT("EditorScreenshot"))->GetBaseDir() / TEXT("Source/EditorScreenshot/Private/Tests") / TEXT("TestDescription.ini");
	File = FPaths::ConvertRelativePathToFull(File);

	if (!FPaths::FileExists(File)) {
		AddError(TEXT("Test description file not found"));
		return false;
	}
	
	AddInfo(TEXT("Description file: ") + File);

	TSharedPtr<FScreenshotter> Screenshotter = FModuleManager::LoadModuleChecked<FEditorScreenshotModule>("EditorScreenshot").GetScreenshotter();

	ADD_LATENT_AUTOMATION_COMMAND(FExecStringLatentCommand(TEXT("EditorScreenshot.Capture ") + File));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForCaptureDone(this, Screenshotter));
	ADD_LATENT_AUTOMATION_COMMAND(FEngineWaitLatentCommand(2.));
	ADD_LATENT_AUTOMATION_COMMAND(FCaptureScreenshotVerifyTests(File, this));
	
	return !HasAnyErrors();
}
