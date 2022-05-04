// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraModifier.h"
#include "CamZoom.generated.h"

/**
 * 
 */
UCLASS()
class KRAZYKARTS_API UCamZoom : public UCameraModifier
{
	GENERATED_BODY()

	UCamZoom();

	//Override
	virtual bool ModifyCamera(float DeltaTime, struct FMinimalViewInfo& InOutPOV) override;
	//End Override

	UPROPERTY(EditDefaultsOnly)
	UCurveFloat* foo;

	virtual void EnableModifier() override;

	virtual void AddedToCamera(APlayerCameraManager* Camera) override;

	float baseFoV;
};
