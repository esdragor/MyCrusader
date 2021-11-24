// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "UnitBase.generated.h"

UCLASS()
class MYCRUSADER_API AUnitBase : public ACharacter
{
	GENERATED_BODY()

public :
	/** Returns CursorToWorld subobject **/
	FORCEINLINE class UStaticMeshComponent* GetCursorToWorld() { return CursorToWorld; }

	UPROPERTY(VisibleAnywhere)
		int IndexPlayer = 0;
	void SetSelected();
	void SetDeselected();
	UFUNCTION(BlueprintCallable)
		void AddTargetEndOverlap(AActor* actorEnd);
	UFUNCTION(BlueprintCallable)
		void DeleteTargetEndOverlap(AActor* actorEnd);
	UFUNCTION(BlueprintCallable)
		float getHealthPercent();
	//UPROPERTY(EditAnywhere)
	//	ASquadronManager* squadron;

protected:

	UPROPERTY(EditInstanceOnly)
		int MaxlifePoint = 15;
	UPROPERTY(EditInstanceOnly)
		int lifePoint = MaxlifePoint;
	UPROPERTY(EditInstanceOnly)
		int Atk_value = 18;
	UPROPERTY(EditInstanceOnly)
		int Def_value = 10;
	UPROPERTY(EditInstanceOnly)
		int Accuracy_value = 10;
	UPROPERTY(EditInstanceOnly)
		int Speed_value = 12;
	UPROPERTY(EditInstanceOnly)
		float maxSpeedAttack = (float)Speed_value / 12.f;
	UPROPERTY(EditInstanceOnly)
		float currentSpeedAttack = maxSpeedAttack;




	UPROPERTY(EditInstanceOnly)
		AUnitBase* target = nullptr;

	bool TakeAHit(int Atk_ennemy);
	UFUNCTION(BlueprintImplementableEvent)
		bool TakeAHitBP();
	void Death();

private:

	/** A decal that projects to the cursor location. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class UStaticMeshComponent* CursorToWorld;













public:
	// Sets default values for this character's properties
	AUnitBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
