// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;
using System;
using System.IO;

public class MagnaChainForUnreal : ModuleRules
{
    public MagnaChainForUnreal(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore" });

        PrivateDependencyModuleNames.AddRange(new string[] { });

        // Uncomment if you are using Slate UI
        // PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        // Uncomment if you are using online features
        // PrivateDependencyModuleNames.Add("OnlineSubsystem");

        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
        LoadMagnaChain(Target);
    }

    public bool LoadMagnaChain(ReadOnlyTargetRules Target)
    {
        Console.WriteLine("ModulePath: " + ModulePath);
        Console.WriteLine("ThirdPartyPath: " + ThirdPartyPath);

        bool isLibrarySupported = false;
        if ((Target.Platform == UnrealTargetPlatform.Win64) || (Target.Platform == UnrealTargetPlatform.Win32))
        {
            isLibrarySupported = true;
            string PlatformString = (Target.Platform == UnrealTargetPlatform.Win64) ? "x64" : "x86";
            string LibrariesPath = Path.Combine(ThirdPartyPath, "MagnaChain", "Library");

            Console.WriteLine("LibrariesPath: " + LibrariesPath);

            string addtiveLib = Path.Combine(LibrariesPath, "magnachain-sdk." + PlatformString + ".lib");
            Console.WriteLine("AddtiveLibary: " + addtiveLib);
            PublicAdditionalLibraries.Add(addtiveLib);
        }
        if (isLibrarySupported)
        {
            // Include path
            string strIncPath = Path.Combine(ThirdPartyPath, "MagnaChain", "Include");
            Console.WriteLine("Include Path: " + strIncPath);
            PublicIncludePaths.Add(strIncPath);
        }
        Definitions.Add(string.Format("WITH_MAGNA_CHAIN_BINDING=£û0£ý", isLibrarySupported ? 1 : 0));
        return isLibrarySupported;
    }

    private string ModulePath
    {
        get
        {
            return ModuleDirectory;
        }
    }

    private string ThirdPartyPath
    {
        get
        {
            return Path.GetFullPath(Path.Combine(ModulePath, "../../ThirdParty"));
        }
    }
}
