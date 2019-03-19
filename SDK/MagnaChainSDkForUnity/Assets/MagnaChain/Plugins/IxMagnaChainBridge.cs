
using CellLink;
using System.Collections.Generic;
using CellLink.RPC;
using System.Net;
using System;
using CellLink.DataEncoders;
using Newtonsoft.Json.Linq;
using Newtonsoft.Json;
using System.Text;
using System.Threading.Tasks;
using CellLink.Policy;
using System.Threading;

namespace MagnaChainEx
{
    public class IxMagnaChainBridge
    {
        public delegate void ON_TRANSFER_DONE(RPCError kError, string strTxid);
        public delegate void ON_COMMAND_DONE(RPCResponse kRsp);

        public delegate void ON_GET_BALANCE(float fBalance);

        public delegate void ON_PUBLISH_CONTRACT_DONE(bool bSucc, string strContractAddress, object kArg);

        public delegate void ON_CALL_CONTRACT_FUNCTION(bool bSucc, string strRet);

        //public static IxMagnaChainBridge ms_kSig;

        private uint MAX_EXT_KEY_INDEX = 2147483000;
        
        //private bool m_bInited = false;

        private Thread m_kAsyncRPCThread;
        private int m_iAsyncRPCThreadQuit = 1;

        private class ASYNC_CMD_INFO
        {
            public string strCmd;
            public object[] arrParams;
            public ON_COMMAND_DONE fnCmdDone;
        }

        private class ASYNC_CMD_RESULT_INFO
        {
            public ON_COMMAND_DONE fnCmdDone;
            public RPCResponse kRsp;
        }

        private class MASTER_KEY_INFO
        {
            public ExtKey kMasterKey;
            public ExtKey kParentKey;
            public string strParentPath;
            public byte ucCurDepth;
            public uint uiCurSubKeyIndex;
        }

        private Dictionary<string, MASTER_KEY_INFO> m_mapMKI;

        private List<ASYNC_CMD_INFO> m_listACI;
        private List<ASYNC_CMD_RESULT_INFO> m_listACRI;

        private float m_fCheckAsyncResultIntervalTime = 0.1f;
        private float m_fCheckAyncResultTick = 0.0f;

        private CellLink.Network m_kCLNet;

        private RPCClient m_kRPC;

        static StandardTransactionPolicy ms_kEasyPolicy = new StandardTransactionPolicy()
        {
            MaxTransactionSize = null,
            MaxTxFee = null,
            MinRelayTxFee = null,
            ScriptVerify = ScriptVerify.Standard & ~ScriptVerify.LowS
        };

        private StringBuilder m_sbK = new StringBuilder(1024);

        public IxMagnaChainBridge()
        {
            //if (ms_kSig != null)
            //{
            //    Logger.LogError("This is a singlon: " + this);
            //    return;
            //}
            //ms_kSig = this;

            m_mapMKI = new Dictionary<string, MASTER_KEY_INFO>();

            m_listACI = new List<ASYNC_CMD_INFO>();
            m_listACRI = new List<ASYNC_CMD_RESULT_INFO>();
        }

        public CellLink.Network network
        {
            get { return m_kCLNet; }
        }

        public bool Initialize(string strRPCAddress, int iRPCPort, string strCredUserName = "", string strCredPwd = "", NETWORK_TYPE eNetType = NETWORK_TYPE.TEST_NET)
        {
            if (string.IsNullOrEmpty(strRPCAddress))
            {
                return false;
            }

            if (eNetType == NETWORK_TYPE.MAIN)
            {
                m_kCLNet = CellLink.Network.Main;
            }
            else if (eNetType == NETWORK_TYPE.TEST_NET)
            {
                m_kCLNet = CellLink.Network.TestNet;
            }
            else
            {
                m_kCLNet = CellLink.Network.RegTest;
            }

            NetworkCredential creds = new NetworkCredential();

            creds.UserName = strCredUserName;
            creds.Password = strCredPwd;
            string strAddress = strRPCAddress + ":" + iRPCPort.ToString();
            try
            {
                m_kRPC = new RPCClient(creds, new Uri(strAddress), m_kCLNet);
                //m_bInited = true;

                m_kAsyncRPCThread = new Thread(new ThreadStart(AsyncCommandThreadProc));
                m_kAsyncRPCThread.IsBackground = true;
                m_iAsyncRPCThreadQuit = 0;
                m_kAsyncRPCThread.Start();             
                return true;
            }
            catch (Exception e)
            {
                Logger.LogError(e.Message);
                return false;
            }
        }

        public void Release()
        {
            Interlocked.Exchange(ref m_iAsyncRPCThreadQuit, 1);

            if (m_mapMKI != null)
            {
                m_mapMKI.Clear();
            }
            m_kRPC = null;

            //ms_kSig = null;
        }

        public void InitTest()
        {
            MASTER_KEY_INFO kMKI = new MASTER_KEY_INFO();
            kMKI.kMasterKey = new ExtKey();
            kMKI.ucCurDepth = 3;        // 直接跳过1级
            kMKI.uiCurSubKeyIndex = MAX_EXT_KEY_INDEX;
            kMKI.strParentPath = "0'/2147483000/";

            KeyPath kPath = new KeyPath("0'/2147483000");
            kMKI.kParentKey = kMKI.kMasterKey.Derive(kPath);
            m_mapMKI["test"] = kMKI;
        }

        // return wif ext key
        public string CreateMasterExtKey(string strName)
        {
            if (string.IsNullOrEmpty(strName))
            {
                return null;
            }

            if (m_mapMKI.ContainsKey(strName))
            {
                Logger.LogError("The master key has exist, name: " + strName);
                return null;
            }

            MASTER_KEY_INFO kMKI = new MASTER_KEY_INFO();
            kMKI.kMasterKey = new ExtKey();
            kMKI.ucCurDepth = 2;        // 直接跳过1级
            kMKI.uiCurSubKeyIndex = 0xFFFFFFFF;
            kMKI.strParentPath = "0'/";

            KeyPath kPath = new KeyPath("0'");
            kMKI.kParentKey = kMKI.kMasterKey.Derive(kPath);
            m_mapMKI[strName] = kMKI;
            return kMKI.kMasterKey.ToString(m_kCLNet);
        }

        // return wif ext key
        public string CreateMasterExtKey(string strName, string strPriKey)
        {
            if (string.IsNullOrEmpty(strName) || string.IsNullOrEmpty(strPriKey))
            {
                return null;
            }

            if (m_mapMKI.ContainsKey(strName))
            {
                Logger.LogError("The master key has exist, name: " + strName);
                return null;
            }

            MASTER_KEY_INFO kMKI = new MASTER_KEY_INFO();
            try
            {
                Key kPriKey = Key.Parse(strPriKey, m_kCLNet);
                kMKI.kMasterKey = new ExtKey(kPriKey);
                kMKI.ucCurDepth = 2;        // 直接跳过1级
                kMKI.uiCurSubKeyIndex = 0xFFFFFFFF;
                kMKI.strParentPath = "0'/";

                KeyPath kPath = new KeyPath("0'");
                kMKI.kParentKey = kMKI.kMasterKey.Derive(kPath);
            }
            catch (Exception e)
            {
                Logger.LogError(e.Message);
                return null;
            }
            m_mapMKI[strName] = kMKI;
            return kMKI.kMasterKey.ToString(m_kCLNet);
        }

        public string CreateMasterExtKeyByAid(string strName, string strAid)
        {
            if (string.IsNullOrEmpty(strName) || string.IsNullOrEmpty(strAid))
            {
                return null;
            }

            if (m_mapMKI.ContainsKey(strName))
            {
                Logger.LogError("The master key has exist, name: " + strName);
                return null;
            }

            MASTER_KEY_INFO kMKI = new MASTER_KEY_INFO();
            kMKI.kMasterKey = ExtKey.CreateByAid(strAid);
            kMKI.ucCurDepth = 2;        // 直接跳过1级
            kMKI.uiCurSubKeyIndex = 0xFFFFFFFF;
            kMKI.strParentPath = "0'/";

            KeyPath kPath = new KeyPath("0'");
            kMKI.kParentKey = kMKI.kMasterKey.Derive(kPath);
            m_mapMKI[strName] = kMKI;
            return kMKI.kMasterKey.ToString(m_kCLNet);            
        }     
        
        public string RestoreMasterExtKeyByAid(string strAid)
        {
            if ( string.IsNullOrEmpty(strAid))
            {
                return null;
            }

            ExtKey kEK = ExtKey.CreateByAid(strAid);
            return kEK.ToString(m_kCLNet);
        }

        public bool IsMasterExtKeyExist(string strName)
        {
            if ( string.IsNullOrEmpty(strName))
            {
                return false;
            }

            if ( m_mapMKI == null )
            {
                return false;
            }

            return m_mapMKI.ContainsKey(strName);
        }

        public bool RemoveMasterKey(string strName)
        {
            if ( string.IsNullOrEmpty(strName))
            {
                return false;
            }

            return m_mapMKI.Remove(strName);
        }

        private string FindLastMCLAddress(List<string> listMCLAddress)
        {
            if (listMCLAddress == null || listMCLAddress.Count == 0)
            {
                return null;
            }

            string strFirstName = null;

            string strLastPath = null;
            string strLastCLAddress = null;
            int iMaxPath = 0;
            int i;
            string strName;
            string strPath;
            string strOptPath;
            string strBTAddress;
            string[] arrSeg;
            string[] arrSegLast;
            int j;

            uint uiLast;
            uint uiCur;

            char[] arrSpt = new char[1];
            arrSpt[0] = '/';
            for (i = 0; i < listMCLAddress.Count; i++)
            {
                if (DecodeMCLAddress(listMCLAddress[i], out strName, out strPath, out strBTAddress) == false)
                {
                    continue;
                }

                if (string.IsNullOrEmpty(strName))
                {
                    Logger.LogError("Null name, ignore CL address: " + listMCLAddress[i]);
                    continue;
                }

                if (string.IsNullOrEmpty(strPath))
                {
                    Logger.LogError("Null path, ignore CL address: " + listMCLAddress[i]);
                    continue;
                }

                if (strFirstName == null)
                {
                    if (!string.IsNullOrEmpty(strName))
                    {
                        strFirstName = strName;
                    }
                }

                if (strName != strFirstName)
                {
                    Logger.LogError("Name mismatch, first name: " + strFirstName + " current name: " + strName + " CL address: " + listMCLAddress[i]);
                    continue;
                }

                strOptPath = strPath.Replace("'", "");
                arrSeg = strOptPath.Split(arrSpt);
                if (arrSeg.Length > iMaxPath)
                {
                    strLastPath = strPath;
                    strLastCLAddress = listMCLAddress[i];
                    iMaxPath = arrSeg.Length;
                }
                else if (arrSeg.Length == iMaxPath)
                {
                    strOptPath = strLastPath.Replace("'", "");
                    arrSegLast = strOptPath.Split(arrSpt);
                    for (j = 0; j < iMaxPath; j++)
                    {
                        try
                        {
                            uiLast = Convert.ToUInt32(arrSegLast[j]);
                            uiCur = Convert.ToUInt32(arrSeg[j]);
                            if (uiCur > uiLast)
                            {
                                strLastPath = strPath;
                                strLastCLAddress = listMCLAddress[i];
                                break;
                            }
                        }
                        catch (Exception e)
                        {
                            Logger.LogError(e.Message);
                        }
                    }
                }
            }
            return strLastCLAddress;
        }

