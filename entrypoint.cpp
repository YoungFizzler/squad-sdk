#include <Windows.h>
#include <iostream>

#include "SDK/Engine_classes.hpp"

DWORD MainThread(HMODULE Module)
{
    /* Open a debug console */
    AllocConsole();
    FILE* Dummy;
    freopen_s(&Dummy, "CONOUT$", "w", stdout);
    freopen_s(&Dummy, "CONIN$", "r", stdin);

    /* Grab Unreal Engine globals */
    SDK::UEngine* Engine = SDK::UEngine::GetEngine();
    SDK::UWorld* World = SDK::UWorld::GetWorld();

    if (!Engine || !World || !World->OwningGameInstance || World->OwningGameInstance->LocalPlayers.Num() == 0)
    {
        std::cout << "Engine or World not found!" << std::endl;
        return 0;
    }

    SDK::APlayerController* MyController = World->OwningGameInstance->LocalPlayers[0]->PlayerController;
    if (!MyController)
    {
        std::cout << "PlayerController not found!" << std::endl;
        return 0;
    }

    /* Example buffs for local player */
    MyController->FOV(150.f);

    auto comp = MyController->Character ? MyController->Character->CharacterMovement : nullptr;
    if (comp)
    {
        comp->MaxAcceleration = 1000.0f;
        comp->MaxWalkSpeed = 1000.0f;
    }

    /* Find a base material */
    SDK::UMaterial* BaseMaterial = nullptr;
    for (int i = 0; i < SDK::UObject::GObjects->Num(); i++)
    {
        SDK::UObject* Obj = SDK::UObject::GObjects->GetByIndex(i);
        if (!Obj || Obj->IsDefaultObject())
            continue;

        if (Obj->IsA(SDK::UMaterial::StaticClass()))
        {
            SDK::UMaterial* Material = static_cast<SDK::UMaterial*>(Obj);
            std::string MaterialName = Material->GetName();

            if (MaterialName.find("Ghost") != std::string::npos ||
                MaterialName.find("UI") != std::string::npos ||
                MaterialName.find("Translucent") != std::string::npos)
            {
                BaseMaterial = Material;
                std::cout << "Using base material: " << Material->GetFullName() << std::endl;
                break;
            }
        }
    }

    /* Fallback if none matched */
    if (!BaseMaterial)
    {
        for (int i = 0; i < SDK::UObject::GObjects->Num(); i++)
        {
            SDK::UObject* Obj = SDK::UObject::GObjects->GetByIndex(i);
            if (Obj && Obj->IsA(SDK::UMaterial::StaticClass()) && !Obj->IsDefaultObject())
            {
                BaseMaterial = static_cast<SDK::UMaterial*>(Obj);
                std::cout << "Using fallback material: " << BaseMaterial->GetFullName() << std::endl;
                break;
            }
        }
    }

    /* Create dynamic material instance */
    SDK::UMaterialInstanceDynamic* DynamicMat = nullptr;
    if (BaseMaterial)
    {
        DynamicMat = SDK::UKismetMaterialLibrary::CreateDynamicMaterialInstance(
            World, BaseMaterial, SDK::FName(), SDK::EMIDCreationFlags::None);

        if (DynamicMat)
        {
            DynamicMat->SetScalarParameterValue(SDK::UKismetStringLibrary::Conv_StringToName(L"DisableDepthTest"), 1.0f);
            DynamicMat->SetScalarParameterValue(SDK::UKismetStringLibrary::Conv_StringToName(L"Opacity"), 0.5f);
            DynamicMat->SetVectorParameterValue(SDK::UKismetStringLibrary::Conv_StringToName(L"Color"), { 1.0f, 0.0f, 0.0f, 1.0f }); // Red
            DynamicMat->SetVectorParameterValue(SDK::UKismetStringLibrary::Conv_StringToName(L"EmissiveColor"), { 1.0f, 0.0f, 0.0f, 1.0f });

            std::cout << "Dynamic material created successfully!" << std::endl;
        }
    }

    /* Apply to ALL actors with a mesh component */
    if (DynamicMat)
    {
        SDK::ULevel* Level = World->PersistentLevel;
        if (Level)
        {
            SDK::TArray<SDK::AActor*>& Actors = Level->Actors;

            for (SDK::AActor* Actor : Actors)
            {
                if (!Actor)
                    continue;

                // Iterate over all components
                auto& Components = Actor->GetComponents();
                for (SDK::UActorComponent* Comp : Components)
                {
                    if (!Comp) continue;

                    if (auto* MeshComp = dynamic_cast<SDK::UMeshComponent*>(Comp))
                    {
                        MeshComp->SetMaterial(0, DynamicMat);
                        std::cout << "Applied wallhack material to: " << Actor->GetFullName() << std::endl;
                    }
                }
            }
        }
    }

    /* Change console key */
    SDK::UInputSettings::GetDefaultObj()->ConsoleKeys[0].KeyName =
        SDK::UKismetStringLibrary::Conv_StringToName(L"F2");

    /* Spawn a new console */
    SDK::UObject* NewObject = SDK::UGameplayStatics::SpawnObject(Engine->ConsoleClass, Engine->GameViewport);
    if (NewObject)
        Engine->GameViewport->ViewportConsole = static_cast<SDK::UConsole*>(NewObject);

    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved)
{
    switch (reason)
    {
    case DLL_PROCESS_ATTACH:
        CreateThread(0, 0, (LPTHREAD_START_ROUTINE)MainThread, hModule, 0, 0);
        break;
    }
    return TRUE;
}
