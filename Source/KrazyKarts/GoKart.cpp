// Fill out your copyright notice in the Description page of Project Settings.


#include "GoKart.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/GameStateBase.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AGoKart::AGoKart()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
}

// Called when the game starts or when spawned
void AGoKart::BeginPlay()
{
	Super::BeginPlay();
}

void AGoKart::GetLifetimeReplicatedProps( TArray< FLifetimeProperty > & OutLifetimeProps ) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME( AGoKart, ServerState );
}

void AGoKart::ApplyRotation(float DeltaTime, float NewSteeringThrow)
{
	float DeltaLocation = FVector::DotProduct(GetActorForwardVector(), Velocity) * DeltaTime;
	float RotationAngle = DeltaLocation / MinimumTurningRadius * NewSteeringThrow;
	FQuat RotationDelta(GetActorUpVector(),
	                    RotationAngle);

	Velocity = RotationDelta.RotateVector(Velocity);
	AddActorWorldRotation(RotationDelta, true);
}

void AGoKart::CreateMove(float DeltaTime, FGoKartMove& Move)
{
	Move.DeltaTime = DeltaTime;
	Move.SteeringThrow = SteeringThrow;
	Move.Throttle = Throttle;

	const AGameStateBase* GameState = GetWorld()->GetGameState();
	if(GameState != nullptr)
	{
		Move.Time = GameState->GetServerWorldTimeSeconds();
	}
}

void AGoKart::ClearAcknowledgedMoves(FGoKartMove LastMove)
{
	TArray<FGoKartMove> NewMoves;

	for(const FGoKartMove& Move : UnacknowledgedMoves)
	{
		if(Move.Time > LastMove.Time)
		{
			NewMoves.Add(Move);
		}
	}

	UnacknowledgedMoves = NewMoves;
}

// Called every frame
void AGoKart::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FGoKartMove Move;
	CreateMove(DeltaTime, Move);

	const ENetRole LocalRole = GetLocalRole();
	
	if(LocalRole == ROLE_AutonomousProxy)
	{
		UnacknowledgedMoves.Add(Move);
		SimulateMove(Move);
	}

	// We are the server and in control of the pawn
	if(LocalRole == ROLE_Authority && IsLocallyControlled())
	{
		Server_SendMove(Move);
	}

	if(LocalRole == ROLE_SimulatedProxy)
	{
		SimulateMove(ServerState.LastMove);
	}
}

void AGoKart::OnRep_ServerState()
{
	SetActorTransform(ServerState.Transform);
	Velocity = ServerState.Velocity;

	ClearAcknowledgedMoves(ServerState.LastMove);

	for (const FGoKartMove& Move : UnacknowledgedMoves)
	{
		SimulateMove(Move);
	}
}

// Called to bind functionality to input
void AGoKart::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AGoKart::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AGoKart::MoveRight);
}

void AGoKart::SimulateMove(const FGoKartMove& Move)
{
	FVector Force = MaxDrivingForce * Move.Throttle * GetActorForwardVector();
	Force += GetAirResistance();
	Force += GetRollingResistance();
	
	FVector Acceleration = Force / Mass;

	Velocity = Velocity + Acceleration * Move.DeltaTime;

	ApplyRotation(Move.DeltaTime, Move.SteeringThrow);
	UpdateLocationFromVelocity(Move.DeltaTime);
}

FVector AGoKart::GetAirResistance()
{
	return - Velocity.GetSafeNormal() * Velocity.SizeSquared() * DragCoefficient; 
}

FVector AGoKart::GetRollingResistance()
{
	// In Meters
	float AccelerationDueToGravity = -GetWorld()->GetGravityZ() / 100;
	float NormalForce = Mass * AccelerationDueToGravity;
	
	return - Velocity.GetSafeNormal() * RollingResistanceCoefficient * NormalForce;
}

void AGoKart::MoveForward(float Value)
{
	Throttle = Value;
}

void AGoKart::MoveRight(float Value)
{
	if(Velocity.Z != 0 || Velocity.X != 0)
	{
		SteeringThrow = Value;
	}
}

void AGoKart::Server_SendMove_Implementation(FGoKartMove Move)
{
	SimulateMove(Move);

	ServerState.LastMove = Move;
	ServerState.Transform = GetActorTransform();
	ServerState.Velocity = Velocity;
}

bool AGoKart::Server_SendMove_Validate(FGoKartMove Move)
{
	return true; // TODO Validate
}

void AGoKart::UpdateLocationFromVelocity(float DeltaTime)
{
	FVector Translation = Velocity * DeltaTime * 100;

	FHitResult Hit;
	AddActorWorldOffset(Translation, true, &Hit);

	if(Hit.IsValidBlockingHit())
	{
		Velocity = FVector::ZeroVector;
	}
}