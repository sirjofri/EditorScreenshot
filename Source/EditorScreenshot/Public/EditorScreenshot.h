// Copyright sirjofri. All rights reserved. See License file for more info.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(LogEditorScreenshot, Log, All);

class FScreenshotter;

class FEditorScreenshotModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	TSharedPtr<FScreenshotter> Screenshotter;
};
