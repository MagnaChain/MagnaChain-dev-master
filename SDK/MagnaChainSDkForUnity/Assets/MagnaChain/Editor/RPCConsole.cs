using UnityEngine;
using System.Collections.Generic;
using UnityEditor;
using System;
using System.IO;
using System.Threading;
using MagnaChainEx;
using CellLink.RPC;
using CellLink;
using CellLink.DataEncoders;
using CellLink.Policy;
using System.Diagnostics;

public class RPCConsole : EditorWindow
{
    public static RPCConsole ms_kSig;

    private const string COMMAND_SPLITE = "O-------------------------------------O";

    // Console UI params
    private static int VIEW_SETTING = 1;
    private static int VIEW_CONSOLE = 0;
    private static int VIEW_TYPE = VIEW_CONSOLE;

    // Settings
    private static string RPC_HOST = "";
    private static int RPC_PORT = 0;
    private static string RPC_USERNAME = "";       
    private static string RPC_PASSWORD = "";
    private static NETWORK_TYPE RPC_NET_TYPE = NETWORK_TYPE.TEST_NET;    

    private IxMagnaChainBridge m_kCB;
    private bool m_bCBInited = false;

    private string m_strRawExeRpcCommand = "getinfo";
    private string m_strExeRpcCommand;
    private object[] m_arrRpcArgs;

    private string m_strLuaFile;

    private Texture m_texIcon;

    private bool m_bNeedResetFocus = false;
    private int m_iPreViewType = VIEW_CONSOLE;

    private string[] m_arrCmdType;

    private List<string[]> m_listCommand;
    private List<string[]> m_listArgsInfo;

    private int m_iLastSelCmdTypeIndex = 0;
    private int m_iLastSelCmdIndex = 0;

    private string m_strRawCurArgInfo;
    private string m_strCurArgsInfo;
    private char[] m_arrArgType;

    private static string ms_strContractAddress = null;
    private string m_strCostCLAddress = null;
    private string m_strCostAmount = null;
    private string m_strSenderCLAddress = null;
    private string m_strChargeCLAddress = null;
    //private string m_strContractCode = null;

    //private string m_strSenderAddress;
    private string m_strCallFunction;
    private string m_strCallArgs;

    private bool m_bContractSendCall = false;

    private static string ms_strImportMasterKeyName;
    private static string ms_strMasterKey;

    private bool m_bEditLuaFile = false;

    private static string ms_strTemplateLuaPrefix;
    private static List<string> ms_listTemplateLuaFile;
    private static string[] ms_arrTemplateLuaFile;
    private static int ms_iSelTemplateLuaFineIndex = 0;
    private static string ms_strSelTemplateLuaFile;    

    public RPCConsole()
    {
        if (m_kCB != null)
        {
            m_kCB.Release();
            m_kCB = null;
        }

        if (m_kCB == null)
        {
            m_kCB = new IxMagnaChainBridge();
        }
        m_bCBInited = m_kCB.Initialize(RPC_HOST, RPC_PORT, RPC_USERNAME, RPC_PASSWORD, RPC_NET_TYPE);
        if (m_bCBInited == false)
        {
            if (EditorUtility.DisplayDialog("Erorr", "Initialize RCP client failed.", "OK"))
            {
                return;
            }
        }

        if (!string.IsNullOrEmpty(ms_strImportMasterKeyName) && !string.IsNullOrEmpty(ms_strMasterKey))
        {
            m_kCB.ImportMasterExtKey(ms_strImportMasterKeyName, ms_strMasterKey);
        }

        InitCommand();
    }

    void Awake()
    {
        
    }

    void OnDestroy()
    {
        if ( m_kCB != null)
        {
            m_kCB.Release();
            m_kCB = null;
        }
    }

