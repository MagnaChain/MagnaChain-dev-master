using UnityEngine;
using UnityEngine.UI;
using System.Collections;

public class MessageBox : MonoBehaviour
{
    public static MessageBox ms_kSig;

    private Text m_txtTitle;
    private Text m_txtContent;

    public MessageBox()
    {
        ms_kSig = this;
    }
    
	// Use this for initialization
	void Start ()
    {
        Transform trsK;

        trsK = transform.Find("bk/Title");
        m_txtTitle = trsK.GetComponent<Text>();

        trsK = transform.Find("bk/txt_bk/Text");
        m_txtContent = trsK.GetComponent<Text>();

        trsK = transform.Find("bk/btn_ok");
        Button btnOK = trsK.GetComponent<Button>();
        btnOK.onClick.AddListener(OnClickOK);

        gameObject.SetActive(false);
    }

    public void OnClickOK()
    {
        gameObject.SetActive(false);
    }

    public void Show(string strTitle, string strContent)
    {
        m_txtTitle.text = strTitle;
        m_txtContent.text = strContent;
        gameObject.SetActive(true);
    }
	
	// Update is called once per frame
	void Update ()
    {
	
	}
}
