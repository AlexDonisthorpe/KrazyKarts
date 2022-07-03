// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GoKartMovementComponent.h"
#include "GoKartMovementReplicator.generated.h"

USTRUCT()
struct FGoKartState
{
	GENERATED_BODY()

	UPROPERTY()
	FTransform Transform;
	
	UPROPERTY()
	FVector Velocity;

	UPROPERTY()
	FGoKartMove LastMove;
};

struct FHermiteCubicSpline
{
	FVector StartLocation;
	FVector StartDerivative;
	FVector TargetLocation;
	FVector TargetDerivative;

	FVector InterpolateLocation(const float LerpRatio) const
	{
		return FMath::CubicInterp(StartLocation, StartDerivative, TargetLocation, TargetDerivative, LerpRatio);
	}
	
	FVector InterpolateDerivative(const float LerpRatio) const
	{
		return FMath::CubicInterpDerivative(StartLocation, StartDerivative, TargetLocation, TargetDerivative, LerpRatio);
	}
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class KRAZYKARTS_API UGoKartMovementReplicator : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UGoKartMovementReplicator();

	void UpdateServerState(const FGoKartMove& Move);
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SendMove(FGoKartMove Move);

	TArray<FGoKartMove> UnacknowledgedMoves;

	UPROPERTY(ReplicatedUsing = OnRep_ServerState)
	FGoKartState ServerState;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
private:
	void ClearAcknowledgedMoves(FGoKartMove LastMove);
	void ClientTick(const float& DeltaTime);
	
	UFUNCTION()
	void OnRep_ServerState();
	void SimulatedProxy_OnRep_ServerState();
	void AutonomousProxy_OnRep_ServerState();

	FHermiteCubicSpline CreateSpline();

	void InterpolateLocation(const FHermiteCubicSpline& Spline, const float& LerpRatio);
	void InterpolateVelocity(const FHermiteCubicSpline& Spline, const float& LerpRatio);
	void InterpolateRotation(const float& LerpRatio);

	float VelocityToDerivative() const
	{
		return ClientTimeBetweenLastUpdate * 100;
	}

	UPROPERTY()
	TWeakObjectPtr<UGoKartMovementComponent> MovementComponent;

	float ClientTimeSinceUpdate = 0.f;
	float ClientTimeBetweenLastUpdate = 0.f;

	FTransform ClientStartTransform;
	FVector ClientStartVelocity;
	float ClientSimulatedTime = 0.f;

	UPROPERTY()
	USceneComponent* MeshOffsetRoot;

	UFUNCTION(BlueprintCallable)
	void SetMeshOffsetRoot(USceneComponent* Root) { MeshOffsetRoot = Root;}
};
