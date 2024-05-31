// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "TPSPlayer.generated.h"

UCLASS()
class TPSPROJECT_API ATPSPlayer : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ATPSPlayer();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void Look(const FInputActionValue& Value);

	void Move(const FInputActionValue& Value);

	void Fire();

	void ChangeToGrenadeGun();
	void ChangeToSniperGun();

	void SniperAim();

	void Run();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(EditAnywhere, Category = PlayerSetting)
	float walkSpeed = 200;

	UPROPERTY(EditAnywhere, Category = PlayerSetting)
	float runSpeed = 600;

	UPROPERTY(VisibleAnywhere, Category = Camera)
	class USpringArmComponent* springArmComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	class UCameraComponent* tpsCamComp;

	UPROPERTY(VisibleAnywhere, Category = GunMesh)
	class USkeletalMeshComponent* gunMeshComp;

	UPROPERTY(VisibleAnywhere, Category = GunMesh)
	class UStaticMeshComponent* sniperGunComp;

	UPROPERTY(EditDefaultsOnly, Category = BulletFactory)
	TSubclassOf<class ABullet> bulletFactory;

	UPROPERTY(EditAnywhere, Category = BulletEffect)
	class UParticleSystem* bulletEffectFactory;

	UPROPERTY(EditDefaultsOnly, Category = SniperUI)
	TSubclassOf<class UUserWidget> sniperUIFactory;

	UPROPERTY(EditDefaultsOnly, Category = SniperUI)
	TSubclassOf<class UUserWidget> crosshairUIFactory;

	UPROPERTY(EditDefaultsOnly, Category = CameraMotion)
	TSubclassOf<class UCameraShakeBase> cameraShake;

	UPROPERTY(EditDefaultsOnly, Category = Sound)
	class USoundBase* bulletSound;

	UPROPERTY(EditAnywhere, Category = Input)
	class UInputMappingContext* IC_tpsPlayer;

	UPROPERTY(EditAnywhere, Category = Input)
	class UInputAction* IA_tpsLook;

	UPROPERTY(EditAnywhere, Category = Input)
	class UInputAction* IA_tpsMove;

	UPROPERTY(EditAnywhere, Category = Input)
	class UInputAction* IA_tpsJump;

	UPROPERTY(EditAnywhere, Category = Input)
	class UInputAction* IA_tpsFire;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Health)
	int32 hp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Health)
	int32 initialHp = 10;

	UFUNCTION(BlueprintCallable, Category = Health)
	void OnHitEvent();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = Health)
	void OnGameOver();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = Health)
	void OnGameWin();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = Health)
	void OnUsingGrenade(bool isGrenade);

	UPROPERTY(EditAnywhere, Category = Input)
	class UInputAction* IA_GrenadeGun;

	UPROPERTY(EditAnywhere, Category = Input)
	class UInputAction* IA_SniperGun;

	UPROPERTY(EditAnywhere, Category = Input)
	class UInputAction* IA_SniperAim;

	UPROPERTY(EditAnywhere, Category = Input)
	class UInputAction* IA_tpsRun;

	bool bUsingGrenadeGun = true;

	bool bSniperAim = false;

	class UUserWidget* _sniperUI;

	class UUserWidget* _crosshairUI;
};
