
using System;
using System.IO;
using System.Text;
using System.Collections.Generic;
using UnityEngine;

public class FileFunc
{
    /// <summary>
    /// 换行符
    /// </summary>
    public static string NEW_LINE = "\r\n";
    #region 检测指定目录是否存在
    /// <summary> 
    /// 检测指定目录是否存在 
    /// </summary> 
    /// <param name="directoryPath">目录的绝对路径</param>         
    public static bool IsExistDirectory(string directoryPath)
    {
#if !UNITY_WEBPLAYER
        return Directory.Exists(directoryPath);
#else
		return false;
#endif
    }
    #endregion

    #region 检测指定文件是否存在
    /// <summary> 
    /// 检测指定文件是否存在,如果存在则返回true。 
    /// </summary> 
    /// <param name="filePath">文件的绝对路径</param>         
    public static bool IsExistFile(string filePath)
    {
		#if !UNITY_WEBPLAYER
        return File.Exists(filePath);
#else
		return false;
#endif
    }
    #endregion

    #region 检测指定目录是否为空
    /// <summary> 
    /// 检测指定目录是否为空 
    /// </summary> 
    /// <param name="directoryPath">指定目录的绝对路径</param>         
    public static bool IsEmptyDirectory(string directoryPath)
    {
        try
        {
			#if !UNITY_WEBPLAYER
            //判断是否存在文件 
            string[] fileNames = GetFileNames(directoryPath);
            if (fileNames.Length > 0)
            {
                return false;
            }
            //判断是否存在文件夹 
            string[] directoryNames = GetDirectories(directoryPath);
            if (directoryNames.Length > 0)
            {
                return false;
            }
			#endif
            return true;
        }
        catch (Exception ex)
        {
            Debug.LogError(ex.Message);
            return true;
        }
    }
    #endregion

    #region 检测指定目录中是否存在指定的文件
    /// <summary> 
    /// 检测指定目录中是否存在指定的文件,若要搜索子目录请使用重载方法. 
    /// </summary> 
    /// <param name="directoryPath">指定目录的绝对路径</param> 
    /// <param name="searchPattern">模式字符串，"*"代表0或N个字符，"?"代表1个字符。 
    /// 范例："Log*.xml"表示搜索所有以Log开头的Xml文件。</param>         
    public static bool Contains(string directoryPath, string searchPattern)
    {
        try
        {
            //获取指定的文件列表 
            string[] fileNames = GetFileNames(directoryPath, searchPattern, false);
            //判断指定文件是否存在 
            if (fileNames.Length == 0)
            {
                return false;
            }
            else
            {
                return true;
            }
        }
        catch (Exception ex)
        {
            Debug.LogError(ex.Message);
            return false;
        }
    }
    /// <summary> 
    /// 检测指定目录中是否存在指定的文件 
    /// </summary> 
    /// <param name="directoryPath">指定目录的绝对路径</param> 
    /// <param name="searchPattern">模式字符串，"*"代表0或N个字符，"?"代表1个字符。 
    /// 范例："Log*.xml"表示搜索所有以Log开头的Xml文件。</param>  
    /// <param name="isSearchChild">是否搜索子目录</param> 
    public static bool Contains(string directoryPath, string searchPattern, bool isSearchChild)
    {
        try
        {
            //获取指定的文件列表 
            string[] fileNames = GetFileNames(directoryPath, searchPattern, true);
            //判断指定文件是否存在 
            if (fileNames.Length == 0)
            {
                return false;
            }
            else
            {
                return true;
            }
        }
        catch (Exception ex)
        {
            Debug.LogError(ex.Message);
            return false;
        }
    }
    #endregion

    #region 创建一个目录
    /// <summary> 
    /// 创建一个目录 
    /// </summary> 
    /// <param name="directoryPath">目录的绝对路径</param> 
    public static void CreateDirectory(string directoryPath)
    {
		#if !UNITY_WEBPLAYER
        //如果目录不存在则创建该目录 
        if (!IsExistDirectory(directoryPath))
        {
            Directory.CreateDirectory(directoryPath);
        }
		#endif
    }
    #endregion

