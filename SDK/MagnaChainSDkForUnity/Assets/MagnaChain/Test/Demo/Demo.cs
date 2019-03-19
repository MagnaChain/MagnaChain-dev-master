using UnityEngine;
using System.Collections;
using MagnaChainEx;
using UnityEngine.UI;

public class Demo : MonoBehaviour
{
    private IxMagnaChainBridge m_kCLB;

    private string m_strMasterKey0 = "xprv9s21ZrQH143K2xVPyvkgHKDH1LW7ZskLcV9BvJFbE9tZFqQ8An34PUUwReiC5TxabWAWrvxJwuimYYJ8qLUPvei4wbCe3jzwHDsjn9iBz3t";
    private string m_strAddress0 = "XFbpEJB6V9Jge7xgHUjGshETomnkeAwKCyI2k6xX99h4wHktihhx7KzASLWbKKJjMEGl0hC0";

    private string m_strMasterKey1 = "xprv9s21ZrQH143K4AAQ1eHhghLR1QJw2E9svFkSyHb7nV9RnnQPwb5X2DMfA8mLNuWLdq7d9i9HD3E6BSWSPAUHvzMSN5PsgqzjoFyQCw1MSJ7";
    private string m_strAddress1 = "XCeWi2AZZgtzDa7D73qVDPDaJp4BdZVvdvI2k6xX99h4wHktihhx7KzASLMiuLEVPCDl0hC0";

    private Role m_kRole0;
    private Role m_kRole1;    

    // Use this for initialization
    void Start ()
    {
        InitializeRPCClient();

        Transform trsK;

        trsK = transform.Find("Role0");
        m_kRole0 = trsK.gameObject.AddComponent<Role>();

        trsK = transform.Find("Role1");
        m_kRole1 = trsK.gameObject.AddComponent<Role>();

        trsK = transform.Find("MessageBox");
        if ( trsK.gameObject.activeSelf == false)
        {
            trsK.gameObject.SetActive(true);
        }
        trsK.gameObject.AddComponent<MessageBox>();

        trsK = transform.Find("btn_clearcache");
        Button btnK = trsK.gameObject.GetComponent<Button>();
        btnK.onClick.AddListener(OnClickClearCache);

        m_kRole0.Initialize(m_kCLB, m_strMasterKey0, m_strAddress0, m_strAddress1, m_kRole1, 0);
        m_kRole1.Initialize(m_kCLB, m_strMasterKey1, m_strAddress1, m_strAddress0, m_kRole0, 1);
    }

    private void InitializeRPCClient()
    {
        m_kCLB = new IxMagnaChainBridge();
        string strHost = "http://127.0.0.1";
        int iPort = 8332;
        string strUser = "user";
        string strPwd = "pwd";
        NETWORK_TYPE eNT = NETWORK_TYPE.MAIN;
        
        bool bRet = m_kCLB.Initialize(strHost, iPort, strUser, strPwd, eNT);        
    }

    public void OnClickClearCache()
    {
        PlayerPrefs.DeleteAll();
    }

    // Update is called once per frame
    void Update ()
    {
        if ( m_kCLB != null )
        {
            m_kCLB.OnUpdate(Time.deltaTime);
        }	
	}
}
