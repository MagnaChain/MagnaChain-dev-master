using System;
using System.IO;
using UnityEngine;

namespace MagnaChainEx
{
    public class LogRecord
    {
        private FileStream mFile;
        private BinaryWriter mFileWriter;

        public LogRecord()
        {
            if (Application.platform == RuntimePlatform.Android)
            {
                DateTime now = DateTime.Now;
                var dir = Application.persistentDataPath + "/MagnaChain_logs/";
                if (!Directory.Exists(dir))
                {
                    Directory.CreateDirectory(dir);
                }
                //FNSdk.SetLogPathInAndroid(dir + "gamelog_" + now.ToString("yyyy-MM-dd") + ".log");
                this.mFile = new FileStream(dir + "log_" + now.ToString("yyyy-MM-dd") + ".log", FileMode.Append);
                this.mFileWriter = new BinaryWriter(this.mFile);
            }
            else if (Application.platform == RuntimePlatform.WindowsPlayer)
            {
                DateTime now = DateTime.Now;
                var dir = Application.dataPath + "/logs/";
                if (!Directory.Exists(dir))
                {
                    Directory.CreateDirectory(dir);
                }
                this.mFile = new FileStream(dir + "log_" + now.ToString("yyyy-MM-dd") + ".log", FileMode.Append);
                this.mFileWriter = new BinaryWriter(this.mFile);
            }
        }

        ~LogRecord()
        {
            Dispose();
        }

        public void Dispose()
        {
            if (this.mFileWriter != null)
            {
                this.mFileWriter.Close();
                this.mFileWriter = null;
            }
            if (this.mFile != null)
            {
                this.mFile.Close();
                this.mFile = null;
            }
        }

        public void Log(string msg)
        {
            if (mFileWriter != null)
            {
                //string[] formatStrs = new string[] { "[INFO：", DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss"), "]", msg, "\r\n" };
                string[] formatStrs = new string[] { "[INFO：", DateTime.Now.ToString("HH:mm:ss"), "]", msg, "\r\n" };
                this.mFileWriter.Write(string.Concat(formatStrs).ToCharArray());
                this.mFileWriter.Flush();
            }
        }

        public void LogWarn(string msg)
        {
            if (mFileWriter != null)
            {
                //string[] formatStrs = new string[] { "[WARN：", DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss"), "]", msg, "\r\n" };
                string[] formatStrs = new string[] { "[WARN：", DateTime.Now.ToString("HH:mm:ss"), "]", msg, "\r\n" };
                this.mFileWriter.Write(string.Concat(formatStrs).ToCharArray());
                this.mFileWriter.Flush();
            }
        }

        public void LogError(string msg)
        {
            if (mFileWriter != null)
            {
                //string[] formatStrs = new string[] { "[ERROR：", DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss"), "]", msg, "\r\n" };
                string[] formatStrs = new string[] { "[ERROR：", DateTime.Now.ToString("HH:mm:ss"), "]", msg, "\r\n" };
                this.mFileWriter.Write(string.Concat(formatStrs).ToCharArray());
                this.mFileWriter.Flush();
            }
        }

        public void LogSys(string msg)
        {
            if (mFileWriter != null)
            {
                //string[] formatStrs = new string[] { "[SYS：", DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss"), "]", msg, "\r\n" };
                string[] formatStrs = new string[] { "[SYS：", DateTime.Now.ToString("HH:mm:ss"), "]", msg, "\r\n" };
                this.mFileWriter.Write(string.Concat(formatStrs).ToCharArray());
                this.mFileWriter.Flush();
            }
        }
    }
}