    #region 创建一个文件
    /// <summary> 
    /// 创建一个文件。 
    /// </summary> 
    /// <param name="filePath">文件的绝对路径</param> 
    public static void CreateFile(string filePath)
    {
        try
        {
			#if !UNITY_WEBPLAYER
            //如果文件不存在则创建该文件 
            if (!IsExistFile(filePath))
            {
                //创建一个FileInfo对象 
                FileInfo file = new FileInfo(filePath);
                //创建文件 
                FileStream fs = file.Create();
                //关闭文件流 
                fs.Close();
            }
			#endif
        }
        catch (Exception ex)
        {

            throw ex;
        }
    }
    /// <summary> 
    /// 创建一个文件,并将字节流写入文件。 
    /// </summary> 
    /// <param name="filePath">文件的绝对路径</param> 
    /// <param name="buffer">二进制流数据</param> 
    public static void CreateFile(string filePath, byte[] buffer)
    {
        try
        {
			#if !UNITY_WEBPLAYER
            //如果文件不存在则创建该文件 
            if (!IsExistFile(filePath))
            {
                //创建一个FileInfo对象 
                FileInfo file = new FileInfo(filePath);
                //创建文件 
                FileStream fs = file.Create();
                //写入二进制流 
                fs.Write(buffer, 0, buffer.Length);
                //关闭文件流 
                fs.Close();
            }
			#endif
        }
        catch (Exception ex)
        {

            throw ex;
        }
    }

    /// <summary>
    /// 创建一个文件
    /// </summary>
    /// <param name="filePath">路径</param>
    /// <param name="s">内容</param>
    /// <param name="encode">编码</param>
    /// <returns></returns>
    public static bool CreateFile(string filePath, string s, string encode)
    {
        bool ret = true;
		#if !UNITY_WEBPLAYER
        //string path = ConfigurationManager.AppSettings["MakeContentPath"];
        Encoding code = Encoding.GetEncoding(encode);
        StreamWriter sw = null;
        try
        {
            sw = new StreamWriter( filePath, false, code);
            sw.Write(s);
            sw.Flush();
        }
        catch (Exception ex)
        {
            ret = false;
            throw ex;
        }
        finally
        {
            sw.Close();
        }
		#endif
        return ret;
    }
    #endregion

    #region 获取文本文件的行数
    /// <summary> 
    /// 获取文本文件的行数 
    /// </summary> 
    /// <param name="filePath">文件的绝对路径</param>         
    public static int GetLineCount(string filePath)
    {
		#if !UNITY_WEBPLAYER
        //将文本文件的各行读到一个字符串数组中 
        string[] rows = File.ReadAllLines(filePath);
        //返回行数 
        return rows.Length;
#else
		return 0;
#endif
    }
    #endregion

    #region 获取一个文件的长度
    /// <summary> 
    /// 获取一个文件的长度,单位为Byte 
    /// </summary> 
    /// <param name="filePath">文件的绝对路径</param>         
    public static int GetFileSize(string filePath)
    {
		#if !UNITY_WEBPLAYER
        //创建一个文件对象 
        FileInfo fi = new FileInfo(filePath);
        //获取文件的大小 
        return (int)fi.Length;
#else 
		return 0;
		#endif
    }
    /// <summary> 
    /// 获取一个文件的长度,单位为KB 
    /// </summary> 
    /// <param name="filePath">文件的路径</param>         
    public static double GetFileSizeByKB(string filePath)
    {
		#if !UNITY_WEBPLAYER
        //创建一个文件对象 
        FileInfo fi = new FileInfo(filePath);
        //获取文件的大小 
        return Convert.ToDouble(Convert.ToDouble(fi.Length) / 1024);
#else
		return 0;
#endif
    }
    /// <summary> 
    /// 获取一个文件的长度,单位为MB 
    /// </summary> 
    /// <param name="filePath">文件的路径</param>         
    public static double GetFileSizeByMB(string filePath)
    {
		#if !UNITY_WEBPLAYER
        //创建一个文件对象 
        FileInfo fi = new FileInfo(filePath);
        //获取文件的大小 
        return Convert.ToDouble(Convert.ToDouble(fi.Length) / 1024 / 1024);
#else
		return 0;
#endif
    }
    #endregion