    private void InitCommand()
    {
        m_listCommand = new List<string[]>();
        m_listArgsInfo = new List<string[]>();

        List<string> listCmdType = new List<string>();
        List<string> listCmd = new List<string>();
        List<string> listArgsInfo = new List<string>();

        listCmdType.Add("misc");
        listCmdType.Add("rawtransaction");
        listCmdType.Add("wallet");        
        listCmdType.Add("blockchain");
        listCmdType.Add("mining");
        listCmdType.Add("network");
        listCmdType.Add("server");
        m_arrCmdType = listCmdType.ToArray();

        // misc
        listCmd.Add("getinfo");
        listArgsInfo.Add("");
        listCmd.Add("testrpc");
        listArgsInfo.Add("");
        listCmd.Add("getmemoryinfo");
        listArgsInfo.Add("\"mode\"");
        listCmd.Add("validateaddress");
        listArgsInfo.Add("\"address\"");
        listCmd.Add("createmultisig");
        listArgsInfo.Add("%d\"nrequired\", \"keys\"");
        listCmd.Add("verifymessage");
        listArgsInfo.Add("\"address\", \"signature\", \"message\"");
        listCmd.Add("signmessagewithprivkey");
        listArgsInfo.Add("\"privkey\", \"message\"");
        m_listCommand.Add(listCmd.ToArray());
        m_listArgsInfo.Add(listArgsInfo.ToArray());

        // rawtransaction
        listCmd.Clear();
        listArgsInfo.Clear();
        listCmd.Add("getrawtransaction");
        listArgsInfo.Add("\"txid\", \"verbose\"");
        listCmd.Add("createrawtransaction");
        listArgsInfo.Add("\"inputs\", \"outputs\", %d\"locktime\", \"replaceable\"");
        listCmd.Add("decoderawtransaction");
        listArgsInfo.Add("\"hexstring\"");
        listCmd.Add("decodescript");
        listArgsInfo.Add("\"hexstring\"");
        listCmd.Add("sendrawtransaction");
        listArgsInfo.Add("\"hexstring\", \"allowhighfees\"");
        listCmd.Add("combinerawtransaction");
        listArgsInfo.Add("\"txs\"");
        listCmd.Add("signrawtransaction");
        listArgsInfo.Add("\"hexstring\", \"prevtxs\", \"privkeys\", \"sighashtype\"");
        listCmd.Add("fundrawtransaction");
        listArgsInfo.Add("\"hexstring\", \"options\"");
        listCmd.Add("resendwallettransactions");
        listArgsInfo.Add("");
        listCmd.Add("getaddresscoins");
        listArgsInfo.Add("\"address\", \"withscript\"");

        listCmd.Add("gettxoutproof");
        listArgsInfo.Add("\"txids\", \"blockhash\"");
        listCmd.Add("verifytxoutproof");
        listArgsInfo.Add("\"proof\"");
        m_listCommand.Add(listCmd.ToArray());
        m_listArgsInfo.Add(listArgsInfo.ToArray());

        // wallet
        listCmd.Clear();
        listArgsInfo.Clear();
        listCmd.Add("abandontransaction");
        listArgsInfo.Add("\"txid\"");
        listCmd.Add("abortrescan");
        listArgsInfo.Add("");
        listCmd.Add("addmultisigaddress");
        listArgsInfo.Add("%d\"nrequired\", \"keys\", \"account\"");
        listCmd.Add("addwitnessaddress");
        listArgsInfo.Add("\"address\"");
        listCmd.Add("backupwallet");
        listArgsInfo.Add("\"destination\"");
        listCmd.Add("bumpfee");
        listArgsInfo.Add("\"txid\", \"options\"");
        listCmd.Add("dumpprivkey");
        listArgsInfo.Add("\"address\"");
        listCmd.Add("dumpwallet");
        listArgsInfo.Add("\"filename\"");
        listCmd.Add("encryptwallet");
        listArgsInfo.Add("\"passphrase\"");
        listCmd.Add("getaccountaddress");
        listArgsInfo.Add("\"account\"");
        listCmd.Add("getaccount");
        listArgsInfo.Add("\"address\"");
        listCmd.Add("getaddressesbyaccount");
        listArgsInfo.Add("\"account\"");
        listCmd.Add("getbalance");
        listArgsInfo.Add("\"account\", \"minconf\", \"include_watchonly\"");
        listCmd.Add("getbalanceof");
        listArgsInfo.Add("\"address\"");
        listCmd.Add("getnewaddress");
        listArgsInfo.Add("\"account\"");
        listCmd.Add("getrawchangeaddress");
        listArgsInfo.Add("");
        listCmd.Add("getreceivedbyaccount");
        listArgsInfo.Add("\"account\", \"minconf\"");
        listCmd.Add("getreceivedbyaddress");
        listArgsInfo.Add("\"address\", \"minconf\"");
        listCmd.Add("gettransaction");
        listArgsInfo.Add("\"txid\", \"include_watchonly\"");
        listCmd.Add("getunconfirmedbalance");
        listArgsInfo.Add("");
        listCmd.Add("getwalletinfo");
        listArgsInfo.Add("");
        listCmd.Add("importmulti");
        listArgsInfo.Add("\"requests\", \"options\"");
        listCmd.Add("importprivkey");
        listArgsInfo.Add("\"privkey\", \"label\", \"rescan\"");
        listCmd.Add("importwallet");
        listArgsInfo.Add("\"filename\"");
        listCmd.Add("importaddress");
        listArgsInfo.Add("\"address\", \"label\", \"rescan\", \"p2sh\"");
        listCmd.Add("importprunedfunds");
        listArgsInfo.Add("\"rawtransaction\", \"txoutproof\"");
        listCmd.Add("importpubkey");
        listArgsInfo.Add("\"pubkey\", \"label\", \"rescan\"");
        listCmd.Add("keypoolrefill");
        listArgsInfo.Add("%d\"newsize\"");
        listCmd.Add("listaccounts");
        listArgsInfo.Add("%d\"minconf\", \"include_watchonly\"");
        listCmd.Add("listaddressgroupings");
        listArgsInfo.Add("");
        listCmd.Add("listlockunspent");
        listArgsInfo.Add("");
        listCmd.Add("listreceivedbyaccount");
        listArgsInfo.Add("%d\"minconf\", \"include_empty\", \"include_watchonly\"");
        listCmd.Add("listreceivedbyaddress");
        listArgsInfo.Add("%d\"minconf\", \"include_empty\", \"include_watchonly\"");
        listCmd.Add("listsinceblock");
        listArgsInfo.Add("\"blockhash\", %d\"target_confirmations\", \"include_watchonly\", \"include_removed\"");
        listCmd.Add("listtransactions");
        listArgsInfo.Add("\"account\", %d\"count\", %d\"skip\", \"include_watchonly\"");
        listCmd.Add("listunspent");
        listArgsInfo.Add("%d\"minconf\", %d\"maxconf\", \"addresses\", \"include_unsafe\", \"query_options\"");
        listCmd.Add("listwallets");
        listArgsInfo.Add("");
        listCmd.Add("lockunspent");
        listArgsInfo.Add("\"unlock\", \"transactions\"");
        listCmd.Add("move");
        listArgsInfo.Add("\"fromaccount\", \"toaccount\", %d\"amount\", %d\"minconf\", \"comment\"");
        listCmd.Add("sendfrom");
        listArgsInfo.Add("\"fromaccount\", \"toaddress\", %d\"amount\", %d\"minconf\", \"comment\", \"comment_to\"");
        listCmd.Add("sendmany");
        listArgsInfo.Add("\"fromaccount\", \"amounts\", %d\"minconf\", \"comment\", \"subtractfeefrom\", \"replaceable\", %d\"conf_target\", \"estimate_mode\"");
        listCmd.Add("sendtoaddress");
        listArgsInfo.Add("\"address\", \"amount\", \"comment\", \"comment_to\", \"subtractfeefromamount\", \"replaceable\", %d\"conf_target\", \"estimate_mode\"");
        
        listCmd.Add("publishcontract");
        listArgsInfo.Add("\"filename\"");
        listCmd.Add("publishcontractcode");
        listArgsInfo.Add("\"codehex\"");
        listCmd.Add("callcontract");
        listArgsInfo.Add("\"callcontract\", %d\"amount\", \"contractaddress\", \"senderaddress\", \"functionname\", \"params...with-space-split\"");
        listCmd.Add("setaccount");
        listArgsInfo.Add("\"address\", \"account\"");
        listCmd.Add("settxfee");
        listArgsInfo.Add("\"amount\"");
        listCmd.Add("signmessage");
        listArgsInfo.Add("\"address\", \"message\"");
        listCmd.Add("walletlock");
        listArgsInfo.Add("");
        listCmd.Add("walletpassphrasechange");
        listArgsInfo.Add("\"oldpassphrase\", \"newpassphrase\"");
        listCmd.Add("walletpassphrase");
        listArgsInfo.Add("\"passphrase\", \"timeout\"");
        listCmd.Add("removeprunedfunds");
        listArgsInfo.Add("\"txid\"");
        m_listCommand.Add(listCmd.ToArray());
        m_listArgsInfo.Add(listArgsInfo.ToArray());

        // blockchain
        listCmd.Clear();
        listArgsInfo.Clear();
        listCmd.Add("getblockchaininfo");
        listArgsInfo.Add("");
        listCmd.Add("getchaintxstats");
        listArgsInfo.Add("%d\"nblocks\", \"blockhash\"");
        listCmd.Add("getbestblockhash");
        listArgsInfo.Add("");
        listCmd.Add("getblockcount");
        listArgsInfo.Add("");
        listCmd.Add("getblock");
        listArgsInfo.Add("\"blockhash\", %d\"verbosity\", %d\"showtx\"");
        listCmd.Add("getlastblocktx");
        listArgsInfo.Add("");
        listCmd.Add("getblockhash");
        listArgsInfo.Add("%d\"height\"");
        listCmd.Add("getblockheader");
        listArgsInfo.Add("\"blockhash\", \"verbose\"");
        listCmd.Add("getchaintips");
        listArgsInfo.Add("");
        listCmd.Add("getdifficulty");
        listArgsInfo.Add("");
        listCmd.Add("getmempoolancestors");
        listArgsInfo.Add("\"txid\", \"verbose\"");
        listCmd.Add("getmempooldescendants");
        listArgsInfo.Add("\"txid\", \"verbose\"");
        listCmd.Add("getmempoolentry");
        listArgsInfo.Add("\"txid\"");
        listCmd.Add("getmempoolinfo");
        listArgsInfo.Add("");
        listCmd.Add("getrawmempool");
        listArgsInfo.Add("\"verbose\"");
        listCmd.Add("gettxout");
        listArgsInfo.Add("\"txid\", %d\"n\", \"include_mempool\"");
        listCmd.Add("gettxoutsetinfo");
        listArgsInfo.Add("");
        listCmd.Add("pruneblockchain");
        listArgsInfo.Add("%d\"height\"");
        listCmd.Add("verifychain");
        listArgsInfo.Add("%d\"checklevel\", %d\"nblocks\"");
        listCmd.Add("preciousblock");
        listArgsInfo.Add("\"blockhash\"");
        listCmd.Add("gettxoutproof");
        listArgsInfo.Add("\"txids\", \"blockhash\"");
        listCmd.Add("verifytxoutproof");
        listArgsInfo.Add("\"proof\"");
        listCmd.Add("generate");
        listArgsInfo.Add("%d\"nblocks\", %d\"maxtries\"");
        listCmd.Add("generateforbigboom");
        listArgsInfo.Add("%d\"nblocks\", %d\"maxtries\"");
        m_listCommand.Add(listCmd.ToArray());
        m_listArgsInfo.Add(listArgsInfo.ToArray());

        // mining
        listCmd.Clear();
        listArgsInfo.Clear();
        listCmd.Add("getnetworkhashps");
        listArgsInfo.Add("%d\"nblocks\", %d\"height\"");
        listCmd.Add("getmininginfo");
        listArgsInfo.Add("");
        listCmd.Add("prioritisetransaction");
        listArgsInfo.Add("\"txid\", %d\"dummy\", %d\"fee_delta\"");
        listCmd.Add("getblocktemplate");
        listArgsInfo.Add("\"address\", \"template_request\"");
        listCmd.Add("submitblock");
        listArgsInfo.Add("\"hexdata\", \"dummy\"");
        listCmd.Add("generatetoaddress");
        listArgsInfo.Add("%d\"nblocks\", \"address\", \"maxtries\"");
        listCmd.Add("setgenerate");
        listArgsInfo.Add("\"generate\"");
        listCmd.Add("estimatefee");
        listArgsInfo.Add("%d\"nblocks\"");
        listCmd.Add("estimatesmartfee");
        listArgsInfo.Add("%d\"conf_target\", \"estimate_mode\"");        
        m_listCommand.Add(listCmd.ToArray());
        m_listArgsInfo.Add(listArgsInfo.ToArray());

        // network
        listCmd.Clear();
        listArgsInfo.Clear();
        listCmd.Add("getconnectioncount");
        listArgsInfo.Add("");
        listCmd.Add("ping");
        listArgsInfo.Add("");
        listCmd.Add("getpeerinfo");
        listArgsInfo.Add("");
        listCmd.Add("addnode");
        listArgsInfo.Add("\"node\", \"command\"");
        listCmd.Add("disconnectnode");
        listArgsInfo.Add("\"address\", \"nodeid\"");
        listCmd.Add("getaddednodeinfo");
        listArgsInfo.Add("\"node\"");
        listCmd.Add("getnettotals");
        listArgsInfo.Add("");
        listCmd.Add("getnetworkinfo");
        listArgsInfo.Add("");
        listCmd.Add("setban");
        listArgsInfo.Add("\"subnet\", \"command\", %d\"bantime\", \"absolute\"");
        listCmd.Add("listbanned");
        listArgsInfo.Add("");
        listCmd.Add("clearbanned");
        listArgsInfo.Add("");
        listCmd.Add("setnetworkactive");
        listArgsInfo.Add("\"state\"");        
        m_listCommand.Add(listCmd.ToArray());
        m_listArgsInfo.Add(listArgsInfo.ToArray());

        // server
        listCmd.Clear();
        listArgsInfo.Clear();
        listCmd.Add("help");
        listArgsInfo.Add("\"command\"");
        listCmd.Add("stop");
        listArgsInfo.Add("");
        listCmd.Add("uptime");
        listArgsInfo.Add("");        
        m_listCommand.Add(listCmd.ToArray());
        m_listArgsInfo.Add(listArgsInfo.ToArray());
    }

