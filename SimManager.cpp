#include "SimManager.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"

ASimManager::ASimManager()
{
    PrimaryActorTick.bCanEverTick = false;
}

void ASimManager::SetWireGlowByTag(FString FullTagName, bool bShouldGlow)
{
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName(*FullTagName), FoundActors);

    UMaterialInterface* Mat = bShouldGlow ? GlowMaterial : DefaultMaterial;
    if (!Mat) return;

    for (AActor* Actor : FoundActors)
    {
        TArray<UStaticMeshComponent*> Meshes;
        Actor->GetComponents<UStaticMeshComponent>(Meshes);
        for (UStaticMeshComponent* Mesh : Meshes)
        {
            Mesh->SetMaterial(0, Mat);
        }
    }
}

void ASimManager::SetWireGlow(int32 RegisterIndex, FString WirePrefix, bool bShouldGlow)
{
    FString FullTag = FString::Printf(TEXT("%s%d"), *WirePrefix, RegisterIndex);
    SetWireGlowByTag(FullTag, bShouldGlow);
}

void ASimManager::ResetAllWires()
{
    TArray<AActor*> AllWireActors;
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("Wire"), AllWireActors);

    // DEBUG: This will print how many wires were found to the Output Log (bottom of UE5)
    UE_LOG(LogTemp, Warning, TEXT("ResetAllWires found %d actors with the 'Wire' tag."), AllWireActors.Num());

    if (AllWireActors.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("NO WIRES FOUND! Make sure your meshes have the 'Wire' tag in the Actor section."));
    }

    for (AActor* Actor : AllWireActors)
    {
        if (!Actor) continue;

        TArray<UStaticMeshComponent*> Meshes;
        Actor->GetComponents<UStaticMeshComponent>(Meshes);

        for (UStaticMeshComponent* Mesh : Meshes)
        {
            if (DefaultMaterial)
            {
                Mesh->SetMaterial(0, DefaultMaterial);
            }
        }
    }
}

void ASimManager::UpdateWireVisuals(int32 rs1, int32 rs2, int32 rd, bool bWriteEnable)
{
    ResetAllWires();

    // 1. Highlight Mux A path (Read RS1)
    SetWireGlow(rs1, "W_MuxA_x", true);

    // 2. Highlight Mux B path (Read RS2)
    SetWireGlow(rs2, "W_MuxB_x", true);

    // 3. Highlight Decoder Selection and Write-Back Bus
    if (bWriteEnable && rd != 0)
    {
        // Control Line: "Open the gate for Register RD"
        SetWireGlow(rd, "W_Dec_RegSelect_x", true);

        // Data Line: "Carry ALU result back to Register RD"
        SetWireGlow(rd, "W_WriteBack_x", true);
    }
}