    #region 获取指定目录中的文件列表
    /// <summary> 
    /// 获取指定目录中所有文件列表 
    /// </summary> 
    /// <param name="directoryPath">指定目录的绝对路径</param>         
    public static string[] GetFileNames(string directoryPath)
    {
		#if !UNITY_WEBPLAYER
        //如果目录不存在，则抛出异常 
        if (!IsExistDirectory(directoryPath))
        {
            throw new FileNotFoundException();
        }
        //获取文件列表 
        return Directory.GetFiles(directoryPath);
#else 
		return null;
#endif
    }
    /// <summary> 
    /// 获取指定目录及子目录中所有文件列表 
    /// </summary> 
    /// <param name="directoryPath">指定目录的绝对路径</param> 
    /// <param name="searchPattern">模式字符串，"*"代表0或N个字符，"?"代表1个字符。 
    /// 范例："Log*.xml"表示搜索所有以Log开头的Xml文件。</param> 
    /// <param name="isSearchChild">是否搜索子目录</param> 
    public static string[] GetFileNames(string directoryPath, string searchPattern, bool isSearchChild)
    {
		#if !UNITY_WEBPLAYER
        //如果目录不存在，则抛出异常 
        if (!IsExistDirectory(directoryPath))
        {
            throw new FileNotFoundException();
        }
        try
        {
            if (isSearchChild)
            {
                return Directory.GetFiles(directoryPath, searchPattern, SearchOption.AllDirectories);
            }
            else
            {
                return Directory.GetFiles(directoryPath, searchPattern, SearchOption.TopDirectoryOnly);
            }
        }
        catch (IOException ex)
        {
            throw ex;
        }
#else
		return null;
#endif
    }
    #endregion

    #region 获取指定目录中的子目录列表
    /// <summary> 
    /// 获取指定目录中所有子目录列表,若要搜索嵌套的子目录列表,请使用重载方法. 
    /// </summary> 
    /// <param name="directoryPath">指定目录的绝对路径</param>         
    public static string[] GetDirectories(string directoryPath)
    {
		#if !UNITY_WEBPLAYER
        try
        {
            return Directory.GetDirectories(directoryPath);
        }
        catch (IOException ex)
        {
            throw ex;
        }
#else
		return null;
		#endif
    }

    #region 获取指定目录中所有文件数量
    /// <summary> 
    /// 获取指定目录中所有文件数量
    /// </summary> 
    /// <param name="directoryPath">指定目录的绝对路径</param>         
    public static int GetFilesCount(DirectoryInfo dirInfo)
    {
        int total = 0;
		#if !UNITY_WEBPLAYER
        total += dirInfo.GetFiles().Length;
        foreach (DirectoryInfo subDir in dirInfo.GetDirectories())
        {
            total += GetFilesCount(subDir);
        }
		#endif
        return total;
    }
    #endregion

    /// <summary> 
    /// 获取指定目录及子目录中所有子目录列表 
    /// </summary> 
    /// <param name="directoryPath">指定目录的绝对路径</param> 
    /// <param name="searchPattern">模式字符串，"*"代表0或N个字符，"?"代表1个字符。 
    /// 范例："Log*.xml"表示搜索所有以Log开头的Xml文件。</param> 
    /// <param name="isSearchChild">是否搜索子目录</param> 
    public static string[] GetDirectories(string directoryPath, string searchPattern, bool isSearchChild)
    {
		#if !UNITY_WEBPLAYER
        try
        {
            if (isSearchChild)
            {
                return Directory.GetDirectories(directoryPath, searchPattern, SearchOption.AllDirectories);
            }
            else
            {
                return Directory.GetDirectories(directoryPath, searchPattern, SearchOption.TopDirectoryOnly);
            }
        }
        catch (IOException ex)
        {
            throw ex;
        }
#else
		return null;
		#endif
    }
    #endregion

    #region 向文本文件写入内容
    /// <summary> 
    /// 向文本文件中写入内容 
    /// </summary> 
    /// <param name="filePath">文件的绝对路径</param> 
    /// <param name="content">写入的内容</param>         
    public static void WriteText(string filePath, string content)
    {
		#if !UNITY_WEBPLAYER
        //向文件写入内容 
        File.WriteAllText(filePath, content);       
		#endif
    }
    #endregion

    #region 向文本文件的尾部追加内容
    /// <summary> 
    /// 向文本文件的尾部追加内容 
    /// </summary> 
    /// <param name="filePath">文件的绝对路径</param> 
    /// <param name="content">写入的内容</param> 
    public static void AppendText(string filePath, string content)
    {
		#if !UNITY_WEBPLAYER
        File.AppendAllText(filePath, content);
		#endif
    }
    #endregion

