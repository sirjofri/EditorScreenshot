// Copyright sirjofri. All rights reserved. See License file for more info.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(LogScriptedEditorScreenshot, Log, All);

class FScreenshotter;

class FScriptedEditorScreenshotModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	TSharedPtr<FScreenshotter> GetScreenshotter() const { return Screenshotter; }

private:
	TSharedPtr<FScreenshotter> Screenshotter;
};