        public bool ImportMasterExtKey(string strMasterWifExtKey, List<string> listRefMCLAddress)
        {
            if (listRefMCLAddress == null || listRefMCLAddress.Count == 0)
            {
                return false;
            }

            if (m_mapMKI == null)
            {
                return false;
            }

            string strLastCLAddress = FindLastMCLAddress(listRefMCLAddress);

            string strName;
            string strPath;
            string strBTAddress;

            if (DecodeMCLAddress(strLastCLAddress, out strName, out strPath, out strBTAddress) == false)
            {
                return false;
            }

            return ImportMasterExtKey(strName, strMasterWifExtKey, strPath);
        }        

        // import wif ext key
        // strLastKeyPath: like "0'/2/1" or "0'/2'/1"
        public bool ImportMasterExtKey(string strName, string strMasterWifExtKey, string strLastKeyPath = null)
        {
            if (string.IsNullOrEmpty(strMasterWifExtKey) || string.IsNullOrEmpty(strName))
            {
                return false;
            }

            if ( m_mapMKI == null )
            {
                return false;
            }

            MASTER_KEY_INFO kMKI;

            if (m_mapMKI.TryGetValue(strName, out kMKI))
            {
                Logger.Log("The master private key has exist, name: " + strName);
                return false;
            }

            try
            {
                kMKI = new MASTER_KEY_INFO();
                try
                {
                    kMKI.kMasterKey = ExtKey.Parse(strMasterWifExtKey, m_kCLNet);
                }
                catch ( Exception e)
                {
                    Logger.LogError(e.Message);
                    return false;
                }

                if ( kMKI.kMasterKey == null )
                {
                    return false;
                }
                
                if (string.IsNullOrEmpty(strLastKeyPath))
                {
                    //kMKI.ucCurDepth = 0;
                    //kMKI.uiCurSubKeyIndex = 0;
                    //kMKI.strParentPath = "";
                    
                    kMKI.ucCurDepth = 2;        // 直接跳过1级
                    kMKI.uiCurSubKeyIndex = 0xFFFFFFFF;
                    kMKI.strParentPath = "0'/";

                    KeyPath kPath = new KeyPath("0'");
                    kMKI.kParentKey = kMKI.kMasterKey.Derive(kPath);
                    m_mapMKI[strName] = kMKI;
                }
                else
                {
                    char[] arrSpt = new char[1];
                    arrSpt[0] = '/';
                    string[] arrSeg = strLastKeyPath.Split(arrSpt);
                    kMKI.ucCurDepth = (byte)arrSpt.Length;

                    string strParentPath;

                    string strIndex = arrSeg[arrSeg.Length - 1].Replace("'", "");
                    kMKI.uiCurSubKeyIndex = Convert.ToUInt32(strIndex);
                    int iL = strLastKeyPath.LastIndexOf("/");
                    if (iL != -1)
                    {
                        kMKI.strParentPath = strLastKeyPath.Substring(0, iL + 1);
                        strParentPath = strLastKeyPath.Substring(0, iL);
                    }
                    else
                    {
                        kMKI.strParentPath = strLastKeyPath + "/";
                        strParentPath = strLastKeyPath;
                    }

                    KeyPath kPath = new KeyPath(strParentPath);
                    kMKI.kParentKey = kMKI.kMasterKey.Derive(kPath);
                }
                m_mapMKI[strName] = kMKI;
                return true;
            }
            catch (Exception e)
            {
                Logger.LogError(e.Message);
                return false;
            }
        }         

        public string EncodeMCLAddress(string strName, string strPath, string strCelAddress)
        {
            if (string.IsNullOrEmpty(strName) || string.IsNullOrEmpty(strPath) || string.IsNullOrEmpty(strCelAddress))
            {
                return null;
            }

            string strNewPath = strPath.Replace("/", "C");
            strNewPath = strNewPath.Replace("'", "h");
            byte[] arrName = Encoding.UTF8.GetBytes(strName);
            string strNewName = Encoders.Base58.EncodeData(arrName);
            string strCLAddress = strCelAddress + "I" + strNewName + "l" + strNewPath;
            return strCLAddress;
        }

        public bool DecodeMCLAddress(string strMCLAddress, out string strName, out string strPath, out string strCelAddress)
        {
            strName = null;
            strPath = null;
            strCelAddress = null;

            if (string.IsNullOrEmpty(strMCLAddress))
            {
                return false;
            }

            int iL = strMCLAddress.IndexOf("I");
            if (iL == -1)
            {
                Logger.LogError("Error CL address 1: " + strMCLAddress);
                return false;
            }

            int iR = strMCLAddress.IndexOf("l", iL);
            if (iR == -1)
            {
                Logger.LogError("Error CL address 2: " + strMCLAddress);
                return false;
            }

            strCelAddress = strMCLAddress.Substring(0, iL);
            string str58Name = strMCLAddress.Substring(iL + 1, iR - iL - 1);
            byte[] arrName = Encoders.Base58.DecodeData(str58Name);
            strName = Encoding.UTF8.GetString(arrName);
            strPath = strMCLAddress.Substring(iR + 1);
            strPath = strPath.Replace("C", "/");
            strPath = strPath.Replace("h", "'");
            return true;
        }

        public string ConvertMCLAddressToCelAddress(string strMCLAddress)
        {
            if (string.IsNullOrEmpty(strMCLAddress))
            {
                return null;
            }

            string strName;
            string strBTAddress;
            string strPath;

            if (DecodeMCLAddress(strMCLAddress, out strName, out strPath, out strBTAddress) == false)
            {
                return null;
            }
            return strBTAddress;
        }

        public string GetCelAddressByPath(string strMasterKeyName, string strPath)
        {
            if ( string.IsNullOrEmpty(strMasterKeyName) || string.IsNullOrEmpty(strPath))
            {
                return null;
            }

            MASTER_KEY_INFO kMKI;

            if (m_mapMKI.TryGetValue(strMasterKeyName, out kMKI) == false)
            {
                Logger.LogError("Can not find master key name: " + strMasterKeyName);
                return null;
            }

            KeyPath kPath;
            ExtKey kNewKey;

            try
            {
                kPath = new KeyPath(strPath);
                //kNewKey = kMKI.kParentKey.Derive(kPath);
                kNewKey = kMKI.kMasterKey.Derive(kPath);
            }
            catch (Exception e)
            {
                Logger.LogError(e.Message);
                return null;
            }

            return kNewKey.PrivateKey.PubKey.GetAddress(m_kCLNet).ToWif();
        }

        public ExtKey GetExtKeyByPath(string strMasterKeyName, string strPath)
        {
            if ( string.IsNullOrEmpty(strMasterKeyName) || string.IsNullOrEmpty(strPath))
            {
                return null;
            }

            MASTER_KEY_INFO kMKI;

            if (m_mapMKI.TryGetValue(strMasterKeyName, out kMKI) == false)
            {
                Logger.LogError("Can not find master key name: " + strMasterKeyName);
                return null;
            }

            KeyPath kPath;
            ExtKey kNewKey;

            try
            {
                kPath = new KeyPath(strPath);
                //kNewKey = kMKI.kParentKey.Derive(kPath);
                kNewKey = kMKI.kMasterKey.Derive(kPath);
            }
            catch ( Exception e )
            {
                Logger.LogError(e.Message);
                return null;
            }

            return kNewKey;
        }