    private void LogError(string strError)
    {
        if ( string.IsNullOrEmpty(strError))
        {
            return;
        }

        //Debug.LogError("<color=red>" + strError + "</color>");
        UnityEngine.Debug.LogError(strError);
    }

    private void LogNormal(string strInfo)
    {
        if ( string.IsNullOrEmpty(strInfo))
        {
            return;
        }

        UnityEngine.Debug.Log(strInfo);
    }

    private void LogRaw(string strInfo)
    {
        if (string.IsNullOrEmpty(strInfo))
        {
            return;
        }

        UnityEngine.Debug.Log(strInfo);
    }

    private void LogSys(string strInfo)
    {
        if (string.IsNullOrEmpty(strInfo))
        {
            return;
        }

        //Debug.Log("<color=#98FB98>" + strInfo + "</color>");
        UnityEngine.Debug.Log(strInfo);
    }

    [MenuItem("MagnaChain/RPCConsole")]
    private static void OpenRPCConsole()
    {
        string strKeyPrefix = Application.dataPath;
        RPC_HOST = PlayerPrefs.GetString(strKeyPrefix + "#RPC_HOST", @"http://127.0.0.1");
        RPC_PORT = PlayerPrefs.GetInt(strKeyPrefix + "#RPC_PORT", 8332);
        RPC_USERNAME = PlayerPrefs.GetString(strKeyPrefix + "#RPC_USERNAME", @"user");
        RPC_PASSWORD = PlayerPrefs.GetString(strKeyPrefix + "#RPC_PASSWORD", @"pwd");
        RPC_NET_TYPE = (NETWORK_TYPE)PlayerPrefs.GetInt(strKeyPrefix + "#RPC_NET_TYPE", (int)NETWORK_TYPE.TEST_NET);

        ms_strImportMasterKeyName = PlayerPrefs.GetString(strKeyPrefix + "#ms_strImportMasterKeyName", "");
        ms_strMasterKey = PlayerPrefs.GetString(strKeyPrefix + "#ms_strMasterKey", "");                
        
        ms_strContractAddress = PlayerPrefs.GetString(strKeyPrefix + "#ms_strContractAddress", "");

        ms_kSig = GetWindow<RPCConsole>(false, "MagnaChain", true);
        ms_kSig.position = new Rect(400, 150, 760, 610);

        Initialize();
    }  
    
