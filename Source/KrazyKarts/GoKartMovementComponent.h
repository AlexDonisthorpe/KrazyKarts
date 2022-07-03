// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GoKartMovementComponent.generated.h"

USTRUCT()
struct FGoKartMove
{
	GENERATED_BODY()

	UPROPERTY()
	float Throttle;
	UPROPERTY()
	float SteeringThrow;

	UPROPERTY()
	float DeltaTime;
	UPROPERTY()
	float Time;

	bool IsValid() const
	{
		return FMath::Abs(Throttle) <= 1 && SteeringThrow <= 1;
	}
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class KRAZYKARTS_API UGoKartMovementComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UGoKartMovementComponent();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void SimulateMove(const FGoKartMove& Move);
	FGoKartMove GetLastMove() { return LastMove; }

	FVector GetVelocity() { return Velocity; }
	void SetVelocity(FVector NewVelocity) { Velocity = NewVelocity; }

	void SetThrottle(float NewThrottle) { Throttle = NewThrottle; }
	void SetSteeringThrow(float NewSteeringThrow) { SteeringThrow = NewSteeringThrow; }

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	FVector GetAirResistance();
	FVector GetRollingResistance();

	void ApplyRotation(float DeltaTime, float NewSteeringThrow);
	void UpdateLocationFromVelocity(float DeltaTime);

	FGoKartMove CreateMove(float DeltaTime);

	// The Mass of the Car (kg)
	UPROPERTY(EditAnywhere)
	float Mass = 1000;

	// Force applied to the car when the throttle is fully down (N)
	UPROPERTY(EditAnywhere)
	float MaxDrivingForce = 10000;

	// Minimum radius of the car turning circle at full lock (m)
	UPROPERTY(EditAnywhere)
	float MinimumTurningRadius = 10;

	// Higher means more Drag (kg/per meter)
	UPROPERTY(EditAnywhere)
	float DragCoefficient = 16;

	// Higher means more rolling resistance
	UPROPERTY(EditAnywhere)
	float RollingResistanceCoefficient = 0.015f;

	float Throttle;
	float SteeringThrow;

	FVector Velocity;
	FGoKartMove LastMove;
};