        // return MagnaChain address
        public string GenerateNewMCLAddress(string strMasterKeyName)
        {
            string strPath = "";
            if (string.IsNullOrEmpty(strMasterKeyName))
            {
                return null;
            }

            MASTER_KEY_INFO kMKI;

            if (m_mapMKI.TryGetValue(strMasterKeyName, out kMKI) == false)
            {
                Logger.LogError("Can not find master key name: " + strMasterKeyName);
                return null;
            }

            KeyPath kPath;
            ExtKey kNewKey;
            string strBTAddress;

            if (kMKI.uiCurSubKeyIndex == 0xFFFFFFFF)
            {
                // first child
                kMKI.uiCurSubKeyIndex = 0;
                string strSubPath = kMKI.uiCurSubKeyIndex.ToString();
                strPath = kMKI.strParentPath + strSubPath;

                try
                {
                    kPath = new KeyPath(strSubPath);
                    kNewKey = kMKI.kParentKey.Derive(kPath);
                    strBTAddress = kNewKey.PrivateKey.PubKey.GetAddress(m_kCLNet).ToWif();
                    return EncodeMCLAddress(strMasterKeyName, strPath, strBTAddress);
                }
                catch (Exception e)
                {
                    Logger.LogError(e.Message);
                    return null;
                }
            }
            else if (kMKI.uiCurSubKeyIndex < MAX_EXT_KEY_INDEX)
            {
                kMKI.uiCurSubKeyIndex += 1;
                string strSubPath = kMKI.uiCurSubKeyIndex.ToString();
                strPath = kMKI.strParentPath + strSubPath;

                try
                {
                    kPath = new KeyPath(strSubPath);
                    kNewKey = kMKI.kParentKey.Derive(kPath);
                    strBTAddress = kNewKey.PrivateKey.PubKey.GetAddress(m_kCLNet).ToWif();
                    return EncodeMCLAddress(strMasterKeyName, strPath, strBTAddress);
                }
                catch (Exception e)
                {
                    Logger.LogError(e.Message);
                    return null;
                }
            }
            else
            {
                kMKI.uiCurSubKeyIndex = 0;

                // check parent brother
                if (kMKI.ucCurDepth == 2)
                {
                    // new parent path                    
                    kMKI.ucCurDepth += 1;
                    kMKI.strParentPath = kMKI.strParentPath + "0/";
                    strPath = kMKI.strParentPath + "0";

                    try
                    {
                        kPath = new KeyPath("0");
                        kMKI.kParentKey = kMKI.kParentKey.Derive(kPath);
                        kNewKey = kMKI.kParentKey.Derive(kPath);
                        strBTAddress = kNewKey.PrivateKey.PubKey.GetAddress(m_kCLNet).ToWif();
                        return EncodeMCLAddress(strMasterKeyName, strPath, strBTAddress);
                    }
                    catch (Exception e)
                    {
                        Logger.LogError(e.Message);
                        return null;
                    }
                }
                else
                {
                    int iL = kMKI.strParentPath.LastIndexOf("/", kMKI.strParentPath.Length - 2);
                    if (iL == -1)
                    {
                        Logger.LogError("Error parent path: " + kMKI.strParentPath);
                        return null;
                    }

                    string strG = kMKI.strParentPath.Substring(0, iL + 1);
                    string strK = kMKI.strParentPath.Substring(iL + 1);
                    strK = strK.Replace("/", "");
                    strK = strK.Replace("'", "");

                    uint uiParentIndex = 0;
                    try
                    {
                        uiParentIndex = Convert.ToUInt32(strK);
                    }
                    catch (Exception e)
                    {
                        Logger.LogError(e.Message);
                        return null;
                    }

                    if (uiParentIndex < MAX_EXT_KEY_INDEX)
                    {
                        // add parent brother node
                        uiParentIndex += 1;
                        string strNewBrotherParentPath = strG + uiParentIndex.ToString();
                        kMKI.strParentPath = strNewBrotherParentPath + "/";
                        strPath = kMKI.strParentPath + "0";

                        try
                        {
                            kPath = new KeyPath(strNewBrotherParentPath);
                            kMKI.kParentKey = kMKI.kMasterKey.Derive(kPath);
                            kPath = new KeyPath("0");
                            kNewKey = kMKI.kParentKey.Derive(kPath);
                            strBTAddress = kNewKey.PrivateKey.PubKey.GetAddress(m_kCLNet).ToWif();
                            return EncodeMCLAddress(strMasterKeyName, strPath, strBTAddress);
                        }
                        catch (Exception e)
                        {
                            Logger.LogError(e.Message);
                            return null;
                        }
                    }
                    else
                    {
                        // add depth
                        kMKI.ucCurDepth += 1;

                        string strNewParentPath = strG + "0/0";
                        kMKI.strParentPath = strNewParentPath + "/";
                        strPath = kMKI.strParentPath + "0";

                        try
                        {
                            kPath = new KeyPath(strNewParentPath);
                            kMKI.kParentKey = kMKI.kMasterKey.Derive(kPath);
                            kPath = new KeyPath("0");
                            kNewKey = kMKI.kParentKey.Derive(kPath);
                            strBTAddress = kNewKey.PrivateKey.PubKey.GetAddress(m_kCLNet).ToWif();
                            return EncodeMCLAddress(strMasterKeyName, strPath, strBTAddress);
                        }
                        catch (Exception e)
                        {
                            Logger.LogError(e.Message);
                            return null;
                        }
                    }
                }
            }
        }

        private string ResolveCmdAndArgs(string strCommand, List<string> listArgs)
        {
            if (string.IsNullOrEmpty(strCommand))
            {
                return null;
            }

            char[] arrSpt = new char[1];
            arrSpt[0] = ' ';
            string[] arrSeg = strCommand.Split(arrSpt);

            if (arrSeg.Length > 1)
            {
                int i;

                for (i = 1; i < arrSeg.Length; i++)
                {
                    listArgs.Add(arrSeg[i]);
                }
            }
            return arrSeg[0];
        }

        // args must be string
        public RPCResponse SendCommand(string strCommand)
        {
            if (m_kRPC == null)
            {
                return null;
            }

            if (string.IsNullOrEmpty(strCommand))
            {
                return null;
            }

            List<string> listParams = new List<string>();
            string strCmd = ResolveCmdAndArgs(strCommand, listParams);
            return SendCommand(strCmd, listParams.ToArray());
        }

        // args must be string
        public void SendCommandAsync(string strCommand, ON_COMMAND_DONE fnCmdDone)
        {
            if (m_kRPC == null)
            {
                if (fnCmdDone != null)
                {
                    fnCmdDone(null);
                }
                return;
            }

            if (string.IsNullOrEmpty(strCommand))
            {
                if (fnCmdDone != null)
                {
                    fnCmdDone(null);
                }
                return;
            }

            List<string> listParams = new List<string>();
            string strCmd = ResolveCmdAndArgs(strCommand, listParams);
            SendCommandAsync(strCmd, listParams.ToArray(), fnCmdDone);
        }

        // Execute rpc command
        public RPCResponse SendCommand(string strOP, object[] arrParams)
        {
            if (m_kRPC == null)
            {
                return null;
            }

            try
            {
                RPCResponse kRsp =  m_kRPC.SendCommand(strOP, arrParams);
                return kRsp;
            }
            catch ( Exception e)
            {
                Logger.LogError(e.Message);
                return null;
            }
        }

        public void SendCommandAsync(string strOP, object[] arrParams, ON_COMMAND_DONE fnCmdDone)
        {
            if (m_kRPC == null)
            {
                if (fnCmdDone != null)
                {
                    fnCmdDone(null);
                }
                return;
            }

            lock(m_listACI)
            {
                ASYNC_CMD_INFO kACI = new ASYNC_CMD_INFO();
                kACI.strCmd = strOP;
                kACI.arrParams = arrParams;
                kACI.fnCmdDone = fnCmdDone;
                m_listACI.Add(kACI);
            }

           // Action<Task<RPCResponse>> fnFinish = delegate (Task<RPCResponse> kT)
           //{               
           //    if (fnCmdDone != null)
           //    {
           //        fnCmdDone(kT.Result);                   
           //    }
           //};

           // try
           // {
           //     Task<RPCResponse> kTask = m_kRPC.SendCommandAsync(strOP, arrParams);
           //     kTask.ContinueWith(fnFinish, TaskContinuationOptions.ExecuteSynchronously);
           // }
           // catch (Exception e)
           // {
           //     Logger.LogError(e.Message);

           //     if (fnCmdDone != null)
           //     {
           //         fnCmdDone(null);
           //     }
           // }
        }

        public float GetBalance(string strMCLAddress)
        {
            if (string.IsNullOrEmpty(strMCLAddress))
            {
                return 0.0f;
            }

            string strName;
            string strPath;
            string strBTAddress;

            DecodeMCLAddress(strMCLAddress, out strName, out strPath, out strBTAddress);
            return GetBalanceByCelAddress(strBTAddress);
        }

        public void GetBalanceAsync(string strMCLAddress, ON_GET_BALANCE fnGetBalance)
        {
            if (string.IsNullOrEmpty(strMCLAddress))
            {
                if (fnGetBalance != null)
                {
                    fnGetBalance(0.0f);
                }
                return;
            }

            string strName;
            string strPath;
            string strBTAddress;

            DecodeMCLAddress(strMCLAddress, out strName, out strPath, out strBTAddress);
            GetBalanceByCelAddressAsync(strBTAddress, fnGetBalance);
        }

        public float GetBalanceByCelAddress(string strCelAddress)
        {
            if (m_kRPC == null)
            {
                return 0.0f;
            }

            if (string.IsNullOrEmpty(strCelAddress))
            {
                return 0.0f;
            }

            RPCResponse kRsp = SendCommand("getbalanceof " + strCelAddress);
            if ( kRsp == null )
            {
                Logger.LogError("getbalanceof failed!");
                return 0.0f;
            }

            if (kRsp.Error != null)
            {
                Logger.LogError("GetBalanceByBTAddress error: " + kRsp.Error.Code + " msg: " + kRsp.Error.Message);
                return 0.0f;
            }

            float fV = 0.0f;
            try
            {
                fV = Convert.ToSingle(kRsp.ResultString);
                return fV * 0.00000001f;
            }
            catch (Exception e)
            {
                Logger.LogError(e.Message);
                return 0.0f;
            }
        }

        public void GetBalanceByCelAddressAsync(string strCelAddress, ON_GET_BALANCE fnGetBalance)
        {
            if (m_kRPC == null)
            {
                if (fnGetBalance != null)
                {
                    fnGetBalance(0.0f);
                }
                return;
            }

            if (string.IsNullOrEmpty(strCelAddress))
            {
                if (fnGetBalance != null)
                {
                    fnGetBalance(0.0f);
                }
                return;
            }

            ON_COMMAND_DONE fnCmdDone = delegate (RPCResponse kRsp)
            {
                if (kRsp == null)
                {
                    Logger.LogError("GetBalanceByBTAddressAsync error!");
                    if (fnGetBalance != null)
                    {
                        fnGetBalance(0.0f);
                    }
                    return;
                }
                if (kRsp.Error != null)
                {
                    Logger.LogError("GetBalanceByBTAddressAsync error: " + kRsp.Error.Code + " msg: " + kRsp.Error.Message);

                    if (fnGetBalance != null)
                    {
                        fnGetBalance(0.0f);
                    }
                    return;
                }

                float fV = 0.0f;
                try
                {
                    fV = Convert.ToSingle(kRsp.ResultString);
                    if (fnGetBalance != null)
                    {
                        fnGetBalance(fV * 0.00000001f);
                    }
                }
                catch (Exception e)
                {
                    Logger.LogError(e.Message);
                    if (fnGetBalance != null)
                    {
                        fnGetBalance(fV * 0.00000001f);
                    }
                }
            };
            SendCommandAsync("getbalanceof " + strCelAddress, fnCmdDone);
        }

        private float ComputeFee(List<Coin> listCoins)
        {
            if ( listCoins == null )
            {
                return 0.0f;
            }
            return UnityEngine.Random.Range(0.0001f, 0.00001f)*100;// Ray has increase DEFAULT_MIN_RELAY_TX_FEE 
        }
        
