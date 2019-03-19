using System.Collections;
using System.Collections.Generic;
using UnityEditor;
using UnityEngine;
using System;
using System.Text;
using System.Diagnostics;
using UniLua.Tools;

public class MagnaChainEditor : Editor
{
    //[MenuItem("MagnaChain/CompileSelectedContract")]
    //public static void ComplieSelectedContract()
    //{
    //    UnityEngine.Debug.Log("<color=aqua>[ComplieSelectedContract begin---------------------------------------------------------------------------------------------------]</color>");

    //    int i;
    //    string strPath;

    //    if (Selection.objects.Length == 0)
    //    {
    //        EditorUtility.DisplayDialog("Prompt", "Please select the special lua files or folders that need to be processed in the right window of Project.", "OK");
    //        return;
    //    }

    //    List<string> listLua = new List<string>();
    //    for (i = 0; i < Selection.objects.Length; i++)
    //    {
    //        strPath = AssetDatabase.GetAssetPath(Selection.objects[i]);
    //        ProcessPath(strPath, listLua);
    //    }

    //    ImplCompileOrCheckLua(listLua);
    //    UnityEngine.Debug.Log("<color=aqua>[ComplieSelectedContract end-----------------------------------------------------------------------------------------------------]</color>");
    //}

    //[MenuItem("MagnaChain/CompileAllContract")]
    //public static void ComplieAllContract()
    //{
    //    UnityEngine.Debug.Log("<color=aqua>[ComplieAllContract begin---------------------------------------------------------------------------------------------------]</color>");
    //    List<string> listLua = new List<string>();
    //    ProcessPath("Assets/MagnaChain/Contract/Src", listLua);

    //    ImplCompileOrCheckLua(listLua);
    //    UnityEngine.Debug.Log("<color=aqua>[ComplieAllContract end-----------------------------------------------------------------------------------------------------]</color>");
    //}

    [MenuItem("MagnaChain/CheckSelectedContract")]
    public static void CheckSelectedContract()
    {
        UnityEngine.Debug.Log("<color=aqua>[CheckSelectedContract begin---------------------------------------------------------------------------------------------------]</color>");
        int i;
        string strPath;

        if (Selection.objects.Length == 0)
        {
            EditorUtility.DisplayDialog("Prompt", "Please select the special lua files or folders that need to be processed in the right window of Project.", "OK");
            return;
        }

        List<string> listLua = new List<string>();
        for (i = 0; i < Selection.objects.Length; i++)
        {
            strPath = AssetDatabase.GetAssetPath(Selection.objects[i]);
            ProcessPath(strPath, listLua);
        }

        ImplCompileOrCheckLua(listLua, true);
        UnityEngine.Debug.Log("<color=aqua>[CheckSelectedContract end-----------------------------------------------------------------------------------------------------]</color>");
    }

    [MenuItem("MagnaChain/CheckAllContract")]
    public static void CheckAllContract()
    {
        UnityEngine.Debug.Log("<color=aqua>[CheckAllContract begin---------------------------------------------------------------------------------------------------]</color>");
        List<string> listLua = new List<string>();
        ProcessPath("Assets/MagnaChain/Contract/Src", listLua);

        ImplCompileOrCheckLua(listLua, true);
        UnityEngine.Debug.Log("<color=aqua>[CheckAllContract end-----------------------------------------------------------------------------------------------------]</color>");
    }

    private static void ImplCompileOrCheckLua(List<string> listLua, bool bCheck = false)
    {
        if (listLua.Count == 0)
        {
            EditorUtility.DisplayDialog("Prompt", "You have not select valid lua file, please select the special lua files or folders that need to be processed in the right window of Project.", "OK");
            return;
        }

        int i;
        string strBytecode;   
        string strCompileError;

        int iSuccNum = 0;
        int iFailedNum = 0;

        float fPercent;

        int iBeginTime = Environment.TickCount;
        EditorUtility.DisplayProgressBar("Progress", "Compiling lua, please wait...", 0.0f);
        for (i = 0; i < listLua.Count; i++)
        {
            // luac -o out.lua 1.lua 
            strBytecode = listLua[i].Replace("/Src/", "/Bytecode/");
            strBytecode = strBytecode.Replace(".lua", ".clx");
            //strArg = "\"-o " + strBytecode + " " + listLua[i] + "\"";
            //if ( CompileLua(strWorkPath, strExe, listLua[i], strBytecode, out strCompileError, out strExeError) ==false )
            if ( bCheck == false)
            {
                try
                {
                    if (LuaCompiler.DumpingToFile(listLua[i], strBytecode, true, out strCompileError) == false)
                    {
                        iFailedNum += 1;
                        if (!string.IsNullOrEmpty(strCompileError))
                        {
                            //UnityEngine.Debug.LogError("Compile failed: " + listLua[i]);
                            UnityEngine.Debug.LogError("<color=red>[E]: " + strCompileError + "</color>");
                        }
                    }
                    else
                    {
                        iSuccNum += 1;
                        UnityEngine.Debug.Log("<color=#98FB98>[O]: " + listLua[i] + "</color>");
                    }
                }
                catch (Exception e)
                {
                    iFailedNum += 1;

                    //UnityEngine.Debug.LogError("Compile failed: " + listLua[i]);
                    UnityEngine.Debug.LogError("<color=red>[E]: " + listLua[i] + " " + e.Message + "</color>");
                }

                fPercent = (float)(i + 1) / listLua.Count;
                EditorUtility.DisplayProgressBar("Progress", "Compiling lua, please wait...", fPercent);
            }
            else
            {
                try
                {
                    LuaCompiler.CompileFile(listLua[i], out strCompileError);
                    if ( !string.IsNullOrEmpty(strCompileError))
                    {
                        iFailedNum += 1;
                        if (!string.IsNullOrEmpty(strCompileError))
                        {
                            //UnityEngine.Debug.LogError("Compile failed: " + listLua[i]);
                            UnityEngine.Debug.LogError("<color=red>[E]: " + strCompileError + "</color>");
                        }
                    }
                    else
                    {
                        iSuccNum += 1;
                        UnityEngine.Debug.Log("<color=#98FB98>[O]: " + listLua[i] + "</color>");
                    }
                }
                catch (Exception e)
                {
                    iFailedNum += 1;

                    //UnityEngine.Debug.LogError("Compile failed: " + listLua[i]);
                    UnityEngine.Debug.LogError("<color=#98FB98>[E]: " + listLua[i] + " " + e.Message + "</color>");
                }

                fPercent = (float)(i + 1) / listLua.Count;
                EditorUtility.DisplayProgressBar("Progress", "Check lua, please wait...", fPercent);
            }
        }
        EditorUtility.ClearProgressBar();
        int iEndTime = Environment.TickCount;

        float fCostTime = (float)(iEndTime - iBeginTime) * 0.001f;
        string strK = string.Format("The job has finish, cost time: {0}(s)\n\nSucceed: {1}\nFailed: {2}\n\nPlease see the console for detail", fCostTime.ToString("f2"), iSuccNum, iFailedNum);

        AssetDatabase.Refresh();
        EditorUtility.DisplayDialog("Finish", strK, "OK");
    }