    #region 将现有文件的内容复制到新文件中
    /// <summary> 
    /// 将源文件的内容复制到目标文件中 
    /// </summary> 
    /// <param name="sourceFilePath">源文件的绝对路径</param> 
    /// <param name="destFilePath">目标文件的绝对路径</param> 
    public static void Copy(string sourceFilePath, string destFilePath)
    {
		#if !UNITY_WEBPLAYER
        File.Copy(sourceFilePath, destFilePath, true);
		#endif
    }
    #endregion

    #region 将文件移动到指定目录
    /// <summary> 
    /// 将文件移动到指定目录 
    /// </summary> 
    /// <param name="sourceFilePath">需要移动的源文件的绝对路径</param> 
    /// <param name="descDirectoryPath">移动到的目录的绝对路径</param> 
    public static void Move(string sourceFilePath, string descDirectoryPath)
    {
		#if !UNITY_WEBPLAYER
        //获取源文件的名称 
        string sourceFileName = GetFileName(sourceFilePath);
        if (IsExistDirectory(descDirectoryPath))
        {
            //如果目标中存在同名文件,则删除 
            if (IsExistFile(descDirectoryPath + "\\" + sourceFileName))
            {
                DeleteFile(descDirectoryPath + "\\" + sourceFileName);
            }
            //将文件移动到指定目录 
            File.Move(sourceFilePath, descDirectoryPath + "\\" + sourceFileName);
        }
		#endif
    }
    #endregion

    #region 复制文件或者文件夹到目标路径下
    /// <summary>
    /// 复制文件或者文件夹到目标路径下
    /// </summary>
    /// <param name="from">源路径</param>
    /// <param name="to">目标路径</param>
    public static void CopyDirectory(string from, string to)
    {
        try
        {
			#if !UNITY_WEBPLAYER
            if (to[to.Length - 1] != Path.DirectorySeparatorChar)
            {
                to += Path.DirectorySeparatorChar;
            }
            if (!Directory.Exists(to))
            {
                Directory.CreateDirectory(to);
            }
            string[] fileList = Directory.GetFileSystemEntries(from);
            foreach (string file in fileList)
            {
                if (Directory.Exists(file))
                {
                    CopyDirectory(file, to + Path.GetFileName(file));
                }
                else
                {
                    File.Copy(file, to + Path.GetFileName(file), true);
                }
            }
			#endif
        }
        catch (Exception ex)
        {
            Debug.LogError(ex.Message);
        }
    }
    #endregion

    #region 移动文件或者文件夹到目标路径下
    /// <summary>
    /// 移动文件或者文件夹到目标路径下
    /// </summary>
    /// <param name="from">源路径</param>
    /// <param name="to">目标路径</param>
    public static void MoveFileOrDirectory(string from, string to)
    {
        
    }
    #endregion

    #region 将流读取到缓冲区中
    /// <summary> 
    /// 将流读取到缓冲区中 
    /// </summary> 
    /// <param name="stream">原始流</param> 
    public static byte[] StreamToBytes(Stream stream)
    {
        try
        {
            //创建缓冲区 
            byte[] buffer = new byte[stream.Length];
            //读取流 
            stream.Read(buffer, 0, Convert.ToInt32(stream.Length));
            //返回流 
            return buffer;
        }
        catch (Exception ex)
        {
            throw ex;
        }
        finally
        {
            //关闭流 
            stream.Close();
        }
    }
    #endregion

    #region 将文件读取到缓冲区中
    /// <summary> 
    /// 将文件读取到缓冲区中 
    /// </summary> 
    /// <param name="filePath">文件的绝对路径</param> 
    public static byte[] FileToBytes(string filePath)
    {
		#if !UNITY_WEBPLAYER
        //获取文件的大小  
        int fileSize = GetFileSize(filePath);
        //创建一个临时缓冲区 
        byte[] buffer = new byte[fileSize];
        //创建一个文件流 
        FileInfo fi = new FileInfo(filePath);
        FileStream fs = fi.Open(FileMode.Open);
        try
        {
            //将文件流读入缓冲区 
            fs.Read(buffer, 0, fileSize);
            return buffer;
        }
        catch (IOException ex)
        {
            throw ex;
        }
        finally
        {
            //关闭文件流 
            fs.Close();
        }
#else
		return null;
		#endif
    }
    #endregion

