using UnityEngine;
using System.Collections;
using UnityEngine.UI;
using CellLink;
using System;
using CellLink.RPC;
using System.Net;
using MagnaChainEx;

public class TestMagnaChain : MonoBehaviour
{
    private string m_strInfo = "";

    private Text m_txtInfo;

    private InputField m_ifMasterKey;    

    private InputField m_ifCLAddr;
    private InputField m_ifMasterKeyName;
    private InputField m_ifBTAddr;
    private InputField m_ifPriKey;

    private InputField m_ifKeyPath;

    //private MagnaChain.Network m_CLNet = MagnaChain.Network.Main;

    private IxMagnaChainBridge m_kCLB;

    private string m_strMasterKeyName = "TestMagnaChain_Cat";

    private GameObject m_goTransferWnd;


    void Awake()
    {
        //ExtKey kEK0 = ExtKey.CreateByAid("chemo is good");
        //Debug.Log("0: " + kEK0.ToString(MagnaChain.Network.Main));

        //ExtKey kEK1 = ExtKey.CreateByAid("chemo is good ");
        //Debug.Log("1: " + kEK1.ToString(MagnaChain.Network.Main));

        //ExtKey kEK2 = ExtKey.CreateByAid("chemoi is good");
        //Debug.Log("2: " + kEK2.ToString(MagnaChain.Network.Main));

        //ExtKey kEK3 = ExtKey.CreateByAid("chemo is good");
        //Debug.Log("3: " + kEK3.ToString(MagnaChain.Network.Main));

        //ExtKey kEK4 = ExtKey.CreateByAid("chemo is good, ok go!");
        //Debug.Log("4: " + kEK4.ToString(MagnaChain.Network.Main));

        //ExtKey kEK5 = ExtKey.CreateByAid("chemo is good, ok god, rteklwq3reew, 23sdfsfl;fwer.,goo3234,sdfwersfsxgfdf dsgfsdfgsdgdgdf534dsfdgfs9, sdrwersxdf2345dsfsdfl.sdfsdfrwrw3sdrsdf234sdfsfsrsdfc,dswer");
        //Debug.Log("5: " + kEK5.ToString(MagnaChain.Network.Main));

        //ExtKey kEK6 = ExtKey.CreateByAid("chemo is good, ok god, rteklwq3reew, 23sdfsfl;fwep.,goo3234,sdfwersfsxgfdf dsgfsdfgsdgdgdf534dsfdgfs9, sdrwersxdf2345dsfsdfl.sdfsdfrwrw3sdrsdf234sdfsfsrsdfc,dswer");
        //Debug.Log("6: " + kEK6.ToString(MagnaChain.Network.Main));

        //ExtKey kEK7 = ExtKey.CreateByAid("chemo is good, ok god, rteklwq3reew, 23sdfsfl;fwep.,goo3234,sdfwersfsxgfdf dsgfsdfgsdgdgdf534dsfdgfs9, sdrwersxdf2345dsfsdfl.sdfsdfrwrw3sdrsdf234sdfsfsrsdfc,dswer9");
        //Debug.Log("7: " + kEK7.ToString(MagnaChain.Network.Main));

        //ExtKey kEK8 = ExtKey.CreateByAid("chemo is good");
        //Debug.Log("8: " + kEK8.ToString(MagnaChain.Network.Main));

        //ExtKey kEK9 = ExtKey.CreateByAid("chemo is good, ok god, rteklwq3reew, 23sdfsfl;fwep.,goo3234,sdfwersfsxgfdf dsgfsdfgsdgdgdf534dsfdgfs9, sdrwersxdf2345dsfsdfl.sdfsdfrwrw3sdrsdf234sdfsfsrsdfc,dswer");
        //Debug.Log("9: " + kEK9.ToString(MagnaChain.Network.Main));
    }

