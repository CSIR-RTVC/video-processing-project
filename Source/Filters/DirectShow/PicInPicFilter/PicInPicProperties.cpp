#include "PicInPicProperties.h"


CUnknown * WINAPI PicInPicProperties::CreateInstance( LPUNKNOWN pUnk, HRESULT *pHr )
{
	PicInPicProperties *pNewObject = new PicInPicProperties(pUnk);
	if (pNewObject == NULL) 
	{
		*pHr = E_OUTOFMEMORY;
	}
	return pNewObject;
}

PicInPicProperties::PicInPicProperties( IUnknown *pUnk ) : 
FilterPropertiesBase(NAME("Pic in pic Properties"), pUnk, IDD_PIC_IN_PIC_DIALOG, IDS_PIC_IN_PIC_CAPTION)
{
	;
}

HRESULT PicInPicProperties::ReadSettings()
{
	initialiseControls();

	int nLength = 0;
	char szBuffer[BUFFER_SIZE];
	HRESULT hr = m_pSettingsInterface->GetParameter(TARGET_HEIGHT, sizeof(szBuffer), szBuffer, &nLength);
	if (SUCCEEDED(hr))
	{
		SetDlgItemText(m_Dlg, IDC_EDIT_TARGET_HEIGHT, szBuffer);
		m_targetHeight = atoi(szBuffer);
	}
	else
	{
		return E_FAIL;
	}
	hr = m_pSettingsInterface->GetParameter(TARGET_WIDTH, sizeof(szBuffer), szBuffer, &nLength);
	if (SUCCEEDED(hr))
	{
		SetDlgItemText(m_Dlg, IDC_EDIT_TARGET_WIDTH, szBuffer);
		m_targetWidth = atoi(szBuffer);
	}
	else
	{
		return E_FAIL;
	}

	hr = m_pSettingsInterface->GetParameter(SUB_PIC_WIDTH, sizeof(szBuffer), szBuffer, &nLength);
	if (SUCCEEDED(hr))
	{
		SetDlgItemText(m_Dlg, IDC_EDIT_SUBPICTURE_WIDTH, szBuffer);
		m_uiSubpicWidth = atoi(szBuffer);
	}
	else
	{
		return E_FAIL;
	}
	hr = m_pSettingsInterface->GetParameter(SUB_PIC_HEIGHT, sizeof(szBuffer), szBuffer, &nLength);
	if (SUCCEEDED(hr))
	{
		SetDlgItemText(m_Dlg, IDC_EDIT_SUBPICTURE_HEIGHT, szBuffer);
		m_uiSubpicHeight = atoi(szBuffer);
	}
	else
	{
		return E_FAIL;
	}

	hr = m_pSettingsInterface->GetParameter(OFFSET_X, sizeof(szBuffer), szBuffer, &nLength);
	if (SUCCEEDED(hr))
	{
		SetDlgItemText(m_Dlg, IDC_EDIT_SUBPICTURE_OFFSET_X, szBuffer);
		m_uiOffsetX = atoi(szBuffer);
	}
	else
	{
		return E_FAIL;
	}
	hr = m_pSettingsInterface->GetParameter(OFFSET_Y, sizeof(szBuffer), szBuffer, &nLength);
	if (SUCCEEDED(hr))
	{
		SetDlgItemText(m_Dlg, IDC_EDIT_SUBPICTURE_OFFSET_Y, szBuffer);
		m_uiOffsetY = atoi(szBuffer);
	}
	else
	{
		return E_FAIL;
	}
	return hr;
	//int nLength = 0;
	//char szBuffer[BUFFER_SIZE];
	//HRESULT hr = m_pSettingsInterface->GetParameter(ORIENTATION, sizeof(szBuffer), szBuffer, &nLength);
	//if (SUCCEEDED(hr))
	//{
	//	int nMixingMode = atoi(szBuffer);
	//	int nRadioID = RADIO_BUTTON_IDS[nMixingMode];
	//	long lResult = SendMessage(				// returns LRESULT in lResult
	//		GetDlgItem(m_Dlg, nRadioID),		// handle to destination control
	//		(UINT) BM_SETCHECK,					// message ID
	//		(WPARAM) 1,							// = 0; not used, must be zero
	//		(LPARAM) 0							// = (LPARAM) MAKELONG ((short) nUpper, (short) nLower)
	//		);
	//	return S_OK;
	//}
	//else
	//{
	//	return E_FAIL;
	//}
	//return hr;
}