    private static void ProcessPath(string strPath, List<string> listLua)
    {
        string strPrefex = Application.dataPath;
        strPrefex = strPrefex.Replace("Assets", "");
        string strFullPath = strPrefex + strPath;
        if ( FileFunc.IsExistDirectory(strFullPath))
        {
            int i;

            string[] arrFiles = FileFunc.GetFileNames(strFullPath, "*.lua", true);
            for ( i = 0; i < arrFiles.Length; i++ )
            {
                arrFiles[i] = arrFiles[i].Replace("\\", "/");
            }
            listLua.AddRange(arrFiles);
        }
        else
        {
            if ( strFullPath.LastIndexOf(".lua") != -1 || strFullPath.LastIndexOf(".Lua") != -1
                || strFullPath.LastIndexOf(".LUA") != -1 )
            {
                strFullPath = strFullPath.Replace("\\", "/");
                listLua.Add(strFullPath);
            }
        }        
    }   
    
    //private static bool CompileLua(string strWorkPath, string strExe, string strInputFile, string strOutputFile, out string strCompileError, out string strExeError)
    //{
    //    System.Diagnostics.Process p = new System.Diagnostics.Process();        
    //    p.StartInfo.FileName = "cmd.exe";        
    //    p.StartInfo.WorkingDirectory = strWorkPath;
    //    p.StartInfo.UseShellExecute = false;    
    //    p.StartInfo.RedirectStandardInput = true;
    //    p.StartInfo.RedirectStandardOutput = true;
    //    p.StartInfo.StandardOutputEncoding = Encoding.Default;
    //    p.StartInfo.RedirectStandardError = true;
    //    p.StartInfo.StandardErrorEncoding = Encoding.Default;
    //    p.StartInfo.CreateNoWindow = true;
    //    p.StartInfo.WindowStyle = System.Diagnostics.ProcessWindowStyle.Normal;
    //    p.Start();

    //    strCompileError = null;
    //    strExeError = null;

    //    p.OutputDataReceived += new DataReceivedEventHandler(OnOutputReceive);
    //    p.BeginOutputReadLine();

    //    p.ErrorDataReceived += new DataReceivedEventHandler(OnErrorReceive);
    //    p.BeginErrorReadLine();

    //    p.StandardInput.AutoFlush = true;

    //    // discard some fuck char        
    //    p.StandardInput.WriteLine("\r");
    //    //p.StandardInput.Flush();
    //    p.StandardError.DiscardBufferedData();
    //    p.StandardOutput.DiscardBufferedData();
    //    //string strK = p.StandardOutput.ReadToEnd();
    //    //string strTError = p.StandardError.ReadToEnd();

    //    // check first
    //    string strCmd;        

    //    strCmd = strExe + " -p " + strInputFile;
    //    p.StandardInput.WriteLine(strCmd);

    //    //strCompileError = p.StandardError.ReadToEnd();
    //    //if (!string.IsNullOrEmpty(strCompileError))
    //    //{
    //    //    return false;
    //    //}

    //    strCmd = strExe + " -o " + strOutputFile + " " + strInputFile;          
    //    p.StandardInput.WriteLine(strCmd);

    //    //string strInfo = p.StandardOutput.ReadToEnd();       

    //    //strExeError = p.StandardError.ReadToEnd();
    //    //if (!string.IsNullOrEmpty(strExeError))
    //    //{
    //    //    return false;
    //    //}

    //    p.WaitForExit();
    //    p.Close();  
    //    return true;
    //}

    //private static void OnOutputReceive(object Sender, DataReceivedEventArgs e)
    //{
    //    if (e.Data != null)
    //    {
    //        UnityEngine.Debug.Log(e.Data);
    //    }
    //}

    //private static void OnErrorReceive(object Sender, DataReceivedEventArgs e)
    //{
    //    if (e.Data != null)
    //    {
    //        UnityEngine.Debug.LogError(e.Data);
    //    }
    //}
}
