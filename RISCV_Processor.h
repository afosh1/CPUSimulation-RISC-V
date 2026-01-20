#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include <vector>

#include "RISCV_CPU.h"
#include "Programs.h"
#include "SimManager.h"

#include "RISCV_Processor.generated.h"

class ASimManager;

UCLASS()
class CPUSIMULATOR_API ARISCV_Processor : public AActor
{
    GENERATED_BODY()

public:
    ARISCV_Processor();

protected:
    virtual void BeginPlay() override;

    void Find_SimManager_in_Level();

public:
    virtual void Tick(float DeltaTime) override;

    RISCV_CPU CpuCore;

    UFUNCTION(BlueprintCallable, Category = "RISC-V Control")
    void Step();

    UFUNCTION(BlueprintCallable, Category = "RISC-V Control")
    void ResetAndLoad();

    UFUNCTION(BlueprintPure, Category = "RISC-V Data")
    int32 GetRegister(int32 Index);

    UFUNCTION(BlueprintPure, Category = "RISC-V Data")
    int32 GetPC();

    UPROPERTY(VisibleAnywhere, Category = "Visualization")
    UTextRenderComponent* RegisterTexts[32];

    void UpdateVisuals();
    void ExecuteAndDisplay(DecodedInstruction& Decoded);

    void UpdateRegisterVisual(int32 RegisterIndex);
    void ResetRegisterMaterials();
    void HighlightSourceRegisters(const DecodedInstruction& Decoded);
    void HighlightDestinationRegister(const DecodedInstruction& Decoded);
    void SetupRootComponent();
    void CreateRegisterPillars();
    void CreateSingleRegisterPillar(int32 Index);
    void CreateFloatingInfoText();
    void CreateALUMesh();
    void UpdateFloatingTextRotation();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "3D CPU")
    UTextRenderComponent* FloatingInfoText;

    // Reference to the SimManager for wire glow control
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "3D CPU")
    ASimManager* SimManager;

    // ALU Mesh components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "3D CPU")
    UStaticMeshComponent* ALUMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "3D CPU")
    UTextRenderComponent* ALUOperationText;

public:
    // ============ Properties ============

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "3D CPU")
    TArray<UStaticMeshComponent*> RegisterPillars;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "3D CPU")
    UStaticMesh* PillarMesh;

    UPROPERTY(EditAnywhere, Category = "3D CPU")
    UMaterialInterface* DefaultMaterial;

    UPROPERTY(EditAnywhere, Category = "3D CPU")
    UMaterialInterface* ActiveMaterial;

    UPROPERTY(EditAnywhere, Category = "3D CPU")
    UMaterialInterface* SourceRegMaterial;

    UPROPERTY(EditAnywhere, Category = "3D CPU")
    UMaterialInterface* DestRegMaterial;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "3D CPU")
    USceneComponent* CPU_Root;
};