    public static void Initialize()
    {
        ms_strTemplateLuaPrefix = Application.dataPath + "/MagnaChain/Contract/Src/";
        FileInfo[] arrFI = FileFunc.FindAllFiles(ms_strTemplateLuaPrefix, "*.lua", true);

        int i;
        int k;
        string strS;
        string strP;

        if ( ms_listTemplateLuaFile == null )
        {
            ms_listTemplateLuaFile = new List<string>();
        }
        else
        {
            ms_listTemplateLuaFile.Clear();
        }

        ms_listTemplateLuaFile.Add("None");
        for ( i = 0; i < arrFI.Length; i++ )
        {
            strS = arrFI[i].FullName.Replace("\\", "/");
            k = strS.IndexOf("/Src/");
            if ( k == -1 )
            {
                continue;
            }
            strP = strS.Substring(k + 5);
            ms_listTemplateLuaFile.Add(strP);
        }
               
        ms_arrTemplateLuaFile = ms_listTemplateLuaFile.ToArray();       

        ms_kSig.m_texIcon = AssetDatabase.LoadAssetAtPath("Assets/MagnaChain/Editor/res/celllink_icon.png", typeof(Texture)) as Texture;                
    }

    private void OnGUI()
    {
        if (ms_kSig == null)
        {
            return;
        }

        if ( m_listCommand == null || m_listArgsInfo == null )
        {
            InitCommand();
        }

        GUILayout.BeginHorizontal();
        GUILayout.Space(9);
        GUI.SetNextControlName("Tabbar");
        VIEW_TYPE = GUILayout.Toolbar(VIEW_TYPE, new GUIContent[] { new GUIContent("RPC Console"), new GUIContent("Settings") }, GUILayout.Width(200));
        GUILayout.EndHorizontal();
        //GUILayout.Space(20);

        float fX = 8.0f;
        float fY = 23.0f;
        float fWidth = ms_kSig.position.width - fX * 2.0f;
        float fHeight = ms_kSig.position.height - 33.0f;
        GUI.SetNextControlName("BackgroundBox");
        GUI.Box(new Rect(8, 23, ms_kSig.position.width - 16, ms_kSig.position.height - 33), "");

        GUILayout.BeginHorizontal();
        float fIX = ms_kSig.position.width - 70.0f;
        float fIY = ms_kSig.position.height - 70.0f;
        GUI.SetNextControlName("IconBox");
        GUI.Box(new Rect(fIX, fIY, 52.0f, 52.0f), m_texIcon);
        GUILayout.EndHorizontal();        

        float fYOffset = 0.0f;
        if ( VIEW_TYPE != m_iPreViewType )
        {
            m_bNeedResetFocus = true;
            m_iPreViewType = VIEW_TYPE;
        }

        if (VIEW_TYPE == VIEW_SETTING)
        {
            DrawSettingsUI(fYOffset);
        }
        else if (VIEW_TYPE == VIEW_CONSOLE)
        {
            DrawConsoleUI(fYOffset);
        }
    }  
    
    private void ResolveArgType()
    {
        m_arrArgType = null;

        if ( string.IsNullOrEmpty(m_strRawCurArgInfo))
        {
            return;
        }

        int i;
        int iK;
        char cT;

        char[] arrSpt = new char[1];
        arrSpt[0] = ',';
        string[] arrSeg = m_strRawCurArgInfo.Split(arrSpt);
        m_arrArgType = new char[arrSeg.Length];
        for ( i = 0; i < arrSeg.Length; i++ )
        {            
            iK = arrSeg[i].IndexOf('%');
            if ( iK == -1 )
            {
                m_arrArgType[i] = 's';
            }
            else
            {
                if ( iK >= arrSeg[i].Length - 1)
                {
                    m_arrArgType[i] = 's';
                    arrSeg[i] = arrSeg[i].Remove(iK, 1);
                }
                else
                {
                    cT = arrSeg[i][iK+1];
                    if ( cT == 'd' || cT == 'D')
                    {
                        m_arrArgType[i] = 'd';
                    }
                    else if ( cT == 'f' || cT == 'F')
                    {
                        m_arrArgType[i] = 'f';
                    }
                    else
                    {
                        m_arrArgType[i] = 's';
                    }

                    arrSeg[i] = arrSeg[i].Remove(iK, 2);
                }
            }
        }

        m_strCurArgsInfo = "";
        for ( i = 0; i < arrSeg.Length; i++ )
        {
            m_strCurArgsInfo += arrSeg[i];
        }
    }  

    private void ResolveExeCommand()
    {
        if ( string.IsNullOrEmpty(m_strRawExeRpcCommand))
        {
            return;
        }

        if ( m_strRawExeRpcCommand[m_strRawExeRpcCommand.Length -1] == ' ')
        {
            m_strRawExeRpcCommand = m_strRawExeRpcCommand.Remove(m_strRawExeRpcCommand.Length - 1);
        }

        char[] arrSpt = new char[1];
        arrSpt[0] = ' ';
        string[] arrSeg = System.Text.RegularExpressions.Regex.Split(m_strRawExeRpcCommand, @"\s+"); //m_strRawExeRpcCommand.Split(arrSpt);
        m_strExeRpcCommand = arrSeg[0];

        int iArgCount;

        if ( m_arrArgType == null )
        {
            iArgCount = 0;
        }
        else
        {
            iArgCount = Math.Min(arrSeg.Length - 1, m_arrArgType.Length);
        }
        
        if ( iArgCount <= 0 )
        {
            m_arrRpcArgs = null;
            return;
        }

        int i;
        char cType;
        string strArg;
        int iArg;
        float fArg;

        m_arrRpcArgs = new object[iArgCount];
        for ( i = 0; i < m_arrRpcArgs.Length; i++ )
        {
            cType = m_arrArgType[i];
            strArg = arrSeg[i + 1];
            try
            {
                if (cType == 'd')
                {
                    iArg = Convert.ToInt32(strArg);
                    m_arrRpcArgs[i] = iArg;
                }
                else if ( cType == 'f')
                {
                    fArg = Convert.ToSingle(strArg);
                    m_arrRpcArgs[i] = fArg;
                }
                else
                {
                    m_arrRpcArgs[i] = strArg;
                }
            }
            catch ( Exception e)
            {
                LogError(e.Message);
                m_arrRpcArgs[i] = strArg;
            }
        }
    }

