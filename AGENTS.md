# Repository Guidelines

## Project Structure & Module Organization
- Source/InfinityFighter/Public: Public headers (module API)
- Source/InfinityFighter/Private: C++ implementations and internals
- Source/*.Target.cs, InfinityFighter.Build.cs: Build targets and module rules
- Content/: Assets (Blueprints, Maps, Input, DataAssets)
- Config/: Engine, game, editor, and input settings
- Tests/ (optional): Automation/Functional tests if added

## Build, Test, and Development Commands
- Open in Editor: UnrealEditor.exe .\InfinityFighter.uproject
- Visual Studio: open InfinityFighter.sln, build target InfinityFighterEditor (Win64, Development)
- CLI build (Windows): UE_PATH\Engine\Build\BatchFiles\Build.bat InfinityFighterEditor Win64 Development -Project=REPO_PATH\InfinityFighter.uproject -WaitMutex -FromMsBuild
- Run game: UE_PATH\Engine\Binaries\Win64\UnrealEditor.exe .\InfinityFighter.uproject -game
- Automation tests (CLI): UnrealEditor-Cmd.exe .\InfinityFighter.uproject -ExecCmds=Automation RunTests InfinityFighter -unattended -nop4 -TestExit=Automation Test Queue Empty

## Coding Style & Naming Conventions
- Unreal style: 4 spaces, #pragma once, one class per ClassName.h/.cpp
- Prefixes: A Actor, U UObject/Component, F Struct, E Enum, I Interface; booleans start with b
- PascalCase for types/functions; variables camelCase; filenames match class (e.g., CharacterActionStatComponent.h/.cpp)
- Keep public API in Public and internals in Private; use UCLASS/USTRUCT/UFUNCTION macros and UPROPERTY metadata consistently
- Folder namespace: Source/InfinityFighter/<Domain>/... (e.g., Component, Router, DA, Base)

## Testing Guidelines
- Prefer Automation Tests (IMPLEMENT_SIMPLE_AUTOMATION_TEST or DEFINE_SPEC) under Source/InfinityFighter/Private/Tests
- Name tests *Tests.cpp or *Spec.cpp; test names Project.Feature.Scenario
- Run via Editor Session Frontend Automation or the CLI command above

## Commit & Pull Request Guidelines
- Branch prefixes: feat/, fix/, chore/, docs/, test/
- Commit message: type(scope): summary (example: fix(skill): null check in AddSkill)
- PRs: include change summary, linked issues, screenshots or video for asset/behavior changes, and test results; list touched paths and any Config edits

## Repo Hygiene & Assets
- Do not commit DerivedDataCache/, Intermediate/, or Saved/; prefer Git LFS for large .uasset/.umap files
- Keep Input and DataAssets organized under existing Content folders; avoid ad-hoc directories