	// Use this for initialization
	void Start ()
    {
        Transform trsK = transform.Find("btn_createMasterKey");
        Button btnT = trsK.GetComponent<Button>();
        btnT.onClick.AddListener(OnClickCreateMasterKey);

        trsK = transform.Find("btn_importMasterKey");
        btnT = trsK.GetComponent<Button>();
        btnT.onClick.AddListener(OnClickImportMasterKey);

        //trsK = transform.Find("btn_createMasterKeyByPriKey");
        //btnT = trsK.GetComponent<Button>();
        //btnT.onClick.AddListener(OnClickCreateMasterKeyByPirKey);         

        trsK = transform.Find("SR/Viewport/Info");
        m_txtInfo = trsK.GetComponent<Text>();

        trsK = transform.Find("MasterKey/input");
        m_ifMasterKey = trsK.GetComponent<InputField>();

        //trsK = transform.Find("PrivateKey/input");
        //m_ifPriKey = trsK.GetComponent<InputField>();

        trsK = transform.Find("CLAddress/input");
        m_ifCLAddr = trsK.GetComponent<InputField>();

        trsK = transform.Find("CLAddress/master_key_name/input_BTAddress");
        m_ifMasterKeyName = trsK.GetComponent<InputField>();

        trsK = transform.Find("CLAddress/bt_Address/input_BTAddress");
        m_ifBTAddr = trsK.GetComponent<InputField>();

        trsK = transform.Find("CLAddress/pri_key/input_prikey");
        m_ifPriKey = trsK.GetComponent<InputField>();

        trsK = transform.Find("CLAddress/key_path/input_keypath");
        m_ifKeyPath = trsK.GetComponent<InputField>();

        trsK = transform.Find("CLAddress/btn_parent/btn_new");
        btnT = trsK.GetComponent<Button>();
        btnT.onClick.AddListener(OnClickNewCLAddress);

        trsK = transform.Find("CLAddress/btn_parent/btn_btAddress");
        btnT = trsK.GetComponent<Button>();
        btnT.onClick.AddListener(OnClickBTA);

        trsK = transform.Find("CLAddress/btn_parent/btn_query");
        btnT = trsK.GetComponent<Button>();
        btnT.onClick.AddListener(OnClickQuery);

        trsK = transform.Find("CLAddress/btn_parent/btn_transfer");
        btnT = trsK.GetComponent<Button>();
        btnT.onClick.AddListener(OnClickTransfer);

        trsK = transform.Find("wnd_transfer");
        m_goTransferWnd = trsK.gameObject;
        if (m_goTransferWnd.activeSelf == false )
        {
            m_goTransferWnd.SetActive(true);
        }
        WndTransfer kWT = m_goTransferWnd.AddComponent<WndTransfer>();
        kWT.SetOwer(this);

        InitializeRPCClient();
    }

    private void InitializeRPCClient()
    {
        m_kCLB = new IxMagnaChainBridge();
        string strHost = "http://127.0.0.1";
        int iPort = 8332;
        string strUser = "user";
        string strPwd = "pwd";
        NETWORK_TYPE eNT = NETWORK_TYPE.MAIN;

        TextOut("Initialize RPC client: " + strHost + ":" + iPort.ToString());        
        bool bRet = m_kCLB.Initialize(strHost, iPort, strUser, strPwd, eNT);
        if ( bRet == false )
        {
            TextOut("Initialize RPC client failed!", "red");
        }               
        else
        {
            TextOut("Initialize RPC client succeed.");
        }
    }

    //private void TextOutKey(Key kPriKey)
    //{        
    //    m_strInfo += ("<color=red>Private Key(Raw):</color> " + kPriKey.ToHex() + "\n");

    //    //BitcoinExtKey kBEKey = kKey.GetWif(m_CLNet);
    //    m_strInfo += ("<color=red>Private Key(Wif):</color> " + kPriKey.ToString(m_CLNet) + "\n");

    //    PubKey kPubKey = kPriKey.PubKey;
    //    m_strInfo += ("<color=lime>Public Key(Raw):</color> " + kPubKey.ToString() + "\n");
    //    m_strInfo += ("<color=lime>Public Key(Wif):</color> " + kPubKey.ToString(m_CLNet) + "\n");

    //    m_strInfo += ("<color=yellow>Public Key Address:</color> " + kPubKey.GetAddress(m_CLNet).ToString() + "\n");

    //    m_strInfo += ("<color=#9B30FF>--------------------------------------------------------------------------------------------------------------------------------------</color>\n");

    //    m_txtInfo.text = m_strInfo;
    //}

    private void TextOut(string strInfo, string strColor = null)
    {
        string strLine;

        if ( string.IsNullOrEmpty(strColor))
        {
            strLine = strInfo + "\n";
        }
        else
        {
            strLine = "<color=" + strColor + ">" + strInfo + "</color>\n";
        }
        m_strInfo += strLine;       

        m_txtInfo.text = m_strInfo;
    }

    //public void OnClickCreateMasterKeyByPriKey()
    //{
    //    string strPriKey = m_ifPriKey.text;
    //    if ( string.IsNullOrEmpty(strPriKey))
    //    {
    //        return;
    //    }

    //    m_kCLB.RemoveMasterKey(m_strMasterKeyName);
    //    string strMasterKey = m_kCLB.CreateMasterExtKey(m_strMasterKeyName, strPriKey);
    //    if (string.IsNullOrEmpty(strMasterKey))
    //    {
    //        TextOut("Create master key failed!", "red");
    //        return;
    //    }
    //    m_ifMasterKey.text = strMasterKey;
    //    TextOut("New master key: " + strMasterKey, "#80FF80");
    //}

