#pragma once

#include "RISCV_Processor.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

ARISCV_Processor::ARISCV_Processor()
{
    SetupRootComponent();
    CreateRegisterPillars();
    CreateFloatingInfoText();
    CreateALUMesh();
}

void ARISCV_Processor::BeginPlay()
{
    Super::BeginPlay();
    Find_SimManager_in_Level();
    ResetAndLoad();
}

void ARISCV_Processor::Find_SimManager_in_Level()
{
    // AUTO-LINK: Find the SimManager in the level automatically
    AActor* FoundActor = UGameplayStatics::GetActorOfClass(GetWorld(), ASimManager::StaticClass());
    if (FoundActor)
    {
        SimManager = Cast<ASimManager>(FoundActor);
        UE_LOG(LogTemp, Warning, TEXT("RISC-V Processor: SimManager linked successfully."));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("RISC-V Processor: SimManager not found in level!"));
    }
}

void ARISCV_Processor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    UpdateFloatingTextRotation();
}

void ARISCV_Processor::SetupRootComponent()
{
    PrimaryActorTick.bCanEverTick = true;
    CPU_Root = CreateDefaultSubobject<USceneComponent>(TEXT("CPU_Root"));
    RootComponent = CPU_Root;
    RegisterPillars.Empty();
}


/*
    Rigisters
*/

void ARISCV_Processor::CreateSingleRegisterPillar(int32 Index)
{
    FString MeshName = FString::Printf(TEXT("Pillar_%d"), Index);
    UStaticMeshComponent* NewPillar = CreateDefaultSubobject<UStaticMeshComponent>(*MeshName);
    NewPillar->SetupAttachment(RootComponent);

    float Row = Index / 8;
    float Col = Index % 8;
    FVector PillarLoc = FVector(Row * 150.0f, Col * 150.0f, 0.0f);
    NewPillar->SetRelativeLocation(PillarLoc);
    NewPillar->SetRelativeScale3D(FVector(0.5f, 0.5f, 2.0f));
    RegisterPillars.Add(NewPillar);

    // Create and attach text label
    FString TextName = FString::Printf(TEXT("RegText_%d"), Index);
    UTextRenderComponent* NewText = CreateDefaultSubobject<UTextRenderComponent>(*TextName);
    NewText->SetupAttachment(NewPillar);

    const float SmallBaseHeight = 30.0f;
    const float SmallMargin = 460.0f;
    float PillarScaleZ = NewPillar->GetRelativeScale3D().Z;
    float OffsetZ = SmallBaseHeight * PillarScaleZ + SmallMargin;

    NewText->SetRelativeLocation(FVector(0.0f, 0.0f, OffsetZ));
    NewText->SetHorizontalAlignment(EHTA_Center);
    NewText->SetWorldSize(30.0f);
    NewText->SetTextRenderColor(FColor::White);
    NewText->SetText(FText::FromString(FString::Printf(TEXT("x%d: 0"), Index)));

    RegisterTexts[Index] = NewText;
}

void ARISCV_Processor::CreateRegisterPillars()
{
    for (int32 i = 0; i < 32; ++i)
    {
        RegisterTexts[i] = nullptr;
    }

    for (int32 i = 0; i < 32; ++i)
    {
        CreateSingleRegisterPillar(i);
    }
}

void ARISCV_Processor::CreateFloatingInfoText()
{
    FloatingInfoText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("InstructionHUD"));
    FloatingInfoText->SetupAttachment(RootComponent);
    FloatingInfoText->SetRelativeLocation(FVector(200.0f, 450.0f, 300.0f));
    FloatingInfoText->SetWorldSize(60.0f);
    FloatingInfoText->SetTextRenderColor(FColor::Cyan);
}

int32 ARISCV_Processor::GetRegister(int32 Index)
{
    return (int32)CpuCore.GetRegisterValue(Index);
}

void ARISCV_Processor::UpdateFloatingTextRotation()
{
    if (!FloatingInfoText || !GetWorld())
    {
        return;
    }

    APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
    if (!PlayerController)
    {
        return;
    }

    APawn* ControlledPawn = PlayerController->GetPawn();
    if (!ControlledPawn)
    {
        return;
    }

    FVector CameraLocation = ControlledPawn->GetActorLocation();
    FRotator NewRotation = (CameraLocation - FloatingInfoText->GetComponentLocation()).Rotation();
    FloatingInfoText->SetWorldRotation(NewRotation);
}


