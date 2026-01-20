#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SimManager.generated.h"

UCLASS()
class CPUSIMULATOR_API ASimManager : public AActor
{
    GENERATED_BODY()

public:
    ASimManager();

    // 1. General version: Glow anything by its full tag (e.g. "Result_Bus")
    UFUNCTION(BlueprintCallable, Category = "Simulation Manager")
    void SetWireGlowByTag(FString FullTagName, bool bShouldGlow);

    // 2. Indexed version: Helper for registers (e.g. Index 5 + "W_MuxA_x" -> "W_MuxA_x5")
    UFUNCTION(BlueprintCallable, Category = "Simulation Manager")
    void SetWireGlow(int32 RegisterIndex, FString WirePrefix, bool bShouldGlow);

    // 3. Resets every actor in the level tagged "Wire" to DefaultMaterial
    UFUNCTION(BlueprintCallable, Category = "Simulation Manager")
    void ResetAllWires();

    // 4. The Master function called by the Processor every Step
    UFUNCTION(BlueprintCallable, Category = "Simulation Manager")
    void UpdateWireVisuals(int32 rs1, int32 rs2, int32 rd, bool bWriteEnable);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wire Visualization")
    UMaterialInterface* DefaultMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wire Visualization")
    UMaterialInterface* GlowMaterial;
};