        private string GenerateRawTransaction(RPCResponse kRsp, Key kPriKey, string strDestAddress, string strChangeAddress, float fMoney, float fFee, out string strTxid)
        {
            strTxid = null;
            if ( fMoney <= 0.0f )
            {
                return null;
            }    
            if (kPriKey == null || string.IsNullOrEmpty(strDestAddress) )
            {
                return null;
            }
            if (kRsp == null || kRsp.Result.Type != JTokenType.Array)
            {
                return null;
            }            

            List<Coin> listCoins = new List<Coin>();
            foreach (var v in kRsp.Result)
            {
                uint256 fromTxHash = uint256.Parse((string)v["txhash"]);
                uint fromOutputIndex = uint.Parse((string)v["outn"]);
                Money amount = Money.Parse((string)v["value"]);
                Script scriptPubKey = new Script(Encoders.Hex.DecodeData((string)v["script"]));
                listCoins.Add(new Coin(fromTxHash, fromOutputIndex, amount, scriptPubKey));
            }

            if ( fFee < 0.0f )
            {
                fFee = ComputeFee(listCoins);
            }

            //BitcoinSecret bitsecret = new BitcoinSecret(strPriKey);
            List<Key> keys = new List<Key>();
            keys.Add(kPriKey);

            StandardTransactionPolicy EasyPolicy = new StandardTransactionPolicy()
            {
                MaxTransactionSize = null,
                MaxTxFee = null,
                MinRelayTxFee = null,
                ScriptVerify = ScriptVerify.Standard & ~ScriptVerify.LowS
            };

            BitcoinAddress kDestAddress = BitcoinAddress.Create(strDestAddress, m_kCLNet);

            BitcoinAddress kChangeAddress;

            if ( string.IsNullOrEmpty(strChangeAddress))
            {
                kChangeAddress = kPriKey.PubKey.GetAddress(m_kCLNet);
            }
            else
            {
                kChangeAddress = BitcoinAddress.Create(strChangeAddress, m_kCLNet);
            }

            TransactionBuilder txBuilder = new TransactionBuilder(0);
            txBuilder.StandardTransactionPolicy = EasyPolicy;

            string strMoney = ((decimal)fMoney).ToString();

            Money kMoney;

            if ( Money.TryParse(strMoney, out kMoney) == false )
            {
                Logger.LogError("Money.TryParse failed(Money): " + strMoney);
                return null;
            }

            Money kFee;

            string strFee = ((decimal)fFee).ToString();
            if (Money.TryParse(strFee, out kFee) == false)
            {
                Logger.LogError("Money.TryParse failed(Fee): " + strFee);
                return null;
            }

            Transaction tx;

            try
            {
                tx = txBuilder
                                .AddCoins(listCoins.ToArray())
                                .AddKeys(keys.ToArray())
                                .Send(kDestAddress, kMoney)
                                .SendFees(kFee)
                                .SetChange(kChangeAddress)
                                .BuildTransaction(false);
                tx.Version = 2; // current transaction version
                tx = txBuilder.SignTransactionInPlace(tx);
            }
            catch ( Exception e)
            {
                Logger.LogError(e.Message);
                return null;
            }            

            TransactionPolicyError[] errors;

            if (txBuilder.Verify(tx, (Money)null, out errors))
            {
                strTxid = tx.GetHash().ToString();
                return tx.ToHex();
            }
            else
            {
                string strError = "Build transaction verify fail:\n";
                foreach (var err in errors)
                {
                    strError += err.ToString() + "\n";
                }
                Logger.LogError(strError);
                return null;
            }
        }    

        //private void ForTransfer(RPCResponse kRsp, string strPriKey, string strDestAddress, string strChangeAddress, float fMoney, float fFee)
        //{                  
        //    if (kRsp.Result != null && kRsp.Result.Type == JTokenType.Array)
        //    {
        //        List<Coin> listCoins = new List<Coin>();
        //        foreach (var v in kRsp.Result)
        //        {
        //            uint256 fromTxHash = uint256.Parse((string)v["txhash"]);
        //            uint fromOutputIndex = uint.Parse((string)v["outn"]);
        //            Money amount = Money.Parse((string)v["value"]);
        //            Script scriptPubKey = new Script(Encoders.Hex.DecodeData((string)v["script"]));
        //            listCoins.Add(new Coin(fromTxHash, fromOutputIndex, amount, scriptPubKey));
        //        }

        //        BitcoinSecret bitsecret = new BitcoinSecret("L1amKPbuEpYwqU2uVRb34i2oUAa328RvNaEoKrCqgdEPLgV6MDcz");
        //        List<Key> keys = new List<Key>();
        //        keys.Add(bitsecret.PrivateKey);

        //        StandardTransactionPolicy EasyPolicy = new StandardTransactionPolicy()
        //        {
        //            MaxTransactionSize = null,
        //            MaxTxFee = null,
        //            MinRelayTxFee = null,
        //            ScriptVerify = ScriptVerify.Standard & ~ScriptVerify.LowS
        //        };

        //        BitcoinAddress dest = BitcoinAddress.Create("XD7SstxYNeBCUzLkVre3gP1ipUjJBGTxRB", m_kCLNet);

        //        TransactionBuilder txBuilder = new TransactionBuilder(0);
        //        txBuilder.StandardTransactionPolicy = EasyPolicy;
        //        Transaction tx = txBuilder
        //            .AddCoins(listCoins.ToArray())
        //            .AddKeys(keys.ToArray())
        //            .Send(dest, Money.Parse("0.5"))
        //            .SendFees(Money.Parse("0.0001"))
        //            .SetChange(dest)
        //            .BuildTransaction(false);
        //        tx.Version = 2; // current transaction version
        //        tx = txBuilder.SignTransactionInPlace(tx);

        //        TransactionPolicyError[] errors;
        //        if (txBuilder.Verify(tx, (Money)null, out errors))
        //        {
        //            string strSendRawTransaction = "sendrawtransaction " + tx.ToHex();
        //            RPCResponse kRsp2 = SendCommand(strSendRawTransaction);
        //            if (kRsp2 == null)
        //            {
        //                Logger.LogError("sendrawtransaction command error!");
        //            }
        //            else
        //            {
        //                if (kRsp2.Error == null)
        //                {
        //                    Logger.LogSys(kRsp2.ResultString);
        //                }
        //                else
        //                {
        //                    Logger.LogError("Error: " + kRsp2.Error.Code + " msg: " + kRsp2.Error.Message);
        //                }
        //            }
        //        }
        //        else
        //        {
        //            string strError = "build transaction verify fail:\n";
        //            foreach (var err in errors)
        //            {
        //                strError += err.ToString() + "\n";
        //            }
        //            Logger.LogError(strError);
        //        }
        //    }
        //}

        public bool TransferRaw(out RPCError kError, string strFromPriKey, string strToCelAddress, float fMoney, out string strTxid, float fFee = -1.0f, string strChangeCelAddress = null)
        {
            strTxid = null;
            kError = null;
            if (string.IsNullOrEmpty(strFromPriKey) || string.IsNullOrEmpty(strToCelAddress) || fMoney <= 0.0f)
            {
                kError = new RPCError(RPCErrorCode.RPC_CUSTOM_ERROR, "Invalid params!");
                return false;
            }

            Key kPriKey;

            try
            {
                kPriKey = Key.Parse(strFromPriKey, m_kCLNet);
            }
            catch ( Exception e)
            {
                kError = new RPCError(RPCErrorCode.RPC_CUSTOM_ERROR, "Parse private key error!");

                Logger.LogError(e.Message);
                return false;
            }

            string strFromBTAddress = kPriKey.PubKey.ToString(m_kCLNet);
            string strQueryBalance = "getaddresscoins " + strFromBTAddress + " true";
            RPCResponse kRsp = SendCommand(strQueryBalance);
            if (kRsp == null)
            {
                kError = new RPCError(RPCErrorCode.RPC_CUSTOM_ERROR, "getaddresscoins command error!");

                Logger.LogError("getaddresscoins command error!");
                return false;
            }
            if (kRsp.Error != null)
            {
                kError = kRsp.Error;

                Logger.LogError("getaddresscoins command error: " + kRsp.Error.Code + " msg: " + kRsp.Error.Message);
                return false;
            }

            string strTrsHex = GenerateRawTransaction(kRsp, kPriKey, strToCelAddress, strChangeCelAddress, fMoney, fFee, out strTxid);
            if (string.IsNullOrEmpty(strTrsHex))
            {
                kError = new RPCError(RPCErrorCode.RPC_CUSTOM_ERROR, "GenerateRawTransaction failed!");

                return false;
            }

            string strSendRawTransaction = "sendrawtransaction " + strTrsHex;
            kRsp = SendCommand(strSendRawTransaction);
            if (kRsp == null)
            {
                kError = new RPCError(RPCErrorCode.RPC_CUSTOM_ERROR, "sendrawtransaction command error!");

                Logger.LogError("sendrawtransaction command error!");
                return false;
            }

            if (kRsp.Error == null)
            {
                Logger.Log(kRsp.ResultString);
                return true;
            }
            else
            {
                kError = kRsp.Error;

                Logger.LogError("Error: " + kRsp.Error.Code + " msg: " + kRsp.Error.Message);
                return false;
            }
        }

        public bool TransferMCLToMCLAddress(out RPCError kError, string strFromMCLAddress, string strToMCLAddress, float fMoney, out string strTxid, float fFee = -1.0f, string strChangeMCLAddress = null)
        {
            strTxid = null;
            kError = null;
            if ( string.IsNullOrEmpty(strToMCLAddress))
            {
                kError = new RPCError(RPCErrorCode.RPC_CUSTOM_ERROR, "Invalid params!");
                return false;
            }

            string strToBTAddress;
            string strToPath;
            string strName;

            if ( DecodeMCLAddress(strToMCLAddress, out strName, out strToPath, out strToBTAddress) == false )
            {
                kError = new RPCError(RPCErrorCode.RPC_CUSTOM_ERROR, "DecodeMCLAddress failed!");
                return false;
            }

            string strChangeBTAddress = null;
            string strChangePath;

            if ( !string.IsNullOrEmpty(strChangeMCLAddress))
            {
                if ( DecodeMCLAddress(strChangeMCLAddress, out strName, out strChangePath, out strChangeBTAddress) == false )
                {
                    kError = new RPCError(RPCErrorCode.RPC_CUSTOM_ERROR, "DecodeMCLAddress failed!");
                    return false;
                }
            }

            return TransferMCLToCelAddress(out kError, strFromMCLAddress, strToBTAddress, fMoney, out strTxid, fFee, strChangeBTAddress);
        }

        public bool TransferMCLToCelAddress(out RPCError kError, string strFromMCLAddress, string strToCelAddress, float fMoney, out string strTxid, float fFee = -1.0f, string strChangeCelAddress = null)
        {
            strTxid = null;
            kError = null;
            if ( string.IsNullOrEmpty(strFromMCLAddress) || string.IsNullOrEmpty(strToCelAddress))
            {
                kError = new RPCError(RPCErrorCode.RPC_CUSTOM_ERROR, "Invalid params!");

                return false;
            }

            string strFromBTAddress;
            string strFromPath;
            string strName;

            DecodeMCLAddress(strFromMCLAddress, out strName, out strFromPath, out strFromBTAddress);
            ExtKey kFrmoKey = GetExtKeyByPath(strName, strFromPath);
            if ( kFrmoKey == null )
            {
                kError = new RPCError(RPCErrorCode.RPC_CUSTOM_ERROR, "GetExtKeyByPath error!");

                return false;
            }

            return TransferRaw(out kError, kFrmoKey.PrivateKey.GetWif(m_kCLNet).ToWif(), strToCelAddress, fMoney, out strTxid, fFee, strChangeCelAddress);
        }     