/*
    ALU Unit
*/ 

void ARISCV_Processor::CreateALUMesh()
{
    ALUMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ALU_Unit"));
    ALUMesh->SetupAttachment(RootComponent);

    FVector ALULocation = FVector(225.0f, 525.0f, 50.0f);
    ALUMesh->SetRelativeLocation(ALULocation);
    ALUMesh->SetRelativeScale3D(FVector(1.5f, 1.5f, 1.5f));

    ALUOperationText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("ALU_Op_Text"));
    ALUOperationText->SetupAttachment(ALUMesh);
    ALUOperationText->SetRelativeLocation(FVector(0.0f, 0.0f, 100.0f));
    ALUOperationText->SetHorizontalAlignment(EHTA_Center);
    ALUOperationText->SetWorldSize(40.0f);
    ALUOperationText->SetTextRenderColor(FColor::Yellow);
    ALUOperationText->SetText(FText::FromString(TEXT("IDLE")));
}


/*
	Highlighting Rgisters
*/

void ARISCV_Processor::ResetRegisterMaterials()
{
    for (UStaticMeshComponent* Pillar : RegisterPillars)
    {
        if (DefaultMaterial)
        {
            Pillar->SetMaterial(0, DefaultMaterial);
        }
    }
}

void ARISCV_Processor::HighlightSourceRegisters(const DecodedInstruction& Decoded)
{
    if (!SourceRegMaterial)
    {
        return;
    }

    if (Decoded.rs1 < 32)
    {
        RegisterPillars[Decoded.rs1]->SetMaterial(0, SourceRegMaterial);
    }
    if (Decoded.rs2 < 32)
    {
        RegisterPillars[Decoded.rs2]->SetMaterial(0, SourceRegMaterial);
    }
}

void ARISCV_Processor::HighlightDestinationRegister(const DecodedInstruction& Decoded)
{
    if (DestRegMaterial && Decoded.rd < 32 && Decoded.rd != 0)
    {
        RegisterPillars[Decoded.rd]->SetMaterial(0, DestRegMaterial);
    }
}

void ARISCV_Processor::UpdateRegisterVisual(int32 RegisterIndex)
{
    if (RegisterIndex >= 0 && RegisterIndex < 32 && RegisterTexts[RegisterIndex])
    {
        int32 Val = (int32)CpuCore.GetRegisterValue(RegisterIndex);
        RegisterTexts[RegisterIndex]->SetText(FText::FromString(FString::Printf(TEXT("x%d: %d"), RegisterIndex, Val)));
    }
}

void ARISCV_Processor::UpdateVisuals()
{
    for (int i = 0; i < 32; i++)
    {
        UpdateRegisterVisual(i);
    }
}


/*
    Execution and Display
*/ 

void ARISCV_Processor::ExecuteAndDisplay(DecodedInstruction& Decoded)
{
    FloatingInfoText->SetText(FText::FromString(CpuCore.Disassemble(Decoded)));
    CpuCore.Execute(Decoded);
    UpdateVisuals();
}

void ARISCV_Processor::Step()
{
    // 1. Reset the 3D Register pillars
    ResetRegisterMaterials();

    // 2. Fetch and Decode the instruction
    uint32_t inst = CpuCore.FetchInstruction();
    DecodedInstruction decoded = CpuCore.Decode(inst);

    // 3. Highlight the 3D Register pillars (the physical boxes)
    HighlightSourceRegisters(decoded);
    HighlightDestinationRegister(decoded);

    // 4. Trigger all Wire Glows via the Manager
    if (SimManager)
    {
        // This handles MuxA, MuxB, RegSelect, and WriteBack in one go
        SimManager->UpdateWireVisuals(decoded.rs1, decoded.rs2, decoded.rd, (decoded.rd != 0));

    }

    // 5. Execute logic and update Floating Text
    ExecuteAndDisplay(decoded);
}

void ARISCV_Processor::ResetAndLoad()
{
    CpuCore = RISCV_CPU();
    std::vector<uint8_t> memoryBytes;
    Run_fibonacciProgram(memoryBytes); // put the program we want to run into memoryBytes
    CpuCore.LoadMemory(memoryBytes, 0);
    UE_LOG(LogTemp, Warning, TEXT("RISC-V: Fibonacci Program Loaded."));
}

int32 ARISCV_Processor::GetPC()
{
    return (int32)CpuCore.GetPC();
}