    public void OnClickCreateMasterKey()
    {
        //if ( m_kCLB.IsMasterExtKeyExist(m_strMasterKeyName) )
        //{
        //    return;
        //}

        string strMasterKeyName = m_ifMasterKeyName.text;
        if ( string.IsNullOrEmpty(strMasterKeyName))
        {
            strMasterKeyName = NewMasterName();
        }
        string strMasterKey = m_kCLB.CreateMasterExtKey(strMasterKeyName);
        if ( string.IsNullOrEmpty(strMasterKey))
        {
            TextOut("Create master key failed!", "red");
            return;
        }
        m_ifMasterKey.text = strMasterKey;
        m_ifMasterKeyName.text = strMasterKeyName;
        TextOut("New master key: " + strMasterKey, "#80FF80");
    }

    public void OnClickImportMasterKey()
    {
        string strKey = m_ifMasterKey.text;
        if ( string.IsNullOrEmpty(strKey))
        {
            return;
        }

        string strMasterKeyName = m_ifMasterKeyName.text;
        if (string.IsNullOrEmpty(strMasterKeyName))
        {
            strMasterKeyName = NewMasterName();
        }

        if ( m_kCLB.ImportMasterExtKey(strMasterKeyName, strKey) )
        {
            m_ifMasterKeyName.text = strMasterKeyName;
            m_strMasterKeyName = strMasterKeyName;
            TextOut("Import new master key succeed.", "#80FF80");
        }
    }    

    private string NewMasterName()
    {
        string strPrefix = "TestMagnaChain_MN_";
        int iK = UnityEngine.Random.Range(1, 10000000);
        m_strMasterKeyName = strPrefix + iK.ToString();
        return m_strMasterKeyName;
    }

    public void OnClickNewCLAddress()
    {
        string strPath;
        string strCLAddress;

        strPath = m_ifKeyPath.text;
        if ( string.IsNullOrEmpty(strPath) )
        {
            strCLAddress = m_kCLB.GenerateNewMCLAddress(m_strMasterKeyName);
        }
        else
        {
            string strCelAddress = m_kCLB.GetCelAddressByPath(m_strMasterKeyName, strPath);
            if ( string.IsNullOrEmpty(strCelAddress))
            {
                TextOut("GetCelAddressByPath error, path: " + strPath, "red");
                return;
            }

            strCLAddress = m_kCLB.EncodeMCLAddress(m_strMasterKeyName, strPath, strCelAddress);
        }
        
        if ( string.IsNullOrEmpty(strCLAddress))
        {
            TextOut("Generate new MCL address error!", "red");
            return;
        }

        m_ifCLAddr.text = strCLAddress;

        string strName;
        string strBTAddress;

        if ( m_kCLB.DecodeMCLAddress(strCLAddress, out strName, out strPath, out strBTAddress))
        {
            m_ifBTAddr.text = strBTAddress;
            m_ifMasterKeyName.text = strName;

            ExtKey kEKey = m_kCLB.GetExtKeyByPath(m_strMasterKeyName, strPath);
            string strPriKey = kEKey.PrivateKey.GetWif(m_kCLB.network).ToWif();
            m_ifPriKey.text = strPriKey;

            TextOut("Generate new MCL address succeed.");
        }
    }

    public void OnClickBTA()
    {
        string strCLAddress = m_ifCLAddr.text;
        if ( string.IsNullOrEmpty(strCLAddress))
        {
            return;
        }

        string strName;
        string strBTAddress;
        string strPath;

        if (m_kCLB.DecodeMCLAddress(strCLAddress, out strName, out strPath, out strBTAddress))
        {
            m_ifBTAddr.text = strBTAddress;
            m_ifMasterKeyName.text = strName;
            m_ifKeyPath.text = strPath;

            TextOut("Decode MCL address succeed.");
        }
    }

    public void OnClickQuery()
    {
        if (string.IsNullOrEmpty(m_ifBTAddr.text))
        {
            return;
        }

        float fV = m_kCLB.GetBalanceByCelAddress(m_ifBTAddr.text);
        TextOut("Balance: " + fV);
    }

    public void OnClickTransfer()
    {
        m_goTransferWnd.SetActive(true);        
    }

    public void Transfer(string strDestBTAddress, string strChangeBTAddress, float fMoney)
    {
        string strPriKey = m_ifPriKey.text;
        if ( string.IsNullOrEmpty(strPriKey))
        {
            return;
        }

        RPCError kError;

        string strTxid = null;
        //if ( m_kCLB.TransferToBTAddress(strSrcCLAddr, strDestBTAddress, fMoney, out strTxid, -1.0f, strChangeBTAddress) == false )
        if ( m_kCLB.TransferRaw(out kError, strPriKey, strDestBTAddress, fMoney, out strTxid, -1.0f, strChangeBTAddress) == false )
        {
            TextOut("Transfer failed!");            
        }
        else
        {
            TextOut("Transfer succeed, txid: " + strTxid, "#80FF80");
        }        

        m_goTransferWnd.SetActive(false);
    }

    // Update is called once per frame
    void Update ()
    {
	
	}
}
