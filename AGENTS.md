# TexasRaider (DeadMaze) - Agent Guidelines

## Project Overview

**TexasRaider** is a wild west-themed Pacman/Nethack-inspired maze crawler built with Unreal Engine 5.6.

- **Engine:** Unreal Engine 5.6
- **Language:** C++ (with Blueprint integration)
- **Module:** DeadMaze (Runtime)
- **Key Systems:** Procedural level generation, enemy AI, room-based maze navigation

## Build & Development Commands

### Building the Project

```bash
# Generate Visual Studio project files (Windows)
# Right-click DeadMaze.uproject → "Generate Visual Studio project files"
# Or use UnrealBuildTool directly

# Build Development Editor (from project root)
"C:\Program Files\Epic Games\UE_5.6\Engine\Build\BatchFiles\Build.bat" DeadMazeEditor Win64 Development "C:\Unreal\TexasRaider\DeadMaze.uproject"

# Build Development Game (standalone)
"C:\Program Files\Epic Games\UE_5.6\Engine\Build\BatchFiles\Build.bat" DeadMaze Win64 Development "C:\Unreal\TexasRaider\DeadMaze.uproject"
```

### Cleaning Build Artifacts

```bash
# Clean intermediate files
rm -rf Binaries/ Intermediate/ DerivedDataCache/ Saved/

# Regenerate project files after cleaning
# Right-click DeadMaze.uproject → "Generate Visual Studio project files"
```

### Running the Project

- **Editor:** Double-click `DeadMaze.uproject` or launch from Epic Games Launcher
- **Standalone:** Run from Visual Studio (Launch configuration) or build executable in `Binaries/Win64/`

## Testing

### Automated Tests

Unreal Engine uses the Automation Testing framework. Tests are typically in:
- `Source/DeadMaze/Tests/` (if created)

```bash
# Run all automation tests (from Unreal Editor)
# Window → Test Automation → Select tests → Run

# Command-line test execution
UnrealEditor.exe "C:\Unreal\TexasRaider\DeadMaze.uproject" -ExecCmds="Automation RunTests DeadMaze" -unattended -nopause -NullRHI -log
```

### Running a Single Test

```bash
# Run specific test by name
UnrealEditor.exe "C:\Unreal\TexasRaider\DeadMaze.uproject" -ExecCmds="Automation RunTests YourTestName" -unattended -nopause -NullRHI -log
```

## Project Structure

```
DeadMaze/
├── Binaries/           # Compiled executables (gitignored)
├── Config/             # Project configuration (.ini files)
├── Content/            # Assets, blueprints, maps
├── DerivedDataCache/   # Cache files (gitignored)
├── Intermediate/       # Build intermediates (gitignored)
├── Saved/              # Logs, autosaves (gitignored)
└── Source/
    ├── DeadMaze/       # Main game module
    │   ├── Public/     # Public headers (.h)
    │   ├── Private/    # Implementation files (.cpp)
    │   └── DeadMaze.Build.cs
    ├── DeadMaze.Target.cs
    └── DeadMazeEditor.Target.cs
```

## Code Style Guidelines

### Naming Conventions

**Classes & Structs:**
- Classes: PascalCase with Unreal prefix (`AActor`, `UObject`, `FStruct`, `EEnum`, `TTemplate`)
  - `A` = Actor-derived classes
  - `U` = UObject-derived classes
  - `F` = Plain C++ structs/classes
  - `E` = Enums
  - `T` = Templates
- Example: `ALevelGeneration`, `EEnemyActionState`, `FVector`

**Variables:**
- Local variables: camelCase (`keysPlaced`, `currentNode`)
- Member variables: `m_` prefix + camelCase (`m_roomType`, `m_roomMesh`, `m_roomWorldLocation`)
- Constants: UPPER_CASE or PascalCase with `const` (`MAZESIZE`, `WORLDLOCATIONOFFSET`)
- Booleans: Use `b` prefix for UPROPERTY (`bCanEverTick`)