        // strChangeBTAddress: 找零地址，为空则找零回发送地址
        // fFee: 手续费，为负则自动计算
        public void TransferRawAsync(string strFromPriKey, string strToCelAddress, float fMoney, ON_TRANSFER_DONE fnTransferDone = null, float fFee = -1.0f, string strChangeCelAddress = null)
        {
            if (string.IsNullOrEmpty(strFromPriKey) || string.IsNullOrEmpty(strToCelAddress) || fMoney <= 0.0f)
            {
                if ( fnTransferDone != null )
                {
                    RPCError kError = new RPCError(RPCErrorCode.RPC_CUSTOM_ERROR, "Invalid params!");
                    fnTransferDone(kError, null);
                }
                return;
            }

            Key kPriKey;

            try
            {
                kPriKey = Key.Parse(strFromPriKey, m_kCLNet);
            }
            catch (Exception e)
            {
                Logger.LogError(e.Message);

                if (fnTransferDone != null)
                {
                    RPCError kError = new RPCError(RPCErrorCode.RPC_CUSTOM_ERROR, "Parse private key error!");
                    fnTransferDone(kError, null);
                }
                return;
            }

            ON_COMMAND_DONE fnSendRawTransaction = delegate(RPCResponse kRsp)
            {
                if (kRsp == null)
                {
                    Logger.LogError("sendrawtransaction command error!");

                    if (fnTransferDone != null)
                    {
                        RPCError kError = new RPCError(RPCErrorCode.RPC_CUSTOM_ERROR, "sendrawtransaction command error!");
                        fnTransferDone(kError, null);
                    }
                    return;
                }

                if (kRsp.Error == null)
                {
                    Logger.Log(kRsp.ResultString);

                    if (fnTransferDone != null)
                    {                        
                        fnTransferDone(null, kRsp.ResultString);
                    }
                }
                else
                {
                    Logger.LogError("Error: " + kRsp.Error.Code + " msg: " + kRsp.Error.Message);

                    if (fnTransferDone != null)
                    {                        
                        fnTransferDone(kRsp.Error, null);
                    }
                }
            };

            ON_COMMAND_DONE fnQueryDone = delegate (RPCResponse kRsp)
            {
                if (kRsp == null)
                {
                    Logger.LogError("getaddresscoins command error!");

                    if (fnTransferDone != null)
                    {
                        RPCError kError = new RPCError(RPCErrorCode.RPC_CUSTOM_ERROR, "getaddresscoins command error!");
                        fnTransferDone(kError, null);
                    }
                    return;
                }

                if (kRsp.Error != null)
                {
                    Logger.LogError("getaddresscoins command error: " + kRsp.Error.Code + " msg: " + kRsp.Error.Message);

                    if (fnTransferDone != null)
                    {                        
                        fnTransferDone(kRsp.Error, null);
                    }
                    return;
                }

                string strTxid;

                string strTrsHex = GenerateRawTransaction(kRsp, kPriKey, strToCelAddress, strChangeCelAddress, fMoney, fFee, out strTxid);
                if (string.IsNullOrEmpty(strTrsHex))
                {
                    if (fnTransferDone != null)
                    {
                        RPCError kError = new RPCError(RPCErrorCode.RPC_CUSTOM_ERROR, "Txid is null!");
                        fnTransferDone(kError, null);
                    }
                    return;
                }

                string strSendRawTransaction = "sendrawtransaction " + strTrsHex;
                //kRsp = SendCommand(strSendRawTransaction);
                SendCommandAsync(strSendRawTransaction, fnSendRawTransaction);                
            };

            string strFromBTAddress = kPriKey.PubKey.ToString(m_kCLNet);
            string strQueryBalance = "getaddresscoins " + strFromBTAddress + " true";
            //RPCResponse kRsp = SendCommand(strQueryBalance);
            SendCommandAsync(strQueryBalance, fnQueryDone);
        }

        public void TransferMCLToCelAddressAsync(string strFromMCLAddress, string strToCelAddress, float fMoney, ON_TRANSFER_DONE fnTransferDone, float fFee = -1.0f, string strChangeCelAddress = null)
        {
            if (string.IsNullOrEmpty(strFromMCLAddress) || string.IsNullOrEmpty(strToCelAddress) || fMoney <= 0.0f)
            {
                if (fnTransferDone != null)
                {
                    RPCError kError = new RPCError(RPCErrorCode.RPC_CUSTOM_ERROR, "Invalid params!");
                    fnTransferDone(kError, null);
                }
                return;
            }

            string strFromBTAddress;
            string strFromPath;
            string strName;

            DecodeMCLAddress(strFromMCLAddress, out strName, out strFromPath, out strFromBTAddress);
            ExtKey kFrmoKey = GetExtKeyByPath(strName, strFromPath);
            if (kFrmoKey == null)
            {
                Logger.LogError("Can not find ext key, name: " + strName + " path: " + strFromPath);
                if (fnTransferDone != null)
                {
                    RPCError kError = new RPCError(RPCErrorCode.RPC_CUSTOM_ERROR, "Not find ext key!");
                    fnTransferDone(kError, null);
                }
                return;
            }

            TransferRawAsync(kFrmoKey.PrivateKey.GetWif(m_kCLNet).ToWif(), strToCelAddress, fMoney, fnTransferDone, fFee, strChangeCelAddress);
        }

        public void TransferMCLToMCLAddressAsync(string strFromMCLAddress, string strToMCLAddress, float fMoney, ON_TRANSFER_DONE fnTransferDone, float fFee = -1.0f, string strChangeMCLAddress = null)
        {            
            if (string.IsNullOrEmpty(strToMCLAddress))
            {
                if (fnTransferDone != null)
                {
                    RPCError kError = new RPCError(RPCErrorCode.RPC_CUSTOM_ERROR, "Invalid params!");
                    fnTransferDone(kError, null);
                }
                return;
            }

            string strToBTAddress;
            string strToPath;
            string strName;

            if (DecodeMCLAddress(strToMCLAddress, out strName, out strToPath, out strToBTAddress) == false)
            {
                if (fnTransferDone != null)
                {
                    RPCError kError = new RPCError(RPCErrorCode.RPC_CUSTOM_ERROR, "Decode dest CL address error!");
                    fnTransferDone(kError, null);
                }
                return;
            }

            string strChangeBTAddress = null;
            string strChangePath;

            if (!string.IsNullOrEmpty(strChangeMCLAddress))
            {
                if (DecodeMCLAddress(strChangeMCLAddress, out strName, out strChangePath, out strChangeBTAddress) == false)
                {
                    if (fnTransferDone != null)
                    {
                        RPCError kError = new RPCError(RPCErrorCode.RPC_CUSTOM_ERROR, "Decode change CL address error!");
                        fnTransferDone(kError, null);
                    }
                    return;
                }
            }

            TransferMCLToCelAddressAsync(strFromMCLAddress, strToBTAddress, fMoney, fnTransferDone, fFee, strChangeBTAddress);
        }

        private void ResolvePublishContractResult(string strResult, out string strContractAddress, out string strSendAddress, out string strCode)
        {
            strContractAddress = null;
            strSendAddress = null;
            strCode = null;

            try
            {
                //Debug.Log(kRsp.ResultString);
                JObject kJO = (JObject)JsonConvert.DeserializeObject(strResult);

                JToken kVal;

                if (kJO.TryGetValue("contractaddress", out kVal) == false)
                {
                    Logger.LogError("Can not find contractaddress, return json: " + strResult);
                    return;
                }
                else
                {
                    strContractAddress = kVal.ToString();
                }

                if (kJO.TryGetValue("senderaddress", out kVal) == false)
                {
                    Logger.LogError("Can not find senderaddress, return json: " + strResult);
                    return;
                }
                else
                {
                    strSendAddress = kVal.ToString();
                }

                if (kJO.TryGetValue("code", out kVal) == false)
                {
                    Logger.LogError("Can not find code, return json: " + strResult);
                    return;
                }
                else
                {
                    strCode = kVal.ToString();
                }
                return;
            }
            catch (Exception e)
            {
                Logger.LogError(e.Message);
                return;
            }
        }

        static private void GetCoinsFromJoken(JToken jtoken, List<Coin> listCoins)
        {
            if (jtoken != null && jtoken.Type == Newtonsoft.Json.Linq.JTokenType.Array && listCoins != null)
            {
                foreach (var v in jtoken)
                {
                    uint256 fromTxHash = uint256.Parse((string)v["txhash"]);
                    uint fromOutputIndex = uint.Parse((string)v["outn"]);
                    Money amount = Money.Parse((string)v["value"]);
                    Script scriptPubKey = new Script(Encoders.Hex.DecodeData((string)v["script"]));
                    listCoins.Add(new Coin(fromTxHash, fromOutputIndex, amount, scriptPubKey));
                }
            }
        }

        private bool SignAndSendTransaction(JToken txhex, JToken jtCoins, List<Key> keys)
        {
            if ( txhex == null || keys == null )
            {
                return false;
            }

            try
            {
                Transaction tx = Transaction.Parse((string)txhex);

                TransactionBuilder txBuilder = new TransactionBuilder(0);
                txBuilder.StandardTransactionPolicy = ms_kEasyPolicy;
                txBuilder.AddKeys(keys.ToArray());

                if (jtCoins != null)
                {
                    List<Coin> txCoins = new List<Coin>();
                    GetCoinsFromJoken(jtCoins, txCoins);
                    txBuilder.AddCoins(txCoins.ToArray());
                }

                tx = txBuilder.SignTransactionInPlace(tx);

                Transaction ntx = new Transaction(tx.ToBytes());
                if (tx.GetHash() != ntx.GetHash())
                {
                    return false;
                }
                //Debug.Assert(tx.GetHash() == ntx.GetHash());

                TransactionPolicyError[] errors;

                if (txBuilder.Verify(tx, (Money)null, out errors))
                {
                    //RPCResponse kRsp2 = RpcCall("sendrawtransaction", tx.ToHex());  //m_kRPC.SendCommand(strSendRawTransaction, params2.ToArray());
                    object[] arrArg = new object[1];
                    arrArg[0] = tx.ToHex();
                    RPCResponse kRsp = SendCommand("sendrawtransaction", arrArg);
                    if (kRsp == null)
                    {
                        return false;
                    }
                    if (kRsp.Error != null)
                    {
                        Logger.LogError("sendrawtransaction failed: " + kRsp.Error.Code + " msg: " + kRsp.Error.Message);
                        return false;
                    }
                    return true;
                }
                else
                {
                    string strError = "";
                    int i;

                    for ( i = 0; i < errors.Length; i++ )
                    {
                        strError += errors[i] + "\t";
                    }

                    Logger.LogError("Build transaction verify fail: " + strError);
                    return false;
                }
            }
            catch ( Exception e)
            {
                Logger.LogError(e.Message);
                return false;
            }
        }