    private void DrawConsoleUI(float fYOffset)
    {
        if (m_bNeedResetFocus)
        {            
            GUI.FocusControl(null);
            m_bNeedResetFocus = false;
        }

        GUILayout.Space(10);        

        GUILayout.BeginHorizontal();
        GUILayout.Space(20f);
        Color kOrgColor = GUI.color;
        GUI.color = Color.cyan;
        GUILayout.Label("Please see unity console for output detail.", GUILayout.Width(300));
        GUI.color = kOrgColor;
        GUILayout.EndHorizontal();

        GUILayout.Space(5);

        GUILayout.BeginHorizontal();
        GUILayout.Space(20f);
        GUILayout.Label("RPC Command:", GUILayout.Width(100));
        GUILayout.EndHorizontal();

        //GUILayout.Space(5);

        GUILayoutOption[] arrLO = new GUILayoutOption[2];
        arrLO[0] = GUILayout.Width(170);
        arrLO[1] = GUILayout.Height(20);

        GUILayout.BeginHorizontal();
        GUILayout.Space(20.0f);
        Color kOldC = GUI.color;
        GUI.color = new Color(255 / 255.0f, 160 / 255.0f, 50 / 255.0f);
        GUILayout.Label("Command Type:", GUILayout.Width(120.0f));
        int iSelCmdTypeIndex = EditorGUILayout.Popup(m_iLastSelCmdTypeIndex, m_arrCmdType, arrLO);
        if ( iSelCmdTypeIndex != m_iLastSelCmdTypeIndex )
        {
            m_iLastSelCmdIndex = 0;
            m_iLastSelCmdTypeIndex = iSelCmdTypeIndex;
        }
        GUI.color = kOldC;        
        GUILayout.EndHorizontal();

        string[] arrCmd = m_listCommand[m_iLastSelCmdTypeIndex];
        string[] arrArgInfo = m_listArgsInfo[m_iLastSelCmdTypeIndex];

        GUILayout.BeginHorizontal();
        GUILayout.Space(20.0f);
        kOldC = GUI.color;
        GUI.color = Color.yellow;
        GUILayout.Label("Command Select:", GUILayout.Width(120.0f));       
        int iSelCmdIndex = EditorGUILayout.Popup(m_iLastSelCmdIndex, arrCmd, arrLO);
        GUI.color = kOldC;
        if ( iSelCmdIndex != m_iLastSelCmdIndex )
        {
            if (arrCmd[iSelCmdIndex] != COMMAND_SPLITE)
            {
                m_strRawCurArgInfo = arrArgInfo[iSelCmdIndex];
                //m_strCurArgsInfo = arrArgInfo[iSelCmdIndex];
                ResolveArgType();
                if ( string.IsNullOrEmpty(m_strCurArgsInfo))
                {
                    m_strRawExeRpcCommand = arrCmd[iSelCmdIndex];
                }
                else
                {
                    m_strRawExeRpcCommand = arrCmd[iSelCmdIndex] + " ";
                }               

                GUI.FocusControl("RPCCmdInput");
            }
            else
            {
                m_strRawExeRpcCommand = "";
                m_strCurArgsInfo = "";
                GUI.FocusControl(null);
            }
            m_iLastSelCmdIndex = iSelCmdIndex;
        }
        GUILayout.Space(5);
        kOldC = GUI.color;
        GUI.color = new Color(0.69f, 0.77f, 0.95f);
        GUILayout.Label(m_strCurArgsInfo, GUILayout.Width(400.0f));
        GUI.color = kOldC;
        GUILayout.EndHorizontal();

        GUILayout.Space(5);

        GUILayout.BeginHorizontal();
        GUILayout.Space(20f);
        //arrLO = new GUILayoutOption[2];
        float fTAWidth = ms_kSig.position.width - 40.0f;
        arrLO[0] = GUILayout.Width(fTAWidth);
        //arrLO[0] = GUILayout.Width(560);
        arrLO[1] = GUILayout.Height(100);
        GUI.SetNextControlName("RPCCmdInput");
        m_strRawExeRpcCommand = EditorGUILayout.TextArea(m_strRawExeRpcCommand, arrLO);
        //m_strRpcCommand = GUILayout.TextArea(m_strRpcCommand, arrLO);
        GUILayout.EndHorizontal();

        GUILayout.Space(10);

        GUILayout.BeginHorizontal();
        float fBtnWidth = 100.0f;
        float fBtnHeight = 25.0f;
        float fBtnXOffset = 210.0f;
        float fXSpace = fBtnXOffset;
        GUILayout.Space(fXSpace);        
        arrLO[0] = GUILayout.Width(fBtnWidth);
        arrLO[1] = GUILayout.Height(fBtnHeight);
        if (GUILayout.Button("Execute", arrLO))
        {
            if (m_bCBInited == false)
            {
                if (EditorUtility.DisplayDialog("Erorr", "MagnaChain has not initialized", "OK"))
                {
                    return;
                }
            }

            if (string.IsNullOrEmpty(m_strRawExeRpcCommand))
            {
                if (EditorUtility.DisplayDialog("Prompt", "The rpc command is empty.", "OK"))
                {
                    return;
                }
            }    

            try
            {
                ResolveExeCommand();

                RPCResponse kRsp = m_kCB.SendCommand(m_strExeRpcCommand, m_arrRpcArgs);
                if (kRsp == null)
                {
                    LogError("Execute command error!");
                }
                else
                {
                    if (kRsp.Error == null)
                    {
                        LogSys(kRsp.ResultString);
                    }
                    else
                    {
                        LogError("Error: " + kRsp.Error.Code + " msg: " + kRsp.Error.Message);
                    }
                }
            }
            catch (Exception e)
            {
                LogError(e.Message);
            }
        }

        float fXSpace2 = (ms_kSig.position.width * 0.5f - fBtnWidth - fBtnXOffset) * 2.0f;
        //fXSpace = fXSpace2 - fXSpace;
        GUILayout.Space(fXSpace2);        
        if ( GUILayout.Button("Clear", arrLO))
        {
            m_strRawExeRpcCommand = "";

            GUI.FocusControl(null);
        }
        GUILayout.EndHorizontal();

        GUILayout.Space(20);

        GUILayout.BeginHorizontal();
        GUILayout.Space(20.0f);
        GUILayout.Label("Template Contract:", GUILayout.Width(120.0f));
        ms_iSelTemplateLuaFineIndex = EditorGUILayout.Popup(ms_iSelTemplateLuaFineIndex, ms_arrTemplateLuaFile, GUILayout.Width(200));
        if ( ms_arrTemplateLuaFile.Length > 0 && ms_iSelTemplateLuaFineIndex > 0)
        {
            ms_strSelTemplateLuaFile = ms_arrTemplateLuaFile[ms_iSelTemplateLuaFineIndex];

            string strFullPath = ms_strTemplateLuaPrefix + ms_strSelTemplateLuaFile;
            if ( strFullPath != m_strLuaFile)
            {
                m_strLuaFile = strFullPath;
            }
        }
        GUILayout.EndHorizontal();

        GUILayout.Space(6);

        //m_strLuaFile = DrawFileChooser("CFLabe", "CFInput", "CFBtn", 20.0f, true, "Contract File:", 90.0f, m_strLuaFile, 220.0f, "", false, false, "lua", 70, 100);
        GUILayout.BeginHorizontal();
        GUILayout.Space(20.0f);
        GUI.SetNextControlName("CFLabe");
        GUILayout.Label("Contract File:", GUILayout.Width(90.0f));    // 120
        //strFilePath = EditorGUILayout.TextField(strFilePath, GUILayout.Width(fTextFiledWidth));   // 330
        float fTFWidth;

        fTFWidth = ms_kSig.position.width - 20.0f - 90.0f - 10.0f - 70.0f - 20.0f - 10.0f - 90.0f;

        //arrLO = new GUILayoutOption[2];
        arrLO[0] = GUILayout.Height(18.0f);
        arrLO[1] = GUILayout.Width(fTFWidth);
        GUI.SetNextControlName("CFInput");

        //strFilePath = GUILayout.TextField(strFilePath, 512, GUILayout.Width(fTextFiledWidth));
        m_strLuaFile = EditorGUILayout.TextField(m_strLuaFile, arrLO);

        GUILayout.Space(10);
        arrLO[1] = GUILayout.Width(70.0f);
        GUI.SetNextControlName("CFBtn");
        if (GUILayout.Button("Browser", arrLO))
        {
            ms_iSelTemplateLuaFineIndex = 0;
            m_strLuaFile = EditorUtility.OpenFilePanel("Choose " + "Contract File:", "", "lua");
        }

        arrLO[0] = GUILayout.Width(70.0f);
        fXSpace = 5.0f;
        GUILayout.Space(fXSpace);
        if (GUILayout.Button("Edit", arrLO))
        {
            if ( !string.IsNullOrEmpty(m_strLuaFile))
            {
                Process process = new Process();
                process.StartInfo.FileName = "notepad.exe";
                process.StartInfo.Arguments = m_strLuaFile;
                process.StartInfo.UseShellExecute = false;
                process.StartInfo.RedirectStandardInput = false;
                process.StartInfo.RedirectStandardOutput = false;
                process.StartInfo.RedirectStandardError = false;
                process.StartInfo.CreateNoWindow = true;
                process.Start();

                m_bEditLuaFile = true;
            }
        }
        GUILayout.EndHorizontal();

        GUILayout.Space(10);        

        m_strCostCLAddress = DrawFileChooser("CCALabel", "CCAInput", "CCABtn", 20.0f, false, "Cost MCL Address:", 130.0f, m_strCostCLAddress, 320.0f, "", false, true);

        GUILayout.Space(5);

        m_strCostAmount = DrawFileChooser("CAXLabel", "CAXInput", "CAXBtn", 20.0f, false, "Cost Amount:", 130.0f, m_strCostAmount, 320.0f, "", false, true);

        GUILayout.Space(5);

        m_strSenderCLAddress = DrawFileChooser("CRALabel", "CRAInput", "CRABtn", 20.0f, false, "Sender MCL Address:", 130.0f, m_strSenderCLAddress, 320.0f, "", false, true);

        GUILayout.Space(10);

        GUILayout.BeginHorizontal();       
        fBtnWidth = 160.0f;
        arrLO[0] = GUILayout.Width(fBtnWidth);
        arrLO[1] = GUILayout.Height(25.0f);
        fXSpace = (ms_kSig.position.width - fBtnWidth) * 0.5f;
        GUILayout.Space(fXSpace);        
        if (GUILayout.Button("Publish Contract", arrLO))
        {
            if (m_bCBInited == false)
            {
                if (EditorUtility.DisplayDialog("Erorr", "MagnaChain has not initialized", "OK"))
                {
                    return;
                }
            }            

            if ( string.IsNullOrEmpty(m_strLuaFile))
            {
                if (EditorUtility.DisplayDialog("Prompt", "Please select contract file first.", "OK"))
                {
                    return;
                }
            }

            if (m_bEditLuaFile)
            {
                if (EditorUtility.DisplayDialog("Prompt", "You may have modified contract file, please make sure the contract file have saved.", "Continue", "Cancel") == false)
                {
                    return;
                }
                m_bEditLuaFile = false;
            }

            string strTxt = null;

            try
            {
                strTxt = File.ReadAllText(m_strLuaFile);
            }
            catch ( Exception e)
            {
                LogError(e.Message);
                return;
            }

            if ( string.IsNullOrEmpty(strTxt))
            {
                EditorUtility.DisplayDialog("Prompt", "The contract content is empty!", "OK");
                return;
            }            

            float fCostAmount = 0.0f;

            try
            {
                fCostAmount = Convert.ToSingle(m_strCostAmount);
            }
            catch
            {
                fCostAmount = 0.0f;
            }
            bool bRet = m_kCB.PublishContract(out ms_strContractAddress, strTxt, m_strCostCLAddress, fCostAmount, m_strSenderCLAddress);
            if (bRet == false)
            {
                LogError("Publish contract failed: " + m_strLuaFile);
                return;
            }            
            
            LogSys("Publish contract successfully, contract address: " + ms_strContractAddress);

            string strPrefix = Application.dataPath;
            string strKey = strPrefix + "#ms_strContractAddress";
            PlayerPrefs.SetString(strKey, ms_strContractAddress);
            PlayerPrefs.Save();
        }
        GUILayout.EndHorizontal();

        GUILayout.Space(15);

        ms_strContractAddress = DrawFileChooser("CADLabel", "CADInput", "CADBtn", 20.0f, false, "Contract Address:", 120.0f, ms_strContractAddress, 330.0f, "", false, true);

        GUILayout.Space(5);

        m_strCallFunction = DrawFileChooser("CFLabel", "CFInput", "CFBtn", 20.0f, false, "Call Function:", 120.0f, m_strCallFunction, 330.0f, "", false, true);

        GUILayout.Space(5);

        m_strCallArgs = DrawFileChooser("CALabel", "CAInput", "CABtn", 20.0f, false, "Call Arguments:", 120.0f, m_strCallArgs, 330.0f, "", false, true);
        
        GUILayout.Space(15);

        GUILayout.BeginHorizontal();

        float fTogWidth = 120.0f;
        arrLO[0] = GUILayout.Width(fTogWidth);
        float fTogX = 40.0f;
        GUILayout.Space(fTogX);
        m_bContractSendCall = GUILayout.Toggle(m_bContractSendCall, "Send Call", arrLO);

        fBtnWidth = 180.0f;
        arrLO[0] = GUILayout.Width(fBtnWidth);
        fXSpace = ms_kSig.position.width * 0.5f - fTogWidth - fTogX - fBtnWidth * 0.5f;
        GUILayout.Space(fXSpace);
        if (GUILayout.Button("Call Contract Function", arrLO))
        {
            if (m_bCBInited == false)
            {
                if (EditorUtility.DisplayDialog("Erorr", "MagnaChain has not initialized", "OK"))
                {
                    return;
                }
            }

            if ( string.IsNullOrEmpty(ms_strContractAddress))
            {
                if (EditorUtility.DisplayDialog("Prompt", "Please input contract address first.", "OK"))
                {
                    return;
                }
            }

            if ( string.IsNullOrEmpty(m_strCallFunction))
            {
                if (EditorUtility.DisplayDialog("Prompt", "Please input call function name first.", "OK"))
                {
                    return;
                }
            }

            float fCostAmount = 0.0f;
            try
            {
                fCostAmount = Convert.ToSingle(m_strCostAmount);
            }
            catch
            {
                fCostAmount = 0.0f;
            }

            object[] arrArg = null;
            if ( !string.IsNullOrEmpty(m_strCallArgs))
            {
                //char[] arrSpt = new char[1];
                //arrSpt[0] = ' ';
                arrArg = System.Text.RegularExpressions.Regex.Split(m_strCallArgs, @"\s+");//m_strCallArgs.Split(arrSpt);
            }

            string strRet = m_kCB.CallContractFunction(m_bContractSendCall, ms_strContractAddress, m_strCostCLAddress, fCostAmount, m_strSenderCLAddress, m_strCallFunction, arrArg);
            if ( string.IsNullOrEmpty(strRet))
            {
                LogError("CallContractFunction failed!");
                return;
            }
            LogSys("CallContractFunction succeed: " + strRet);
        }

        //fXSpace = ms_kSig.position.width - fBtnOffset * 2.0f - fBtnWidth * 2.0f;
        //GUILayout.Space(fXSpace);
        //if (GUILayout.Button("SendCall Contract", arrLO))
        //{
        //    if (m_bCBInited == false)
        //    {
        //        if (EditorUtility.DisplayDialog("Erorr", "MagnaChain has not initialized", "OK"))
        //        {
        //            return;
        //        }
        //    }

        //    if (string.IsNullOrEmpty(m_strContractAddress))
        //    {
        //        if (EditorUtility.DisplayDialog("Prompt", "Please publish contract first.", "OK"))
        //        {
        //            return;
        //        }
        //    }

        //    if (string.IsNullOrEmpty(m_strCallFunction))
        //    {
        //        if (EditorUtility.DisplayDialog("Prompt", "Please input call function name first.", "OK"))
        //        {
        //            return;
        //        }
        //    }

        //    string strRet = m_kCB.SendCallContract(m_strContractAddress, m_strCallFunction, m_strSenderAddress, m_strCallArgs);
        //    if (string.IsNullOrEmpty(strRet))
        //    {
        //        LogError("SendCallContract failed: " + m_strContractAddress);
        //        return;
        //    }
        //    LogSys("SendCallContract succeed: " + strRet);
        //}
        GUILayout.EndHorizontal();
    }
        
