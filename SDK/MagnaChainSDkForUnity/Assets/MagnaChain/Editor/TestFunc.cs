using UnityEngine;
using System.Collections;
using UnityEditor;

public class TestFunc : Editor
{
    [MenuItem("TestFunc/TestCompile")]
    static void TestCompile()
    {
        //UniLua.Tools.Compiler.CompileFile("C:/aa.lua");

        string strError;

        UniLua.Tools.LuaCompiler.DumpingToFile("G:/TestError0.lua", "G:/TestError0.clx", true, out strError);
        if ( !string.IsNullOrEmpty(strError))
        {
            Debug.LogError(strError);            
        }
    }
}