        private void SignAndSendTransactionAsync(JToken txhex, JToken jtCoins, List<Key> keys, Action<bool> fnFinish)
        {
            if (txhex == null || keys == null)
            {
                if ( fnFinish != null )
                {
                    fnFinish(false);
                }
                return;
            }

            try
            {
                Transaction tx = Transaction.Parse((string)txhex);

                TransactionBuilder txBuilder = new TransactionBuilder(0);
                txBuilder.StandardTransactionPolicy = ms_kEasyPolicy;
                txBuilder.AddKeys(keys.ToArray());

                if (jtCoins != null)
                {
                    List<Coin> txCoins = new List<Coin>();
                    GetCoinsFromJoken(jtCoins, txCoins);
                    txBuilder.AddCoins(txCoins.ToArray());
                }

                tx = txBuilder.SignTransactionInPlace(tx);

                Transaction ntx = new Transaction(tx.ToBytes());
                if (tx.GetHash() != ntx.GetHash())
                {
                    if (fnFinish != null)
                    {
                        fnFinish(false);
                    }
                    return;
                }
                //Debug.Assert(tx.GetHash() == ntx.GetHash());

                TransactionPolicyError[] errors;

                if (txBuilder.Verify(tx, (Money)null, out errors))
                {
                    //RPCResponse kRsp2 = RpcCall("sendrawtransaction", tx.ToHex());  //m_kRPC.SendCommand(strSendRawTransaction, params2.ToArray());
                    object[] arrArg = new object[1];
                    arrArg[0] = tx.ToHex();

                    ON_COMMAND_DONE fnSendRawTransactionDone = delegate(RPCResponse kRsp)
                    {
                        if (kRsp == null)
                        {
                            if (fnFinish != null)
                            {
                                fnFinish(false);
                            }
                            return;
                        }
                        if (kRsp.Error != null)
                        {
                            Logger.LogError("sendrawtransaction failed: " + kRsp.Error.Code + " msg: " + kRsp.Error.Message);

                            if (fnFinish != null)
                            {
                                fnFinish(false);
                            }
                            return;
                        }


                        if (fnFinish != null)
                        {
                            fnFinish(true);
                        }
                    };

                    SendCommandAsync("sendrawtransaction", arrArg, fnSendRawTransactionDone);                    
                }
                else
                {
                    string strError = "";
                    int i;

                    for (i = 0; i < errors.Length; i++)
                    {
                        strError += errors[i] + "\t";
                    }

                    Logger.LogError("Build transaction verify fail: " + strError);

                    if (fnFinish != null)
                    {
                        fnFinish(false);
                    }
                }
            }
            catch (Exception e)
            {
                Logger.LogError(e.Message);

                if (fnFinish != null)
                {
                    fnFinish(false);
                }
            }
        }

        public void PublishContractAsync(ON_PUBLISH_CONTRACT_DONE fnDone, string strContractContent, Key kCostPriKey, float fCostAmount, Key kSenderPriKey, string strChargeCelAddress = null, object kArg = null)
        {            
            if (string.IsNullOrEmpty(strContractContent) || kCostPriKey == null || kSenderPriKey == null)
            {
                if ( fnDone != null )
                {
                    fnDone(false, null, kArg);
                }
                return;
            }

            if (fCostAmount <= 0.0f)
            {
                if (fnDone != null)
                {
                    fnDone(false, null, kArg);
                }
                return;
            }

            string strCostFromAddress = kCostPriKey.PubKey.GetAddress(m_kCLNet).ToWif();
            string strSenderAddressHex = kSenderPriKey.PubKey.ToHex();

            if (string.IsNullOrEmpty(strChargeCelAddress))
            {
                strChargeCelAddress = strCostFromAddress;
            }

            string strContractHex = HexEncoder.HexStr(strContractContent, false);            

            object[] arrArgs = new object[5];
            arrArgs[0] = strContractHex;
            arrArgs[1] = strCostFromAddress;
            arrArgs[2] = strSenderAddressHex;
            arrArgs[3] = ((decimal)fCostAmount).ToString();
            arrArgs[4] = strChargeCelAddress;

            ON_COMMAND_DONE fnPublishDone = delegate (RPCResponse kRsp)
            {
                if (kRsp == null)
                {
                    if (fnDone != null)
                    {
                        fnDone(false, null, kArg);
                    }
                    return;
                }
                if (kRsp.Error != null)
                {
                    Logger.LogError("prepublishcode error: " + kRsp.Error.Code + " msg: " + kRsp.Error.Message);

                    if (fnDone != null)
                    {
                        fnDone(false, null, kArg);
                    }
                    return;
                }

                //private keys, two need
                List<Key> keys = new List<Key>();                        

                keys.Add(kCostPriKey);
                keys.Add(kSenderPriKey);

                JToken kTX = kRsp.Result["txhex"];
                JToken kCoin = kRsp.Result["coins"];

                Transaction tx = Transaction.Parse((string)kTX);
                //string strContractAddress = tx.TContractData.Address.GetAddress(m_kCLNet).ToWif();
                BitcoinContractPubKeyAddress bitContractPK = new BitcoinContractPubKeyAddress(tx.TContractData.Address, m_kCLNet);
                string strContractAddress = bitContractPK.ToString();

                Action<bool> fnSigned = delegate (bool bRet)
                {
                    if (fnDone != null)
                    {
                        fnDone(bRet, strContractAddress, kArg);
                    }
                }; 

                SignAndSendTransactionAsync(kTX, kCoin, keys, fnSigned);
            };

            SendCommandAsync("prepublishcode", arrArgs, fnPublishDone);            
        }

        public void PublishContractAsync(ON_PUBLISH_CONTRACT_DONE fnDone, string strContractContent, string strCostFromMCLAddress, float fCostAmount, string strSendMCLAddress, string strChargeMCLAdrres = null, object kArg = null)
        {            
            if (string.IsNullOrEmpty(strContractContent) || string.IsNullOrEmpty(strCostFromMCLAddress) || string.IsNullOrEmpty(strSendMCLAddress))
            {
                if ( fnDone != null )
                {
                    fnDone(false, null, kArg);
                }
                return;
            }

            if (fCostAmount <= 0.0f)
            {
                if (fnDone != null)
                {
                    fnDone(false, null, kArg);
                }
                return;
            }

            string strCostName;
            string strCostBTAddress;
            string strCostPath;

            if (DecodeMCLAddress(strCostFromMCLAddress, out strCostName, out strCostPath, out strCostBTAddress) == false)
            {
                if (fnDone != null)
                {
                    fnDone(false, null, kArg);
                }
                return;
            }

            string strSenderName;
            string strSenderBTAddress;
            string strSenderPath;

            if (DecodeMCLAddress(strSendMCLAddress, out strSenderName, out strSenderPath, out strSenderBTAddress) == false)
            {
                if (fnDone != null)
                {
                    fnDone(false, null, kArg);
                }
                return;
            }

            ExtKey kCostKey = GetExtKeyByPath(strCostName, strCostPath);
            if (kCostKey == null)
            {
                if (fnDone != null)
                {
                    fnDone(false, null, kArg);
                }
                return;
            }

            ExtKey kSenderKey = GetExtKeyByPath(strSenderName, strSenderPath);
            if (kSenderKey == null)
            {
                if (fnDone != null)
                {
                    fnDone(false, null, kArg);
                }
                return;
            }

            string strChargeCelAddress;

            if (string.IsNullOrEmpty(strChargeMCLAdrres))
            {
                strChargeCelAddress = strCostBTAddress;
            }
            else
            {
                string strChargeName;
                string strChargePath;

                if (DecodeMCLAddress(strChargeMCLAdrres, out strChargeName, out strChargePath, out strChargeCelAddress) == false)
                {
                    if (fnDone != null)
                    {
                        fnDone(false, null, kArg);
                    }
                    return;
                }
            }

            PublishContractAsync(fnDone, strContractContent, kCostKey.PrivateKey, fCostAmount, kSenderKey.PrivateKey, strChargeCelAddress, kArg);
        }

        public bool PublishContract(out string strContractAddress, string strContractContent, Key kCostPriKey, float fCostAmount, Key kSenderPriKey, string strChargeCelAddress = null)
        {
            strContractAddress = null;
            if (string.IsNullOrEmpty(strContractContent) || kCostPriKey == null || kSenderPriKey == null )
            {
                return false;
            }

            if (fCostAmount < 0.0f)
            {
                return false;
            }

            string strCostFromAddress = kCostPriKey.PubKey.GetAddress(m_kCLNet).ToWif();
            string strSenderAddressHex = kSenderPriKey.PubKey.ToHex();

            if ( string.IsNullOrEmpty( strChargeCelAddress))
            {
                strChargeCelAddress = strCostFromAddress;
            }

            string strContractHex = HexEncoder.HexStr(strContractContent, false);

            //string fromaddress = "XD7SstxYNeBCUzLkVre3gP1ipUjJBGTxRB";//费用支出地址
            //string senderpubkeyhex = "02eaf227d7c4b38fd8798d82d00081d5f7833f5e3a690a5ec72103ac37ae0db877"; //公约地址hex format,
            //string amount = "0.1313";//发送给智能合约的金额
            //string changeaddress = "XD7SstxYNeBCUzLkVre3gP1ipUjJBGTxRB";//找零地址

            object[] arrArgs = new object[5];
            arrArgs[0] = strContractHex;
            arrArgs[1] = strCostFromAddress;
            arrArgs[2] = strSenderAddressHex;
            arrArgs[3] = ((decimal)fCostAmount).ToString();
            arrArgs[4] = strChargeCelAddress;

            RPCResponse kRsp = SendCommand("prepublishcode", arrArgs);
            if ( kRsp == null )
            {
                return false;
            }
            if ( kRsp.Error != null )
            {
                Logger.LogError("prepublishcode error: " + kRsp.Error.Code + " msg: " + kRsp.Error.Message);
                return false;
            }

            //private keys, two need
            List<Key> keys = new List<Key>();
            //BitcoinSecret bitsecret = new BitcoinSecret("L1amKPbuEpYwqU2uVRb34i2oUAa328RvNaEoKrCqgdEPLgV6MDcz");           

            keys.Add(kCostPriKey);
            keys.Add(kSenderPriKey);

            JToken kTX = kRsp.Result["txhex"];
            JToken kCoin = kRsp.Result["coins"];

            Transaction tx = Transaction.Parse((string)kTX);
            //strContractAddress = tx.TContractData.Address.GetAddress(m_kCLNet).ToWif();
            BitcoinContractPubKeyAddress bitContractPK = new BitcoinContractPubKeyAddress(tx.TContractData.Address, m_kCLNet);
            strContractAddress = bitContractPK.ToString();
            //Console.WriteLine("txid", tx.GetHash());
            //Console.WriteLine("contractaddress", tx.ContractAddress);

            return SignAndSendTransaction(kTX, kCoin, keys);
        }