    private void DrawSettingsUI(float fYOffset)
    {
        if (m_bNeedResetFocus)
        {            
            GUI.FocusControl(null);
            m_bNeedResetFocus = false;
        }

        GUILayout.Space(10);

        RPC_HOST = DrawFileChooser("RHLabe", "RHInput", "RHBtn", 20.0f, false, "Rpc Host:", 120.0f, RPC_HOST, 330.0f, "", false, true);
        GUILayout.Space(10);

        string strRpcPort = RPC_PORT.ToString();
        strRpcPort = DrawFileChooser("RPLabe", "RPInput", "RPBtn", 20.0f, false, "Rpc port:", 120.0f, strRpcPort, 330.0f, @"8332", false, true);
        try
        {
            RPC_PORT = Convert.ToInt32(strRpcPort);
        }
        catch(Exception e)
        {
            RPC_PORT = 8332;
        }
        GUILayout.Space(10);

        RPC_USERNAME = DrawFileChooser("RUNLabe", "RUNInput", "RUNBtn", 20.0f, false, "Rpc user name:", 120.0f, RPC_USERNAME, 330.0f, "", false, true);
        GUILayout.Space(10);

        RPC_PASSWORD = DrawFileChooser("RPSLabe", "RPSInput", "RPSBtn", 20.0f, false, "Rpc password:", 120.0f, RPC_PASSWORD, 330.0f, "", true, true);
        GUILayout.Space(10);

        GUILayout.BeginHorizontal();
        GUILayout.Space(20.0f);                
        GUILayout.Label("Network:", GUILayout.Width(120.0f));
        //GUILayout.Space(10.0f);
        string[] arrNetType = new string[3];
        arrNetType[0] = "MAIN";
        arrNetType[1] = "TEST_NET";
        arrNetType[2] = "REG_TEST";
        RPC_NET_TYPE = (NETWORK_TYPE)EditorGUILayout.Popup((int)RPC_NET_TYPE, arrNetType, GUILayout.Width(90));
        GUILayout.EndHorizontal();

        GUILayout.Space(25);

        GUILayout.BeginHorizontal();
        float fBtnWidth = 150.0f;
        float fX = ms_kSig.position.width * 0.5f - fBtnWidth * 0.5f;        
        GUILayout.Space(fX);
        GUILayoutOption[] arrLO = new GUILayoutOption[2];
        arrLO[0] = GUILayout.Width(fBtnWidth);
        arrLO[1] = GUILayout.Height(40);
        GUI.SetNextControlName("SaveBtn");
        if (GUILayout.Button("Save", arrLO))
        {
            string strKeyPrefix = Application.dataPath;
            if ( !string.IsNullOrEmpty(RPC_HOST))
            {
                PlayerPrefs.SetString(strKeyPrefix + "#RPC_HOST", RPC_HOST);                
            }

            if (RPC_PORT > 0)
            {
                PlayerPrefs.SetInt(strKeyPrefix + "#RPC_PORT", RPC_PORT);
            }


            if (!string.IsNullOrEmpty(RPC_USERNAME))
            {
                PlayerPrefs.SetString(strKeyPrefix + "#RPC_USERNAME", RPC_USERNAME);
            }


            if (!string.IsNullOrEmpty(RPC_PASSWORD))
            {
                PlayerPrefs.SetString(strKeyPrefix + "#RPC_PASSWORD", RPC_PASSWORD);
            }
                        
            PlayerPrefs.SetInt(strKeyPrefix + "#RPC_NET_TYPE", (int)RPC_NET_TYPE);

            PlayerPrefs.Save();

            if (m_kCB != null)
            {
                m_kCB.Release();
                m_kCB = null;
            }

            m_kCB = new IxMagnaChainBridge();
            m_bCBInited = m_kCB.Initialize(RPC_HOST, RPC_PORT, RPC_USERNAME, RPC_PASSWORD, RPC_NET_TYPE);
            if (m_bCBInited == false)
            {
                if (EditorUtility.DisplayDialog("Erorr", "Initialize RCP client failed.", "OK"))
                {
                    return;
                }
            }

            if ( !string.IsNullOrEmpty(ms_strImportMasterKeyName) && !string.IsNullOrEmpty(ms_strMasterKey))
            {
                m_kCB.ImportMasterExtKey(ms_strImportMasterKeyName, ms_strMasterKey);
            }
        }
        //GUI.FocusControl("SaveBtn");
        GUILayout.EndHorizontal();

        GUILayout.Space(40);        
        ms_strImportMasterKeyName = DrawFileChooser("IMKNLabe", "IMKNInput", "IMKNBtn", 20.0f, false, "Master key name:", 120.0f, ms_strImportMasterKeyName, 330.0f, "", false, true);

        GUILayout.Space(10);        
        ms_strMasterKey = DrawFileChooser("MKLabe", "MKInput", "MKBtn", 20.0f, false, "Master key:", 120.0f, ms_strMasterKey, 330.0f, "", false, true);

        GUILayout.Space(20);

        GUILayout.BeginHorizontal();
        fBtnWidth = 180.0f;
        fX = ms_kSig.position.width * 0.5f - fBtnWidth * 0.5f;
        GUILayout.Space(fX);
        arrLO[0] = GUILayout.Width(fBtnWidth);
        arrLO[1] = GUILayout.Height(30);
        GUI.SetNextControlName("ImportMasterKey");
        if (GUILayout.Button("Import Master Key", arrLO))
        {
            if ( m_kCB == null )
            {
                if (EditorUtility.DisplayDialog("Prompt", "MagnaChain has not initialized.", "OK"))
                {
                    return;
                }
            }

            if ( string.IsNullOrEmpty(ms_strImportMasterKeyName))
            {
                if (EditorUtility.DisplayDialog("Prompt", "Please input master key name.", "OK"))
                {
                    return;
                }
            }

            if (string.IsNullOrEmpty(ms_strMasterKey))
            {
                if (EditorUtility.DisplayDialog("Prompt", "Please input master key.", "OK"))
                {
                    return;
                }
            }

            if ( m_kCB.IsMasterExtKeyExist(ms_strImportMasterKeyName))
            {
                if (EditorUtility.DisplayDialog("Prompt", "The master key has imported: " + ms_strImportMasterKeyName, "OK"))
                {
                    return;
                }
            }

            if ( m_kCB.ImportMasterExtKey(ms_strImportMasterKeyName, ms_strMasterKey) == false )
            {
                if (EditorUtility.DisplayDialog("Error", "Import master key failed!", "OK"))
                {
                    return;
                }
            }
            else
            {
                string strKeyPrefix = Application.dataPath;               

                PlayerPrefs.SetString(strKeyPrefix + "#ms_strImportMasterKeyName", ms_strImportMasterKeyName);
                PlayerPrefs.SetString(strKeyPrefix + "#ms_strMasterKey", ms_strMasterKey);
                PlayerPrefs.Save();

                if (EditorUtility.DisplayDialog("Done", "Import master key succeed.", "OK"))
                {
                    return;
                }
            }
        }
        GUILayout.EndHorizontal();
    }