    #region 将文件读取到字符串中
    /// <summary> 
    /// 将文件读取到字符串中 
    /// </summary> 
    /// <param name="filePath">文件的绝对路径</param> 
    public static string FileToString(string filePath)
    {
        return FileToString(filePath, Encoding.Default);
    }
    /// <summary> 
    /// 将文件读取到字符串中 
    /// </summary> 
    /// <param name="filePath">文件的绝对路径</param> 
    /// <param name="encoding">字符编码</param> 
    public static string FileToString(string filePath, Encoding encoding)
    {
        //创建流读取器 
        StreamReader reader = new StreamReader(filePath, encoding);
        try
        {
            //读取流 
            return reader.ReadToEnd();
        }
        catch (Exception ex)
        {
            throw ex;
        }
        finally
        {
            //关闭流读取器 
            reader.Close();
        }
    }
    #endregion

    #region 保存文件到指定目录
    /// <summary>
    /// 保存文件到指定目录
    /// </summary>
    /// <param name="fileName"></param>
    /// <param name="bytes"></param>
    /// <returns></returns>
    static public bool Save(string path, byte[] bytes)
    {
		#if !UNITY_WEBPLAYER
        if (bytes == null)
        {
            if (File.Exists(path)) File.Delete(path);
            return true;
        }

        FileStream file = null;

        try
        {
            file = File.Create(path);
        }
        catch (System.Exception ex)
        {
            Debug.LogError(ex.Message);
            return false;
        }

        file.Write(bytes, 0, bytes.Length);
        file.Close();
#endif
        return true;
    }
    #endregion

    #region 从文件的绝对路径中获取文件名( 包含扩展名 )
    /// <summary> 
    /// 从文件的绝对路径中获取文件名( 包含扩展名 ) 
    /// </summary> 
    /// <param name="filePath">文件的绝对路径</param>         
    public static string GetFileName(string filePath)
    {
        //获取文件的名称 
		#if !UNITY_WEBPLAYER
        FileInfo fi = new FileInfo(filePath);
        return fi.Name;
#else
		return string.Empty;
#endif
    }
    #endregion

    #region 从文件的绝对路径中获取文件名( 不包含扩展名 )
    /// <summary> 
    /// 从文件的绝对路径中获取文件名( 不包含扩展名 ) 
    /// </summary> 
    /// <param name="filePath">文件的绝对路径</param>         
    public static string GetFileNameNoExtension(string filePath)
    {
        //获取文件的名称 
		#if !UNITY_WEBPLAYER
        FileInfo fi = new FileInfo(filePath);
        return fi.Name.Split('.')[0];
#else
		return string.Empty;
#endif
    }
    #endregion

    #region 从文件夹的绝对路径中获取文件夹名
    /// <summary> 
    /// 从文件夹的绝对路径中获取文件夹名
    /// </summary> 
    /// <param name="filePath">文件夹的绝对路径</param>         
    public static string GetDiretoryName(string filePath)
    {
		#if !UNITY_WEBPLAYER
        //获取文件的名称 
        DirectoryInfo fi = new DirectoryInfo(filePath);
        return fi.Name;
#else
		return string.Empty;
#endif
    }
    #endregion

    #region 从文件的绝对路径中获取扩展名
    /// <summary> 
    /// 从文件的绝对路径中获取扩展名 
    /// </summary> 
    /// <param name="filePath">文件的绝对路径</param>         
    public static string GetExtension(string filePath)
    {
		#if !UNITY_WEBPLAYER
        //获取文件的名称 
        FileInfo fi = new FileInfo(filePath);
        return fi.Extension;
#else
		return string.Empty;
#endif
    }
    #endregion

    #region 清空指定目录
    /// <summary> 
    /// 清空指定目录下所有文件及子目录,但该目录依然保存. 
    /// </summary> 
    /// <param name="directoryPath">指定目录的绝对路径</param> 
    public static void ClearDirectory(string directoryPath)
    {
        if (IsExistDirectory(directoryPath))
        {
            //删除目录中所有的文件 
            string[] fileNames = GetFileNames(directoryPath);
            for (int i = 0; i < fileNames.Length; i++)
            {
                DeleteFile(fileNames[i]);
            }
            //删除目录中所有的子目录 
            string[] directoryNames = GetDirectories(directoryPath);
            for (int i = 0; i < directoryNames.Length; i++)
            {
                DeleteDirectory(directoryNames[i]);
            }
        }
    }
    #endregion