        public bool PublishContract(out string strContractAddress, string strContractContent, string strCostFromMCLAddress, float fCostAmount, string strSendMCLAddress, string strChargeMCLAdrres = null)
        {
            strContractAddress = null;
            if ( string.IsNullOrEmpty(strContractContent) || string.IsNullOrEmpty(strCostFromMCLAddress) || string.IsNullOrEmpty(strSendMCLAddress))
            {
                return false;
            }

            if ( fCostAmount < 0.0f )
            {
                return false;
            }           

            string strCostName;
            string strCostBTAddress;
            string strCostPath;
            
            if ( DecodeMCLAddress(strCostFromMCLAddress, out strCostName, out strCostPath, out strCostBTAddress) == false)
            {
                return false;                
            }

            string strSenderName;
            string strSenderBTAddress;
            string strSenderPath;

            if ( DecodeMCLAddress(strSendMCLAddress, out strSenderName, out strSenderPath, out strSenderBTAddress) == false )
            {
                return false;
            }

            ExtKey kCostKey = GetExtKeyByPath(strCostName, strCostPath);
            if ( kCostKey == null )
            {
                return false;
            }

            ExtKey kSenderKey = GetExtKeyByPath(strSenderName, strSenderPath);
            if (kSenderKey == null )
            {
                return false;
            }

            string strChargeBTAddress;

            if ( string.IsNullOrEmpty(strChargeMCLAdrres))
            {
                strChargeBTAddress = strCostBTAddress;
            }
            else
            {
                string strChargeName;                
                string strChargePath;

                if ( DecodeMCLAddress(strChargeMCLAdrres, out strChargeName, out strChargePath, out strChargeBTAddress) == false )
                {
                    return false;
                }
            }

            return PublishContract(out strContractAddress, strContractContent, kCostKey.PrivateKey, fCostAmount, kSenderKey.PrivateKey, strChargeBTAddress);
        }  
        
        // bSendCall: 是否构造交易
        public string CallContractFunction(bool bSendCall, string strContractAddress, Key kCostPriKey, float fCostAmount, Key kSenderPriKey, string strFuncName, object[] arrArgs, string strChargeCelAddress = null)
        {
            if ( string.IsNullOrEmpty(strContractAddress) || kCostPriKey == null || kSenderPriKey == null || string.IsNullOrEmpty(strFuncName))
            {
                return null;
            }

            //string contractaddress = "XWPdrSPJvvBBKGqYM6ZEwSvbEBaRi24M7C";// 智能合约地址
            //string fundaddress = "XD7SstxYNeBCUzLkVre3gP1ipUjJBGTxRB";// 我的币地址
            //string amount = "0.1313"; // 转给智能合约的币
            //string senderpubkeyhex = "02eaf227d7c4b38fd8798d82d00081d5f7833f5e3a690a5ec72103ac37ae0db877";//作为sender的公钥的pubkey的hex
            //string changeaddress = "XD7SstxYNeBCUzLkVre3gP1ipUjJBGTxRB";//找零地址
            //string issendcall = "1"; //是否要构造交易，1是，0否

            //string funcname = "getBalanceOf"; //要调用的智能合约的函数名称
            //string param1 = "XYGPn2328RTvCZTstNQwRNUUGqq42sqjzx"; //参数  

            List<object> listArg = new List<object>();
            listArg.Add(strContractAddress);
            string strCostAddress = kCostPriKey.PubKey.GetAddress(m_kCLNet).ToWif();
            listArg.Add(strCostAddress);
            listArg.Add(((decimal)fCostAmount).ToString());
            listArg.Add(kSenderPriKey.PubKey.ToHex());
            if ( string.IsNullOrEmpty(strChargeCelAddress))
            {
                listArg.Add(strCostAddress);
            }
            else
            {
                listArg.Add(strChargeCelAddress);
            }
            if ( bSendCall )
            {
                listArg.Add("1");
            }
            else
            {
                listArg.Add("0");
            }
            listArg.Add(strFuncName);
            if ( arrArgs != null && arrArgs.Length > 0 )
            {
                listArg.AddRange(arrArgs);
            }            
                   
            RPCResponse kRsp = SendCommand("precallcontract", listArg.ToArray());
            if ( kRsp == null)
            {
                return null;
            }

            if ( kRsp.Error != null )
            {
                Logger.LogError("precallcontract error: " + kRsp.Error.Code + " msg: " + kRsp.Error.Message);
                return null;
            }

            string strRet = null;
            if ( kRsp.Result != null)
            {
                //Console.WriteLine("contract return value:" + kRsp.Result["call_return"] == null ? "null" : kRsp.Result["call_return"].ToString());
                JToken kReturn = null;
                JToken kTX = null;
                JToken kCoin = null;

                try
                {
                    kReturn = kRsp.Result["return"];
                    kTX = kRsp.Result["txhex"];
                    kCoin = kRsp.Result["coins"];
                }
                catch ( Exception e)
                {
                    
                }

                if (kReturn != null)
                {
                    strRet = kReturn.ToString();
                }

                if (kTX != null && kCoin != null && bSendCall)
                {
                    //private keys
                    List<Key> keys = new List<Key>();
                    //BitcoinSecret bitsecret = new BitcoinSecret("L1amKPbuEpYwqU2uVRb34i2oUAa328RvNaEoKrCqgdEPLgV6MDcz");

                    keys.Add(kCostPriKey);
                    keys.Add(kSenderPriKey);                    

                    SignAndSendTransaction(kTX, kCoin, keys);
                }
                //else
                //{
                //    //Console.WriteLine("智能合约数据没修过，没交易提交");
                //    return null;
                //}
            }

            return strRet;                      
        }

        // bSendCall: 是否构造交易
        public string CallContractFunction(bool bSendCall, string strContractAddress, string strCostMCLAddress, float fCostAmount, string strSenderMCLAddress, string strFuncName, object[] arrArgs, string strChargeMCLAddress = null)
        {
            if (string.IsNullOrEmpty(strContractAddress) || string.IsNullOrEmpty(strCostMCLAddress) || string.IsNullOrEmpty(strSenderMCLAddress) || string.IsNullOrEmpty(strFuncName))
            {
                return null;
            }

            string strName;
            string strPath;
            string strBTAddress;

            if ( DecodeMCLAddress(strCostMCLAddress, out strName, out strPath, out strBTAddress) == false )
            {
                return null;
            }
            ExtKey kCostKey = GetExtKeyByPath(strName, strPath);

            if (DecodeMCLAddress(strSenderMCLAddress, out strName, out strPath, out strBTAddress) == false)
            {
                return null;
            }
            ExtKey kSenderKey = GetExtKeyByPath(strName, strPath);

            string strChargeBTAddress;

            if ( !string.IsNullOrEmpty(strChargeMCLAddress))
            {
                if (DecodeMCLAddress(strChargeMCLAddress, out strName, out strPath, out strChargeBTAddress) == false)
                {
                    return null;
                }
            }
            else
            {
                strChargeBTAddress = kCostKey.PrivateKey.PubKey.GetAddress(m_kCLNet).ToWif();
            }
            return CallContractFunction(bSendCall, strContractAddress, kCostKey.PrivateKey, fCostAmount, kSenderKey.PrivateKey, strFuncName, arrArgs, strChargeBTAddress);
        }

        public void CallContractFunctionAsync(ON_CALL_CONTRACT_FUNCTION fnCallDone, bool bSendCall, string strContractAddress, Key kCostPriKey, float fCostAmount, Key kSenderPriKey, string strFuncName, object[] arrArgs, string strChargeCelAddress = null)
        {
            if (string.IsNullOrEmpty(strContractAddress) || kCostPriKey == null || kSenderPriKey == null || string.IsNullOrEmpty(strFuncName))
            {
                if (fnCallDone != null )
                {
                    fnCallDone(false, null);
                }
                return;
            }            

            List<object> listArg = new List<object>();
            listArg.Add(strContractAddress);
            string strCostAddress = kCostPriKey.PubKey.GetAddress(m_kCLNet).ToWif();
            listArg.Add(strCostAddress);
            listArg.Add(((decimal)fCostAmount).ToString());
            listArg.Add(kSenderPriKey.PubKey.ToHex());
            if (string.IsNullOrEmpty(strChargeCelAddress))
            {
                listArg.Add(strCostAddress);
            }
            else
            {
                listArg.Add(strChargeCelAddress);
            }
            if ( bSendCall )
            {
                listArg.Add("1");
            }
            else
            {
                listArg.Add("0");
            }
            listArg.Add(strFuncName);
            if ( arrArgs != null && arrArgs.Length > 0 )
            {
                listArg.AddRange(arrArgs);
            }            

            ON_COMMAND_DONE fnCallContractDone = delegate (RPCResponse kRsp)
            {
                if (kRsp == null)
                {
                    if (fnCallDone != null )
                    {
                        fnCallDone(false, null);
                    }
                    return;
                }

                if (kRsp.Error != null)
                {
                    Logger.LogError("precallcontract error: " + kRsp.Error.Code + " msg: " + kRsp.Error.Message);
                    if (fnCallDone != null)
                    {
                        fnCallDone(false, null);
                    }
                    return;
                }

                string strRet = null;
                if (kRsp.Result != null)
                {
                    //Console.WriteLine("contract return value:" + kRsp.Result["call_return"] == null ? "null" : kRsp.Result["call_return"].ToString());
                    JToken kReturn = null;
                    JToken kTX = null;
                    JToken kCoin = null;

                    try
                    {
                        kReturn = kRsp.Result["call_return"];
                        kTX = kRsp.Result["txhex"];
                        kCoin = kRsp.Result["coins"];
                    }
                    catch (Exception e)
                    {

                    }

                    if (kReturn != null)
                    {
                        strRet = kReturn.ToString();
                    }

                    if ( bSendCall )
                    {
                        if (kTX != null && kCoin != null)
                        {
                            //private keys
                            List<Key> keys = new List<Key>();
                            //BitcoinSecret bitsecret = new BitcoinSecret("L1amKPbuEpYwqU2uVRb34i2oUAa328RvNaEoKrCqgdEPLgV6MDcz");

                            keys.Add(kCostPriKey);
                            keys.Add(kSenderPriKey);

                            Action<bool> fnSigned = delegate (bool bRet)
                            {
                                if (fnCallDone != null)
                                {
                                    fnCallDone(bRet, strRet);
                                }
                            };
                            SignAndSendTransactionAsync(kTX, kCoin, keys, fnSigned);
                        }
                        else
                        {
                            if (fnCallDone != null)
                            {
                                fnCallDone(false, strRet);
                            }
                        }
                    }
                    else
                    {
                        if (fnCallDone != null)
                        {
                            fnCallDone(true, strRet);
                        }
                    }
                }
            };
            SendCommandAsync("precallcontract", listArg.ToArray(), fnCallContractDone);
        }

