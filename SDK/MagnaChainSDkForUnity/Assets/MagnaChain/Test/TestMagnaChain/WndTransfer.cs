using UnityEngine;
using UnityEngine.UI;
using System.Collections;
using System;

public class WndTransfer : MonoBehaviour
{
    private InputField m_ifDestAddress;
    private InputField m_ifChangeAddress;
    private InputField m_ifMoney;

    private TestMagnaChain m_kTCL;


    public void SetOwer(TestMagnaChain kT)
    {
        m_kTCL = kT;
    }
    
	// Use this for initialization
	void Start ()
    {
        Transform trsK;

        trsK = transform.Find("bk/DestBTAddress/input");
        m_ifDestAddress = trsK.GetComponent<InputField>();

        trsK = transform.Find("bk/ChangeBTAddress/input");
        m_ifChangeAddress = trsK.GetComponent<InputField>();

        trsK = transform.Find("bk/Money/input");
        m_ifMoney = trsK.GetComponent<InputField>();

        Button btnK;

        trsK = transform.Find("bk/btn_transfer");
        btnK = trsK.GetComponent<Button>();
        btnK.onClick.AddListener(OnClickTransfer);

        trsK = transform.Find("bk/btn_cancel");
        btnK = trsK.GetComponent<Button>();
        btnK.onClick.AddListener(OnClickCancel);

        gameObject.SetActive(false);
    }

    public void OnClickTransfer()
    {
        string strDestAddr = m_ifDestAddress.text;
        string strChangeAddr = m_ifChangeAddress.text;
        string strMoney = m_ifMoney.text;

        if ( string.IsNullOrEmpty(strMoney) || string.IsNullOrEmpty(strDestAddr))
        {
            return;
        }

        float fMoney = 0.0f;
        try
        {
            fMoney = Convert.ToSingle(strMoney);
        }
        catch( Exception e)
        {
            Debug.LogError(e.Message);
            return;
        }

        if ( fMoney <= 0.0f )
        {
            return;
        }

        m_kTCL.Transfer(strDestAddr, strChangeAddr, fMoney);
    }

    public void OnClickCancel()
    {
        gameObject.SetActive(false);
    }
	
	// Update is called once per frame
	void Update ()
    {
	
	}
}
