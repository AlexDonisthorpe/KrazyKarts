// Fill out your copyright notice in the Description page of Project Settings.


#include "CamZoom.h"
#include "Curves/CurveFloat.h"

UCamZoom::UCamZoom()
{
	bDisabled=0;
}

bool UCamZoom::ModifyCamera(float DeltaTime, FMinimalViewInfo& InOutPOV)
{
	const float fovIncrease = 50.f;

	InOutPOV.FOV = baseFoV + fovIncrease*foo->GetFloatValue(Alpha);

	GEngine->AddOnScreenDebugMessage(-1, 1, FColor::White, FString::Printf(TEXT("%f"), InOutPOV.FOV));	
	return Super::ModifyCamera(DeltaTime, InOutPOV);
}

void UCamZoom::EnableModifier()
{
	Super::EnableModifier();
}

void UCamZoom::AddedToCamera(APlayerCameraManager* Camera)
{
	Super::AddedToCamera(Camera);

	baseFoV = Camera->DefaultFOV;
	
	if(!foo)
	{
		return;
	}
    
	auto& test = foo->FloatCurve.Keys;
	
	if(test.Last().Value == AlphaInTime)
	{
		return;
	}
	else
	for(auto& a : test)
	{
		a.Value /= test.Last().Value;
		a.Time /= test.Last().Time;
			
		a.Value *= AlphaInTime;
		a.Time *= AlphaInTime;
	}



}
