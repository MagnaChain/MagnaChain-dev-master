using System;
using UnityEngine;

namespace MagnaChainEx
{
    public class Logger
    {
        public static bool ENABLE_NORMAL = true;
        public static bool ENABLE_WARNING = false;
        public static bool ENABLE_ERROR = true;
        public static bool ENABLE_SYS = true;

        public static bool LOG_TO_CONSOLE_VIEW = true;

        public static LogRecord ms_kRec = new LogRecord();

        //private static bool ms_bEditor = CheckEditor();

        //private static bool CheckEditor()
        //{d
        //    if (UnityEngine.Application.platform == UnityEngine.RuntimePlatform.WindowsEditor || UnityEngine.Application.platform == UnityEngine.RuntimePlatform.OSXEditor)
        //    {
        //        return true;
        //    }
        //    else
        //    {
        //        return false;
        //    }
        //}

        public static void Log(string msg)
        {

#if UNITY_EDITOR || (UNITY_IOS && ENABLE_NORMAL)
            UnityEngine.Debug.Log(msg);
#else
        if (ENABLE_NORMAL)
        {
            ms_kRec.Log(msg);                        
        }        
#endif

            //if (ENABLE_NORMAL && LOG_TO_CONSOLE_VIEW && IxDebugConsole.ms_kSig != null)
            //{
            //    IxDebugConsole.ms_kSig.TextOutNormal(msg);
            //}
        }

        public static void LogError(string msg, bool bStackInfo = true, bool bNoLogToConsole = false)
        {
#if UNITY_EDITOR || (UNITY_IOS && ENABLE_ERROR)
            UnityEngine.Debug.LogError(msg);
#else
        if (ENABLE_ERROR)
        {            
            ms_kRec.LogError(msg);
            if (bStackInfo)
            {
                string strStack = StackTraceUtility.ExtractStackTrace();
                ms_kRec.LogError(strStack);
            }                        
        }        
#endif

            //if ( bNoLogToConsole == false)
            //{
            //    if (ENABLE_ERROR && LOG_TO_CONSOLE_VIEW && IxDebugConsole.ms_kSig != null)
            //    {
            //        IxDebugConsole.ms_kSig.TextOutError(msg);
            //        //if (bStackInfo)
            //        //{
            //        //    IxDebugConsole.ms_kSig.TextOutError(strStack);
            //        //}
            //    }
            //}        
        }

        public static void LogError(string msg, Exception ex)
        {
            if (ENABLE_ERROR)
            {
                LogError(msg + "\nError: " + ex.Message + "\n" + ex.StackTrace, false, false);
            }

            //#if !UNITY_EDITOR
            //        if (IxDebugConsole.ms_kSig != null)
            //        {
            //            IxDebugConsole.ms_kSig.TextOutError(msg);            
            //        }
            //#endif

            //if (ENABLE_ERROR && LOG_TO_CONSOLE_VIEW && IxDebugConsole.ms_kSig != null)
            //{
            //    IxDebugConsole.ms_kSig.TextOutError(msg);
            //}
        }

        public static void LogWarn(string msg)
        {
#if UNITY_EDITOR || (UNITY_IOS && ENABLE_WARNING)
            UnityEngine.Debug.LogWarning(msg);
#else
        if (ENABLE_WARNING)
        {            
            ms_kRec.LogWarn(msg);
        }        
#endif

            //if (ENABLE_WARNING && LOG_TO_CONSOLE_VIEW && IxDebugConsole.ms_kSig != null)
            //{
            //    IxDebugConsole.ms_kSig.TextOutWarning(msg);
            //}
        }

        public static void LogSys(string msg, bool bStackInfo = false)
        {
#if UNITY_EDITOR || (UNITY_IOS && ENABLE_SYS)
            UnityEngine.Debug.Log(msg);
#else
        if (ENABLE_SYS)
        {
            ms_kRec.LogSys(msg);
        
            if (bStackInfo)
            {
                string strStack = StackTraceUtility.ExtractStackTrace();
                ms_kRec.LogSys(strStack);
            }  
        }         
#endif

            //if (ENABLE_SYS && LOG_TO_CONSOLE_VIEW && IxDebugConsole.ms_kSig != null)
            //{
            //    IxDebugConsole.ms_kSig.TextOutSys(msg);
            //}
        }
    }
}

