// Fill out your copyright notice in the Description page of Project Settings.


#include "UnitBase.h"
#include "SelectionUnitHUD.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"


// Sets default values
AUnitBase::AUnitBase()
{
	// Set size for player capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	// Don't rotate character to camera direction
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Rotate character to moving direction
	GetCharacterMovement()->RotationRate = FRotator(0.f, 640.f, 0.f);
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;

	// Create a decal in the world to show the cursor's location

	static ConstructorHelpers::FObjectFinder<UStaticMesh>SphereMeshAsset(TEXT("StaticMesh'/Engine/BasicShapes/Sphere.Sphere'"));
	CursorToWorld = CreateDefaultSubobject<UStaticMeshComponent>("CursorToWorld");
	CursorToWorld->SetStaticMesh(SphereMeshAsset.Object);
	CursorToWorld->SetupAttachment(RootComponent);
	//static ConstructorHelpers::FObjectFinder<UMaterial> DecalMaterialAsset(TEXT("Material'/Game/TopDownCPP/Blueprints/M_Cursor_Decal.M_Cursor_Decal'"));
	//if (DecalMaterialAsset.Succeeded())
	//{
	//	CursorToWorld->SetDecalMaterial(DecalMaterialAsset.Object);
	//}
	//CursorToWorld->DecalSize = FVector(16.0f, 32.0f, 32.0f);
	CursorToWorld->SetWorldScale3D(FVector(0.01f, 1, 1));
	CursorToWorld->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f).Quaternion());
	CursorToWorld->SetRelativeLocation(FVector(0.f, 0.f, -80.f));
	CursorToWorld->SetVisibility(false);


	// Activate ticking in order to update the cursor every frame.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
}

// Called when the game starts or when spawned
void AUnitBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AUnitBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (target)
	{
		currentSpeedAttack -= DeltaTime;
		if (currentSpeedAttack <= 0.0f)
		{
			if (rand() % 100 < Accuracy_value)
				if (target->TakeAHit(Atk_value)) //is dead
					target = nullptr;
			currentSpeedAttack = maxSpeedAttack;
		}
	}
}



void AUnitBase::SetSelected()
{
		CursorToWorld->SetVisibility(true);
}

void AUnitBase::SetDeselected()
{
	CursorToWorld->SetVisibility(false);

}

void AUnitBase::AddTargetEndOverlap(AActor *actorEnd)
{
	if (target == nullptr)
	{
		AUnitBase *enemyBegin = Cast<AUnitBase>(actorEnd);
		if (enemyBegin != nullptr)
			if (enemyBegin->IndexPlayer != IndexPlayer) //different team
			{
				target = enemyBegin;
			}
	}
}

void AUnitBase::DeleteTargetEndOverlap(AActor *actorEnd)
{
	if (target != nullptr)
	{
		AUnitBase *enemyEnd = Cast<AUnitBase>(actorEnd);
		if (enemyEnd != nullptr)
			if (target == enemyEnd)
				target = nullptr;
	}
}

bool AUnitBase::TakeAHit(int Atk_ennemy)
{
	int damage = Atk_ennemy - Def_value;

	if (damage <= 0)
		return false;
	lifePoint -= damage;
	if (lifePoint <= 0)
	{
		Death();
		return (true);
	}
	TakeAHitBP();
	return (false);
}

void AUnitBase::Death()
{
	//squadron->GetUnits()->Remove(this);

	Cast<ASelectionUnitHUD>(GetWorld()->GetFirstPlayerController()->GetHUD())->UnitSelection.Remove(this);
	Destroy();
}

float AUnitBase::getHealthPercent()
{
	return ((float)lifePoint / (float)MaxlifePoint);
}

// Called to bind functionality to input
void AUnitBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