    private string DrawFileChooser(string strLableCtrlName, string strInputCtrlName, string strBtnCtrlName, float fXOffset, bool bFileSel, string strLabel, float fLableWidth, string strFilePath, float fTextFiledWidth, string strDefaultOpenPath, bool bPasswordShow = false, bool bHideOpenBtn = false, string strFileExt = null, int iBtnWidth = 70, int iReseverWidth = 0)
    {
        GUILayout.BeginHorizontal();
        GUILayout.Space(fXOffset);
        GUI.SetNextControlName(strLableCtrlName);
        GUILayout.Label(strLabel, GUILayout.Width(fLableWidth));    // 120
        //strFilePath = EditorGUILayout.TextField(strFilePath, GUILayout.Width(fTextFiledWidth));   // 330
        float fTFWidth;

        if ( !bHideOpenBtn )
        {
            fTFWidth = ms_kSig.position.width - fXOffset - fLableWidth - 10.0f - iBtnWidth - 20.0f - 10.0f - iReseverWidth;
        }
        else
        {
            fTFWidth = ms_kSig.position.width - fXOffset - fLableWidth - 10.0f - 20.0f - 10.0f - iReseverWidth;
        }

        GUILayoutOption[] arrLO = new GUILayoutOption[2];
        arrLO[0] = GUILayout.Height(18.0f);
        arrLO[1] = GUILayout.Width(fTFWidth);
        GUI.SetNextControlName(strInputCtrlName);
        if ( bPasswordShow )
        {
            //strFilePath = GUILayout.PasswordField(strFilePath, '*', 512, GUILayout.Width(fTextFiledWidth));
            strFilePath = EditorGUILayout.PasswordField(strFilePath, arrLO);
        }
        else
        {
            //strFilePath = GUILayout.TextField(strFilePath, 512, GUILayout.Width(fTextFiledWidth));
            strFilePath = EditorGUILayout.TextField(strFilePath, arrLO); 
        }

        if (bHideOpenBtn == false)
        {
            GUILayout.Space(10);
            arrLO[1] = GUILayout.Width(iBtnWidth);
            GUI.SetNextControlName(strBtnCtrlName);           
            if (GUILayout.Button("Browser", arrLO))
            {
                string tempPath;
                if ( bFileSel == false )
                {
                    tempPath = EditorUtility.OpenFolderPanel("Choose " + strLabel, strDefaultOpenPath, "");

                    if (!string.IsNullOrEmpty(tempPath))
                    {
                        strFilePath = tempPath + "/";
                    }
                }
                else
                {
                    tempPath = EditorUtility.OpenFilePanel("Choose " + strLabel, strDefaultOpenPath, strFileExt);
                    strFilePath = tempPath;
                }                
            }
        }
        GUILayout.EndHorizontal();
        return strFilePath;
    }
}