        public void CallContractFunctionAsync(ON_CALL_CONTRACT_FUNCTION fnCallDone, bool bSendCall, string strContractAddress, string strCostMCLAddress, float fCostAmount, string strSenderMCLAddress, string strFuncName, object[] arrArgs, string strChargeMCLAddress = null)
        {
            if (string.IsNullOrEmpty(strContractAddress) || string.IsNullOrEmpty(strCostMCLAddress) || string.IsNullOrEmpty(strSenderMCLAddress) || string.IsNullOrEmpty(strFuncName))
            {
                if ( fnCallDone != null )
                {
                    fnCallDone(false, null);
                }
                return;
            }

            string strName;
            string strPath;
            string strBTAddress;

            if (DecodeMCLAddress(strCostMCLAddress, out strName, out strPath, out strBTAddress) == false)
            {
                if (fnCallDone != null)
                {
                    fnCallDone(false, null);
                }
                return;
            }
            ExtKey kCostKey = GetExtKeyByPath(strName, strPath);

            if (DecodeMCLAddress(strSenderMCLAddress, out strName, out strPath, out strBTAddress) == false)
            {
                if (fnCallDone != null)
                {
                    fnCallDone(false, null);
                }
                return;
            }
            ExtKey kSenderKey = GetExtKeyByPath(strName, strPath);

            string strChargeCelAddress;

            if (!string.IsNullOrEmpty(strChargeMCLAddress))
            {
                if (DecodeMCLAddress(strChargeMCLAddress, out strName, out strPath, out strChargeCelAddress) == false)
                {
                    if (fnCallDone != null)
                    {
                        fnCallDone(false, null);
                    }
                    return;
                }
            }
            else
            {
                strChargeCelAddress = kCostKey.PrivateKey.PubKey.GetAddress(m_kCLNet).ToWif();
            }

            CallContractFunctionAsync(fnCallDone, bSendCall, strContractAddress, kCostKey.PrivateKey, fCostAmount, kSenderKey.PrivateKey, strFuncName, arrArgs, strChargeCelAddress);
        }

        //public string CallContract(string strContractAddress, string strFunction, string strSender = null, string strArgs = null)
        //{
        //    if ( string.IsNullOrEmpty(strContractAddress) || string.IsNullOrEmpty(strFunction))
        //    {
        //        return null;
        //    }

        //    m_sbK.Length = 0;
        //    m_sbK.Append("call ");
        //    m_sbK.Append(strContractAddress);
        //    m_sbK.Append(" ");
        //    if ( string.IsNullOrEmpty(strSender))
        //    {
        //        m_sbK.Append("\"\"");
        //    }
        //    else
        //    {
        //        m_sbK.Append(strSender);
        //    }
        //    m_sbK.Append(" ");
        //    m_sbK.Append(strFunction);            
        //    if ( !string.IsNullOrEmpty(strArgs))
        //    {
        //        m_sbK.Append(" ");
        //        m_sbK.Append(strArgs);
        //    }

        //    RPCResponse kRsp = SendCommand(m_sbK.ToString());
        //    if ( kRsp == null )
        //    {
        //        Logger.LogError("CallContract error!");
        //        return null;
        //    }
        //    if (kRsp.Error != null)
        //    {
        //        Logger.LogError("CallContract error: " + kRsp.Error.Code + " msg: " + kRsp.Error.Message);
        //        return null;
        //    }
        //    return kRsp.ResultString;
        //}

        //public void CallContractAsync(ON_CALL_CONTRACT fnCallDone, string strContractAddress, string strFunction, string strSender = null, string strArgs = null)
        //{
        //    if (string.IsNullOrEmpty(strContractAddress) || string.IsNullOrEmpty(strFunction))
        //    {
        //        if ( fnCallDone != null )
        //        {
        //            fnCallDone(null);
        //        }
        //        return;
        //    }

        //    m_sbK.Length = 0;
        //    m_sbK.Append("call ");
        //    m_sbK.Append(strContractAddress);
        //    m_sbK.Append(" ");
        //    if (string.IsNullOrEmpty(strSender))
        //    {
        //        m_sbK.Append("\"\"");
        //    }
        //    else
        //    {
        //        m_sbK.Append(strSender);
        //    }
        //    m_sbK.Append(" ");
        //    m_sbK.Append(strFunction);
        //    if (!string.IsNullOrEmpty(strArgs))
        //    {
        //        m_sbK.Append(" ");
        //        m_sbK.Append(strArgs);
        //    }

        //    ON_COMMAND_DONE fnCmdDone = delegate (RPCResponse kRsp)
        //    {
        //        if ( kRsp == null )
        //        {
        //            if ( fnCallDone != null )
        //            {
        //                Logger.LogError("CallContract error!");
        //            }
        //            return;
        //        }
        //        if (kRsp.Error != null)
        //        {
        //            Logger.LogError("CallContract error: " + kRsp.Error.Code + " msg: " + kRsp.Error.Message);
        //            return;
        //        }

        //        if ( fnCallDone != null )
        //        {
        //            fnCallDone(kRsp.ResultString);
        //        }                
        //    };
        //    SendCommandAsync(m_sbK.ToString(), fnCmdDone);            
        //}

        //public string SendCallContract(string strContractAddress, string strFunction, string strSender = null, string strArgs = null)
        //{
        //    if (string.IsNullOrEmpty(strContractAddress))
        //    {
        //        return null;
        //    }

        //    if (string.IsNullOrEmpty(strContractAddress) || string.IsNullOrEmpty(strFunction))
        //    {
        //        return null;
        //    }

        //    m_sbK.Length = 0;
        //    m_sbK.Append("sendcall ");
        //    m_sbK.Append(strContractAddress);
        //    m_sbK.Append(" ");
        //    if (string.IsNullOrEmpty(strSender))
        //    {
        //        m_sbK.Append("\"\"");
        //    }
        //    else
        //    {
        //        m_sbK.Append(strSender);
        //    }
        //    m_sbK.Append(" ");
        //    m_sbK.Append(strFunction);
        //    if (!string.IsNullOrEmpty(strArgs))
        //    {
        //        m_sbK.Append(" ");
        //        m_sbK.Append(strArgs);
        //    }

        //    RPCResponse kRsp = SendCommand(m_sbK.ToString());
        //    if ( kRsp == null )
        //    {
        //        Logger.LogError("SendCallContract error!");
        //        return null;
        //    }
        //    if (kRsp.Error != null)
        //    {
        //        Logger.LogError("SendCallContract error: " + kRsp.Error.Code + " msg: " + kRsp.Error.Message);
        //        return null;
        //    }
        //    return kRsp.ResultString;
        //}

        //public void SendCallContractAsync(ON_CALL_CONTRACT fnCallDone, string strContractAddress, string strFunction, string strSender = null, string strArgs = null)
        //{
        //    if (string.IsNullOrEmpty(strContractAddress) || string.IsNullOrEmpty(strFunction))
        //    {
        //        if (fnCallDone != null)
        //        {
        //            fnCallDone(null);
        //        }
        //        return;
        //    }

        //    m_sbK.Length = 0;
        //    m_sbK.Append("sendcall ");
        //    m_sbK.Append(strContractAddress);
        //    m_sbK.Append(" ");
        //    if (string.IsNullOrEmpty(strSender))
        //    {
        //        m_sbK.Append("\"\"");
        //    }
        //    else
        //    {
        //        m_sbK.Append(strSender);
        //    }
        //    m_sbK.Append(" ");
        //    m_sbK.Append(strFunction);
        //    if (!string.IsNullOrEmpty(strArgs))
        //    {
        //        m_sbK.Append(" ");
        //        m_sbK.Append(strArgs);
        //    }

        //    ON_COMMAND_DONE fnCmdDone = delegate (RPCResponse kRsp)
        //    {
        //        if (kRsp == null)
        //        {
        //            if (fnCallDone != null)
        //            {
        //                Logger.LogError("SendCallContractAsync error!");
        //            }
        //            return;
        //        }
        //        if (kRsp.Error != null)
        //        {
        //            Logger.LogError("SendCallContractAsync error: " + kRsp.Error.Code + " msg: " + kRsp.Error.Message);
        //            return;
        //        }

        //        if (fnCallDone != null)
        //        {
        //            fnCallDone(kRsp.ResultString);
        //        }
        //    };
        //    SendCommandAsync(m_sbK.ToString(), fnCmdDone);
        //}

        private void AsyncCommandThreadProc()
        {
            List<ASYNC_CMD_RESULT_INFO> listTmpACRI = new List<ASYNC_CMD_RESULT_INFO>();

            while ( true )
            {
                if ( m_iAsyncRPCThreadQuit == 1 )
                {
                    break;
                }

                listTmpACRI.Clear();
                lock (m_listACI)
                {
                    if ( m_listACI.Count > 0 )
                    {
                        int i;
                        ASYNC_CMD_INFO kACI;
                        ASYNC_CMD_RESULT_INFO kACRI;
                        RPCResponse kRsp;

                        for (i = 0; i < m_listACI.Count; i++)
                        {
                            kACI = m_listACI[i];

                            kRsp = SendCommand(kACI.strCmd, kACI.arrParams);
                            kACRI = new ASYNC_CMD_RESULT_INFO();
                            kACRI.fnCmdDone = kACI.fnCmdDone;
                            kACRI.kRsp = kRsp;
                            listTmpACRI.Add(kACRI);
                        }
                        m_listACI.Clear();
                    }                    
                }

                if ( listTmpACRI.Count > 0 )
                {
                    lock(m_listACRI)
                    {
                        m_listACRI.AddRange(listTmpACRI);
                    }
                }

                Thread.Sleep(10);
            }            
        }

        // must call per frame
        public void OnUpdate(float fIntervalTime)
        {
            m_fCheckAyncResultTick += fIntervalTime;
            if ( m_fCheckAyncResultTick >= m_fCheckAsyncResultIntervalTime )
            {
                m_fCheckAyncResultTick = 0.0f;
                lock(m_listACRI)
                {
                    if ( m_listACRI.Count > 0)
                    {
                        int i;
                        ASYNC_CMD_RESULT_INFO kACRI;

                        for ( i = 0; i < m_listACRI.Count; i++ )
                        {
                            kACRI = m_listACRI[i];
                            if ( kACRI.fnCmdDone != null )
                            {
                                kACRI.fnCmdDone(kACRI.kRsp);
                            }
                        }
                        m_listACRI.Clear();
                    }
                }
            }            
        }
    }
}
