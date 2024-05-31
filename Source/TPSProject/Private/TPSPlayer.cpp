// Fill out your copyright notice in the Description page of Project Settings.


#include "TPSPlayer.h"
#include <GameFramework/SpringArmComponent.h>
#include <Camera/CameraComponent.h>
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/Controller.h"
#include "Bullet.h"
#include <Blueprint/UserWidget.h>
#include <Kismet/GameplayStatics.h>
#include "EnemyFSM.h"
#include <GameFramework/CharacterMovementComponent.h>
#include "PlayerAnim.h"
#include "TPSProject.h"
#include <Kismet/GameplayStatics.h>

// Sets default values
ATPSPlayer::ATPSPlayer()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	springArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	springArmComp->SetupAttachment(RootComponent);
	springArmComp->SetRelativeLocation(FVector(0, 70, 90));
	springArmComp->TargetArmLength = 400;
	springArmComp->bUsePawnControlRotation = true;

	tpsCamComp = CreateDefaultSubobject<UCameraComponent>(TEXT("TpsCamComp"));
	tpsCamComp->SetupAttachment(springArmComp);
	tpsCamComp->bUsePawnControlRotation = false;

	bUseControllerRotationPitch = true;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	JumpMaxCount = 2;

	gunMeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("GunMeshComp"));
	gunMeshComp->SetupAttachment(GetMesh(), TEXT("hand_rSocket"));

	sniperGunComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SniperGunComp"));
	sniperGunComp->SetupAttachment(GetMesh(), TEXT("hand_rSocket"));
}

// Called when the game starts or when spawned
void ATPSPlayer::BeginPlay()
{
	Super::BeginPlay();

	hp = initialHp;

	GetCharacterMovement()->MaxWalkSpeed = walkSpeed;

	ChangeToSniperGun();

	_sniperUI = CreateWidget(GetWorld(), sniperUIFactory);
	_crosshairUI = CreateWidget(GetWorld(), crosshairUIFactory);

	_crosshairUI->AddToViewport();
}