HRESULT PicInPicProperties::OnApplyChanges( void )
{
	int nLength = 0;
	char szBuffer[BUFFER_SIZE];
	HRESULT hr;

	// Target dimensions
	nLength = GetDlgItemText(m_Dlg, IDC_EDIT_TARGET_WIDTH, szBuffer, BUFFER_SIZE);
	m_targetWidth = atoi(szBuffer);
	hr = m_pSettingsInterface->SetParameter(TARGET_WIDTH, szBuffer);
	nLength = GetDlgItemText(m_Dlg, IDC_EDIT_TARGET_HEIGHT, szBuffer, BUFFER_SIZE);
	m_targetHeight = atoi(szBuffer);
	m_pSettingsInterface->SetParameter(TARGET_HEIGHT, szBuffer);

	// Subpicture dimensions
	nLength = GetDlgItemText(m_Dlg, IDC_EDIT_SUBPICTURE_WIDTH, szBuffer, BUFFER_SIZE);
	m_targetWidth = atoi(szBuffer);
	m_pSettingsInterface->SetParameter(SUB_PIC_WIDTH, szBuffer);
	nLength = GetDlgItemText(m_Dlg, IDC_EDIT_SUBPICTURE_HEIGHT, szBuffer, BUFFER_SIZE);
	m_targetHeight = atoi(szBuffer);
	m_pSettingsInterface->SetParameter(SUB_PIC_HEIGHT, szBuffer);

	// Check if custom position should be used
	int iCheck = SendMessage( GetDlgItem(m_Dlg, IDC_CHK_OFFSET), (UINT) BM_GETCHECK,	0, 0);
	if (iCheck != 0)
	{
		// Use custom offset
		nLength = GetDlgItemText(m_Dlg, IDC_EDIT_SUBPICTURE_OFFSET_X, szBuffer, BUFFER_SIZE);
		m_uiOffsetX = atoi(szBuffer);
		hr = m_pSettingsInterface->SetParameter(OFFSET_X, szBuffer);
		nLength = GetDlgItemText(m_Dlg, IDC_EDIT_SUBPICTURE_OFFSET_Y, szBuffer, BUFFER_SIZE);
		m_uiOffsetY = atoi(szBuffer);
		hr = m_pSettingsInterface->SetParameter(OFFSET_Y, szBuffer);
	}
	return S_OK;
}

void PicInPicProperties::initialiseControls()
{
	short lower = 0;
	short upper = SHRT_MAX;

	InitCommonControls();

	SendMessage(GetDlgItem(m_Dlg, IDC_CMB_POSITION), CB_RESETCONTENT, 0, 0);
	//Add default option
	SendMessage(GetDlgItem(m_Dlg, IDC_CMB_POSITION), CB_ADDSTRING,	   0, (LPARAM)"Left bottom");
	SendMessage(GetDlgItem(m_Dlg, IDC_CMB_POSITION), CB_SELECTSTRING,  0, (LPARAM)"Left bottom");
	// Now populate the graphs
	SendMessage(GetDlgItem(m_Dlg, IDC_CMB_POSITION), CB_INSERTSTRING,  1, (LPARAM)"Left top");
	SendMessage(GetDlgItem(m_Dlg, IDC_CMB_POSITION), CB_INSERTSTRING,  2, (LPARAM)"Right top");
	SendMessage(GetDlgItem(m_Dlg, IDC_CMB_POSITION), CB_INSERTSTRING,  3, (LPARAM)"Right bottom");
	SendMessage(GetDlgItem(m_Dlg, IDC_CMB_POSITION), CB_INSERTSTRING,  4, (LPARAM)"Custom");
	SendMessage(GetDlgItem(m_Dlg, IDC_CMB_POSITION), CB_SETMINVISIBLE, 5, 0);

	long lResult = SendMessage(			// returns LRESULT in lResult
		GetDlgItem(m_Dlg, IDC_SPIN1),	// handle to destination control
		(UINT) UDM_SETRANGE,			// message ID
		(WPARAM) 0,						// = 0; not used, must be zero
		(LPARAM) MAKELONG ( upper, lower)      // = (LPARAM) MAKELONG ((short) nUpper, (short) nLower)
		);
	lResult = SendMessage(			// returns LRESULT in lResult
		GetDlgItem(m_Dlg, IDC_SPIN2),	// handle to destination control
		(UINT) UDM_SETRANGE,			// message ID
		(WPARAM) 0,						// = 0; not used, must be zero
		(LPARAM) MAKELONG ( upper, lower)      // = (LPARAM) MAKELONG ((short) nUpper, (short) nLower)
		);
	lResult = SendMessage(			// returns LRESULT in lResult
		GetDlgItem(m_Dlg, IDC_SPIN3),	// handle to destination control
		(UINT) UDM_SETRANGE,			// message ID
		(WPARAM) 0,						// = 0; not used, must be zero
		(LPARAM) MAKELONG ( upper, lower)      // = (LPARAM) MAKELONG ((short) nUpper, (short) nLower)
		);
	lResult = SendMessage(			// returns LRESULT in lResult
		GetDlgItem(m_Dlg, IDC_SPIN4),	// handle to destination control
		(UINT) UDM_SETRANGE,			// message ID
		(WPARAM) 0,						// = 0; not used, must be zero
		(LPARAM) MAKELONG ( upper, lower)      // = (LPARAM) MAKELONG ((short) nUpper, (short) nLower)
		);
	lResult = SendMessage(			// returns LRESULT in lResult
		GetDlgItem(m_Dlg, IDC_SPIN5),	// handle to destination control
		(UINT) UDM_SETRANGE,			// message ID
		(WPARAM) 0,						// = 0; not used, must be zero
		(LPARAM) MAKELONG ( upper, lower)      // = (LPARAM) MAKELONG ((short) nUpper, (short) nLower)
		);
	lResult = SendMessage(			// returns LRESULT in lResult
		GetDlgItem(m_Dlg, IDC_SPIN6),	// handle to destination control
		(UINT) UDM_SETRANGE,			// message ID
		(WPARAM) 0,						// = 0; not used, must be zero
		(LPARAM) MAKELONG ( upper, lower)      // = (LPARAM) MAKELONG ((short) nUpper, (short) nLower)
		);
}