    #region 清空文件内容
    /// <summary> 
    /// 清空文件内容 
    /// </summary> 
    /// <param name="filePath">文件的绝对路径</param> 
    public static void ClearFile(string filePath)
    {
		#if !UNITY_WEBPLAYER
        //删除文件 
        File.Delete(filePath);
        //重新创建该文件 
        CreateFile(filePath);
#endif
    }
    #endregion

    #region 删除指定文件
    /// <summary> 
    /// 删除指定文件 
    /// </summary> 
    /// <param name="filePath">文件的绝对路径</param> 
    public static void DeleteFile(string filePath)
    {
		#if !UNITY_WEBPLAYER
        if (IsExistFile(filePath))
        {
            File.Delete(filePath);
        }
#endif
    }
    #endregion

    #region 删除指定目录
    /// <summary> 
    /// 删除指定目录及其所有子目录 
    /// </summary> 
    /// <param name="directoryPath">指定目录的绝对路径</param> 
    public static void DeleteDirectory(string directoryPath,Action deleteCallBack = null)
    {
		#if !UNITY_WEBPLAYER
        if (IsExistDirectory(directoryPath))
        {
            string[] files = Directory.GetFiles(directoryPath);
            string[] dirs = Directory.GetDirectories(directoryPath);
            foreach (string file in files)
            {
                try
                {
                    File.SetAttributes(file, FileAttributes.Normal);
                    File.Delete(file);
                }
                catch (Exception e)
                {
                    Debug.LogError("Delete file failed: " + file);
                    Debug.LogError(e.Message);
                    return;
                }

                if (deleteCallBack != null)
                {
                    deleteCallBack();
                }
            }

            foreach (string dir in dirs)
            {
                try
                {
                    DeleteDirectory(dir, deleteCallBack);
                }
                catch (Exception e)
                {
                    Debug.LogError("Delete dir failed: " + dir);
                    Debug.LogError(e.Message);
                    return;
                }
            }
            Directory.Delete(directoryPath, false);
        }
#endif
    }
    #endregion

    private static void ForFilesFind(DirectoryInfo kDI, System.Collections.Generic.List<FileInfo> listFI, string strSearchPattern)
    {
        FileInfo[] arrFI = kDI.GetFiles(strSearchPattern);
        listFI.AddRange(arrFI);

        int i;

        DirectoryInfo[] arrDI = kDI.GetDirectories();
        for (i = 0; i < arrDI.Length; i++)
        {
            ForFilesFind(arrDI[i], listFI, strSearchPattern);
        }
    }

    private static int SortByFileNameFunc(FileInfo kFI1, FileInfo kFI2)
    {
        if (kFI1 == null || kFI2 == null)
        {
            return 0;
        }
        else
        {
            return string.Compare(kFI1.FullName, kFI2.FullName, true);
        }
    }

    public static FileInfo[] FindAllFiles(string strPath, string strSearchPattern = "*", bool bSortByFileName = false)
    {
        if (strPath == null)
        {
            return null;
        }

        List<FileInfo> listFI = new List<FileInfo>();
        DirectoryInfo kDI = new DirectoryInfo(strPath);
        ForFilesFind(kDI, listFI, strSearchPattern);

        if (bSortByFileName)
        {
            listFI.Sort(SortByFileNameFunc);
        }
        return listFI.ToArray();
    }

    public delegate void ON_WALK_FILE(string strFilePath);

    public static void EnumFile(string strPath, ON_WALK_FILE fnWalk, bool isGetFile= false)
    {
        if (strPath == null)
        {
            return;
        }

        DirectoryInfo kDI = new DirectoryInfo(strPath);
        ForEnumFile(kDI, fnWalk, isGetFile);
    }

    private static void ForEnumFile(FileSystemInfo info, ON_WALK_FILE fnWalk, bool isGetFile = false)
    {
        if (!info.Exists)
        {
            return;
        }

        DirectoryInfo dir = info as DirectoryInfo;

        //不是目录 
        if (dir == null)
        {
            return;
        }

        int i;
        FileInfo file;

        FileSystemInfo[] files = dir.GetFileSystemInfos();
        for (i = 0; i < files.Length; i++)
        {
            file = files[i] as FileInfo;

            //是文件 
            if (file != null)
            {
                if (fnWalk != null)
                {
                    fnWalk(file.FullName);
                }
            }
            else
            {
                if (isGetFile)
                {
                    if (fnWalk != null)
                    {
                        fnWalk(files[i].FullName);
                    }
                }
                ForEnumFile(files[i], fnWalk, isGetFile);
            }
        }
    }
}