**Functions:**
- PascalCase for public/protected methods (`BeginPlay()`, `GenerateGrid()`, `CreateRooms()`)
- camelCase for private/local lambdas

### File Organization

- **Headers (.h):** Place in `Source/DeadMaze/Public/` for public APIs
- **Implementation (.cpp):** Place in `Source/DeadMaze/Private/`
- **One class per file** (unless small helper structs/enums)
- **File naming:** Match the primary class name (`LevelGeneration.h`, `LevelGeneration.cpp`)

### Header Structure

```cpp
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
// ... other includes
#include "ClassName.generated.h"  // ALWAYS LAST for UCLASS

UCLASS()
class DEADMAZE_API AClassName : public AActor
{
    GENERATED_BODY()
    
public:
    // Public interface
    
protected:
    // Protected members
    
private:
    // Private members
};
```

### Includes & Dependencies

- **CoreMinimal.h:** Required first include in most files
- **Generated.h:** MUST be last include in header files (e.g., `LevelGeneration.generated.h`)
- **Prefer forward declarations** in headers when possible
- **Group includes:** Engine includes, then project includes
- **Add module dependencies** to `DeadMaze.Build.cs` as needed

### UPROPERTY & Specifiers

```cpp
// EditAnywhere: Editable in editor and per-instance
UPROPERTY(EditAnywhere, Category = "Level Generation")
TSubclassOf<AActor> WallClass;

// BlueprintReadWrite: Accessible in Blueprints
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action State")
EEnemyActionState actionState;

// Common specifiers: EditDefaultsOnly, BlueprintReadOnly, VisibleAnywhere
```

### Types & Const Correctness

- **Use Unreal types:** `int32`, `float`, `FVector`, `FString`, `TArray`, `TMap`
- **STL is allowed:** This project uses `std::vector`, `std::queue`, `std::shared_ptr`
- **Const correctness:** Mark read-only methods as `const`, use `const&` for parameters
- **Auto keyword:** Acceptable for complex iterators/lambdas (e.g., structured bindings)

### Comments & Documentation

- **Copyright header:** Use standard Unreal comment at top of files
- **TODO comments:** Mark incomplete/temporary code
- **Inline comments:** Explain "why" not "what" for complex logic
- **Minimize commented-out code:** Remove dead code; use version control

### Error Handling & Logging

```cpp
// Logging (common categories: LogTemp, LogInit)
UE_LOG(LogTemp, Warning, TEXT("Spawning key at (%d, %d)"), x, y);
UE_LOG(LogTemp, Error, TEXT("Failed to load asset: %s"), *AssetPath);

// Check for valid pointers before use
if (WallClass)
{
    GetWorld()->SpawnActor<AActor>(WallClass, Location, Rotation);
}

// Assertions for development builds
check(MyPointer != nullptr);
checkf(Condition, TEXT("Error message: %s"), *Details);
```

### Formatting

- **Indentation:** Tabs (observed in existing code)
- **Braces:** Opening brace on same line for functions, new line acceptable for control flow
- **Line length:** Keep reasonable (~120 chars), but no strict limit
- **Trailing whitespace:** Remove

## Common Patterns

### Actor Spawning

```cpp
AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(
    ActorClass, 
    SpawnLocation, 
    SpawnRotation
);
```

### Accessing Components

```cpp
UStaticMeshComponent* MeshComp = FindComponentByClass<UStaticMeshComponent>();
```

### Iterating Unreal Containers

```cpp
for (const FVector& Point : PathPoints)
{
    // Process point
}
```

## Important Notes

- **Platform:** Project is Windows-based; paths use backslashes in absolute form
- **Engine version:** 5.6 - ensure compatible API usage
- **PCH:** Project uses explicit/shared PCHs (`PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs`)
- **Hot reload:** Limited in UE5; prefer full recompilation for major changes
- **Blueprints:** C++ classes can be extended in Blueprints; use UPROPERTY/UFUNCTION appropriately

---

*This document is intended for AI coding agents working on the TexasRaider project.*
