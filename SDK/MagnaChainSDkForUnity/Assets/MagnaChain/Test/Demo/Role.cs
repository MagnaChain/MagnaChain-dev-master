using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using UnityEngine.UI;
using MagnaChainEx;
using System;
using CellLink.RPC;

public class Role : MonoBehaviour
{
    private string m_strMasterKey;    

    private string m_strAddress;
    private string m_strBTAddress;

    private string m_strDestAddress;

    private float m_fBalance;

    private InputField m_ifDestAddress;
    private InputField m_ifMoney;

    private Text m_txtBalance;
    private Text m_txtAddress;
    private Text m_txtBTAddress;

    private IxMagnaChainBridge m_kCLB;
    private string m_strMasterKeyName;

    private Role m_kOtherRole;
    private int m_iID;

    private float m_fUpdateTick = -1.0f;
    private float m_fUpdateTime = 2.0f;

    private const string SAVE_ADDRESS_NAME_PREFIX = "MagnaChain_TiG_";

    public void Initialize(IxMagnaChainBridge kCLB, string strMasterKey, string strAddress, string strDestAddress, Role kOther, int iID)
    {
        m_kCLB = kCLB;
        m_kOtherRole = kOther;
        m_iID = iID;

        m_strMasterKey = strMasterKey;        
        m_strAddress = strAddress;
        if ( m_strDestAddress == null )
        {
            m_strDestAddress = strDestAddress;
        }        

        string strSaveName = SAVE_ADDRESS_NAME_PREFIX + m_iID;
        if ( PlayerPrefs.HasKey(strSaveName))
        {
            m_strAddress = PlayerPrefs.GetString(strSaveName);
            m_kOtherRole.SetDestCLAddress(m_strAddress);
        }

        //string strName;
        string strPath;        

        m_kCLB.DecodeMCLAddress(m_strAddress, out m_strMasterKeyName, out strPath, out m_strBTAddress);
        
        List<string> listAddress = new List<string>();
        listAddress.Add(strAddress);
        m_kCLB.ImportMasterExtKey(strMasterKey, listAddress);
        
        if ( m_txtAddress != null )
        {
            m_txtAddress.text = strAddress;
        }
        if ( m_txtBTAddress != null )
        {
            m_txtBTAddress.text = m_strBTAddress;
        }
        if ( m_ifDestAddress != null )
        {
            m_ifDestAddress.text = m_strDestAddress;
        }

        m_fUpdateTick = m_fUpdateTime;
        UpdateBalance();
    }

	// Use this for initialization
	void Start ()
    {
        Transform trsK;

        trsK = transform.Find("CLAddress/Value");
        m_txtAddress = trsK.GetComponent<Text>();
        if ( m_strAddress != null )
        {
            m_txtAddress.text = m_strAddress;
        }

        trsK = transform.Find("BTAddress/Value");
        m_txtBTAddress = trsK.GetComponent<Text>();
        if (m_strBTAddress != null)
        {
            m_txtBTAddress.text = m_strBTAddress;
        }

        trsK = transform.Find("Balance/Value");
        m_txtBalance = trsK.GetComponent<Text>();

        trsK = transform.Find("toCLAddress/if_toAddress");
        m_ifDestAddress = trsK.GetComponent<InputField>();
        if ( m_strDestAddress != null )
        {
            m_ifDestAddress.text = m_strDestAddress;
        }

        trsK = transform.Find("toMoney/if_toMoney");
        m_ifMoney = trsK.GetComponent<InputField>();

        trsK = transform.Find("btn_transfer");
        Button btnK = trsK.GetComponent<Button>();
        btnK.onClick.AddListener(OnClickTransfer);
    }

    public void SetDestCLAddress(string strAddress)
    {
        m_strDestAddress = strAddress;
        if ( m_ifDestAddress != null )
        {
            m_ifDestAddress.text = strAddress;
        }
    }
	
    public void OnClickTransfer()
    {
        string strMoney = m_ifMoney.text;
        string strDestAddress = m_ifDestAddress.text;

        string strChangeCLAddress = null;
        if ( m_iID >1000)
        {
            strChangeCLAddress = m_kCLB.GenerateNewMCLAddress(m_strMasterKeyName);
        }

        if ( string.IsNullOrEmpty(strMoney) || string.IsNullOrEmpty(strDestAddress))
        {
            MessageBox.ms_kSig.Show("Prompt", "Money or DestAddress is null");
            return;
        }

        float fMoney = Convert.ToSingle(strMoney);
        if ( fMoney <= 0.0f )
        {
            MessageBox.ms_kSig.Show("Prompt", "Money must > 0");
            return;
        }

        if (fMoney > m_fBalance)
        {
            MessageBox.ms_kSig.Show("Prompt", "Not enough money");
            return;
        }

        IxMagnaChainBridge.ON_TRANSFER_DONE fnTransferDone = delegate (RPCError kError, string strTxid)
        {
            if ( kError != null )
            {
                Debug.LogError(kError.Message);
                return;
            }

            Debug.Log("Transfer done.");

            if ( m_iID > 1000 )
            {
                m_strAddress = strChangeCLAddress;
                m_txtAddress.text = strChangeCLAddress;

                string strName;
                string strPath;
                string strBTAddress;

                m_kCLB.DecodeMCLAddress(strChangeCLAddress, out strName, out strPath, out strBTAddress);
                m_txtBTAddress.text = strBTAddress;

                m_kOtherRole.SetDestCLAddress(strChangeCLAddress);

                string strSaveName = SAVE_ADDRESS_NAME_PREFIX + m_iID;
                PlayerPrefs.SetString(strSaveName, strChangeCLAddress);
            }            

            m_fUpdateTick = m_fUpdateTime;
            UpdateBalance();
        };
        m_kCLB.TransferMCLToMCLAddressAsync(m_strAddress, strDestAddress, fMoney, fnTransferDone, -1.0f, strChangeCLAddress);
    }

    private void UpdateBalance()
    {
        if ( m_kCLB == null && string.IsNullOrEmpty(m_strAddress))
        {
            return;
        }

        if ( m_fUpdateTick < 0.0f )
        {
            return;
        }

        m_fUpdateTick += Time.deltaTime;
        if ( m_fUpdateTick < m_fUpdateTime )
        {
            return;
        }

        m_fUpdateTick = -1.0f;
        IxMagnaChainBridge.ON_GET_BALANCE fnBalance = delegate (float fBalance)
        {
            //Debug.Log("ON_GET_BALANCE: " + fBalance + " time: " + Time.time);

            m_fBalance = fBalance;
            if ( m_txtBalance != null )
            {
                m_txtBalance.text = fBalance.ToString();
            }
            m_fUpdateTick = 0.0f;
        };
        m_kCLB.GetBalanceAsync(m_strAddress, fnBalance);
    }

	// Update is called once per frame
	void Update ()
    {
        UpdateBalance();
    }
}