// Called every frame
void ATPSPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ATPSPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	UEnhancedInputComponent* EnhancedPlayerInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (EnhancedPlayerInputComponent != nullptr) {
		APlayerController* PlayerController = Cast<APlayerController>(GetController());
		if (PlayerController != nullptr) {
			UEnhancedInputLocalPlayerSubsystem* EnhancedSubsystem =
				ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
			if (EnhancedSubsystem != nullptr) {
				EnhancedSubsystem->AddMappingContext(IC_tpsPlayer, 1);
			}
		}
		EnhancedPlayerInputComponent->BindAction(IA_tpsLook, ETriggerEvent::Triggered, this, &ATPSPlayer::Look);
		EnhancedPlayerInputComponent->BindAction(IA_tpsMove, ETriggerEvent::Triggered, this, &ATPSPlayer::Move);
		EnhancedPlayerInputComponent->BindAction(IA_tpsJump, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedPlayerInputComponent->BindAction(IA_tpsJump, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		EnhancedPlayerInputComponent->BindAction(IA_tpsFire, ETriggerEvent::Triggered, this, &ATPSPlayer::Fire);
		EnhancedPlayerInputComponent->BindAction(IA_GrenadeGun, ETriggerEvent::Triggered, this, &ATPSPlayer::ChangeToGrenadeGun);
		EnhancedPlayerInputComponent->BindAction(IA_SniperGun, ETriggerEvent::Triggered, this, &ATPSPlayer::ChangeToSniperGun);
		EnhancedPlayerInputComponent->BindAction(IA_SniperAim, ETriggerEvent::Started, this, &ATPSPlayer::SniperAim);
		EnhancedPlayerInputComponent->BindAction(IA_SniperAim, ETriggerEvent::Completed, this, &ATPSPlayer::SniperAim);
		EnhancedPlayerInputComponent->BindAction(IA_tpsRun, ETriggerEvent::Started, this, &ATPSPlayer::Run);
		EnhancedPlayerInputComponent->BindAction(IA_tpsRun, ETriggerEvent::Completed, this, &ATPSPlayer::Run);
	}
}


void ATPSPlayer::Move(const FInputActionValue& Value) {
	FVector2D InputValue = Value.Get<FVector2D>();
	if (Controller != nullptr && (InputValue.X != 0.0f || InputValue.Y != 0.0f)) {
		const FRotator YawRotation(0, Controller->GetControlRotation().Yaw, 0);

		if (InputValue.X != 0.0f) {
			const FVector RightDirection = UKismetMathLibrary::GetRightVector(YawRotation);
			AddMovementInput(RightDirection, InputValue.X);
		}
		if (InputValue.Y != 0.0f) {
			const FVector ForwardDirection = YawRotation.Vector();
			AddMovementInput(ForwardDirection, InputValue.Y);
		}
	}

}


void ATPSPlayer::Look(const FInputActionValue& Value) {
	FVector2D InputValue = Value.Get<FVector2D>();
	UE_LOG(LogTemp, Log, TEXT("Log Message"));
	if (InputValue.X != 0.0f) {
		AddControllerYawInput(InputValue.X);

	}
	if (InputValue.Y != 0.0f) {
		AddControllerPitchInput(InputValue.Y);
	}
}

void ATPSPlayer::Fire() {

	UGameplayStatics::PlaySound2D(GetWorld(), bulletSound);

	auto controller = GetWorld()->GetFirstPlayerController();
	controller->PlayerCameraManager->StartCameraShake(cameraShake);

	auto anim = Cast<UPlayerAnim>(GetMesh()->GetAnimInstance());
	anim->PlayAttackAnim();

	if (bUsingGrenadeGun) {
		FTransform firePosition = gunMeshComp->GetSocketTransform(TEXT("FirePosition"));
		GetWorld()->SpawnActor<ABullet>(bulletFactory, firePosition);
	}
	else {
		FVector startPos = tpsCamComp->GetComponentLocation();
		FVector endPos = tpsCamComp->GetComponentLocation() + tpsCamComp->GetForwardVector() * 5000;
		FHitResult hitInfo;
		FCollisionQueryParams params;
		params.AddIgnoredActor(this);
		bool bHit = GetWorld()->LineTraceSingleByChannel(hitInfo, startPos, endPos, ECC_Visibility, params);
		if (bHit) {
			FTransform bulletTrans;
			bulletTrans.SetLocation(hitInfo.ImpactPoint);
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), bulletEffectFactory, bulletTrans);
			
			auto hitComp = hitInfo.GetComponent();
			if (hitComp && hitComp->IsSimulatingPhysics()) {
				FVector force = -hitInfo.ImpactNormal * hitComp->GetMass() * 500000;
				hitComp->AddForce(force);
			}

			auto enemy = hitInfo.GetActor()->GetDefaultSubobjectByName(TEXT("FSM"));
			if (enemy) {
				auto enemyFSM = Cast<UEnemyFSM>(enemy);
				enemyFSM->OnDamageProcess();
			}
		}
	}
}

void ATPSPlayer::ChangeToGrenadeGun() {
	bUsingGrenadeGun = true;
	sniperGunComp->SetVisibility(false);
	gunMeshComp->SetVisibility(true);

	OnUsingGrenade(bUsingGrenadeGun);
}

void ATPSPlayer::ChangeToSniperGun() {
	bUsingGrenadeGun = false;
	sniperGunComp->SetVisibility(true);
	gunMeshComp->SetVisibility(false);

	OnUsingGrenade(bUsingGrenadeGun);
}

void ATPSPlayer::SniperAim() {
	if (bUsingGrenadeGun) {
		return;
	}
	if (bSniperAim == false) {
		bSniperAim = true;
		_sniperUI->AddToViewport();
		tpsCamComp->SetFieldOfView(45.0f);
		_crosshairUI->RemoveFromParent();
	}
	else {
		bSniperAim = false;
		_sniperUI->RemoveFromParent();
		tpsCamComp->SetFieldOfView(90.0f);
		_crosshairUI->AddToViewport();
	}
}

void ATPSPlayer::Run() {
	auto movement = GetCharacterMovement();
	if (movement->MaxWalkSpeed > walkSpeed) {
		movement->MaxWalkSpeed = walkSpeed;
	}
	else {
		movement->MaxWalkSpeed = runSpeed;
	}
}

void ATPSPlayer::OnHitEvent() {
	PRINT_LOG(TEXT("Damaged !!!!!!!!!"));

	hp--;
	if (hp <= 0) {
		PRINT_LOG(TEXT("Player is dead!"));

		OnGameOver();
	}
}

void ATPSPlayer::OnGameOver_Implementation() {
	UGameplayStatics::SetGamePaused(GetWorld(), true);
}
