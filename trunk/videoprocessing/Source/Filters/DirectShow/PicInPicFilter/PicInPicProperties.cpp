#include "PicInPicProperties.h"

CUnknown * WINAPI PicInPicProperties::CreateInstance(LPUNKNOWN pUnk, HRESULT *pHr)
{
  PicInPicProperties *pNewObject = new PicInPicProperties(pUnk);
  if (pNewObject == NULL)
  {
    *pHr = E_OUTOFMEMORY;
  }
  return pNewObject;
}

PicInPicProperties::PicInPicProperties(IUnknown *pUnk) :
FilterPropertiesBase(NAME("Pic in pic Properties"), pUnk, IDD_PIC_IN_PIC_DIALOG, IDS_PIC_IN_PIC_CAPTION)
{

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

  hr = setEditTextFromIntFilterParameter(BORDER_WIDTH, IDC_EDIT_BORDER_WIDTH);
  return hr;
}

HRESULT PicInPicProperties::OnApplyChanges(void)
{
  int nLength = 0;
  char szBuffer[BUFFER_SIZE];
  HRESULT hr;

  int nValue;
  // Target dimensions
  nLength = GetDlgItemText(m_Dlg, IDC_EDIT_TARGET_WIDTH, szBuffer, BUFFER_SIZE);
  nValue = atoi(szBuffer);
  if (m_targetWidth != nValue)
  {
    // value has changed
    hr = m_pSettingsInterface->SetParameter(TARGET_WIDTH, szBuffer);
    if (FAILED(hr))
    {
      return hr;
    }
    m_targetWidth = nValue;
  }
  nLength = GetDlgItemText(m_Dlg, IDC_EDIT_TARGET_HEIGHT, szBuffer, BUFFER_SIZE);
  nValue = atoi(szBuffer);
  if (m_targetHeight != nValue)
  {
    // value has changed
    hr = m_pSettingsInterface->SetParameter(TARGET_HEIGHT, szBuffer);
    if (FAILED(hr))
    {
      return hr;
    }
    m_targetHeight = nValue;
  }

  // Subpicture dimensions
  nLength = GetDlgItemText(m_Dlg, IDC_EDIT_SUBPICTURE_WIDTH, szBuffer, BUFFER_SIZE);
  nValue = atoi(szBuffer);
  if (m_uiSubpicWidth != nValue)
  {
    // value has changed
    hr = m_pSettingsInterface->SetParameter(SUB_PIC_WIDTH, szBuffer);
    if (FAILED(hr))
    {
      return hr;
    }
    m_uiSubpicWidth = nValue;
  }
  nLength = GetDlgItemText(m_Dlg, IDC_EDIT_SUBPICTURE_HEIGHT, szBuffer, BUFFER_SIZE);
  nValue = atoi(szBuffer);
  if (m_uiSubpicHeight != nValue)
  {
    // value has changed
    hr = m_pSettingsInterface->SetParameter(SUB_PIC_HEIGHT, szBuffer);
    if (FAILED(hr))
    {
      return hr;
    }
    m_uiSubpicHeight = nValue;
  }

  // Use custom offset
  nLength = GetDlgItemText(m_Dlg, IDC_EDIT_SUBPICTURE_OFFSET_X, szBuffer, BUFFER_SIZE);
  nValue = atoi(szBuffer);
  if (m_uiOffsetX != nValue)
  {
    hr = m_pSettingsInterface->SetParameter(OFFSET_X, szBuffer);
    if (FAILED(hr))
    {
      return hr;
    }
    m_uiOffsetX = nValue;
  }
  nLength = GetDlgItemText(m_Dlg, IDC_EDIT_SUBPICTURE_OFFSET_Y, szBuffer, BUFFER_SIZE);
  nValue = atoi(szBuffer);
  if (m_uiOffsetY != nValue)
  {
    hr = m_pSettingsInterface->SetParameter(OFFSET_Y, szBuffer);
    if (FAILED(hr))
    {
      return hr;
    }
    m_uiOffsetY = nValue;
  }

  hr = setIntFilterParameterFromEditText(BORDER_WIDTH, IDC_EDIT_BORDER_WIDTH);

  return hr;
}

void PicInPicProperties::initialiseControls()
{
  InitCommonControls();

  short lower = 0;
  short upper = (short)USHRT_MAX;
  setSpinBoxRange(IDC_SPIN1, lower, upper);
  setSpinBoxRange(IDC_SPIN2, lower, upper);
  setSpinBoxRange(IDC_SPIN3, lower, upper);
  setSpinBoxRange(IDC_SPIN4, lower, upper);
  setSpinBoxRange(IDC_SPIN5, lower, upper);
  setSpinBoxRange(IDC_SPIN6, lower, upper);
  setSpinBoxRange(IDC_SPIN7, lower, upper);
}
