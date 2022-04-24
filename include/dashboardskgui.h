///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version 3.10.1-0-g8feb16b)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/artprov.h>
#include <wx/bitmap.h>
#include <wx/bmpbuttn.h>
#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/colour.h>
#include <wx/dialog.h>
#include <wx/font.h>
#include <wx/gdicmn.h>
#include <wx/icon.h>
#include <wx/image.h>
#include <wx/intl.h>
#include <wx/listbox.h>
#include <wx/panel.h>
#include <wx/scrolwin.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>
#include <wx/stc/stc.h>
#include <wx/string.h>
#include <wx/textctrl.h>
#include <wx/treectrl.h>
#include <wx/xrc/xmlres.h>

///////////////////////////////////////////////////////////////////////////

namespace DashboardSKPlugin {

///////////////////////////////////////////////////////////////////////////////
/// Class MainConfigFrame
///////////////////////////////////////////////////////////////////////////////
class MainConfigFrame : public wxDialog {
private:
protected:
    wxStaticText* m_stSelf;
    wxTextCtrl* m_tSelf;
    wxButton* m_btnSignalK;
    wxStaticText* m_stDashboard;
    wxChoice* m_comboDashboard;
    wxButton* m_btnRemoveDashboard;
    wxButton* m_btnNewDashboard;
    wxPanel* m_panelList;
    wxScrolledWindow* m_scrolledWindowInstrumentList;
    wxStaticText* m_stInstruments;
    wxListBox* m_lbInstruments;
    wxBitmapButton* m_bpAddButton;
    wxBitmapButton* m_bpRemoveButton;
    wxBitmapButton* m_bpMoveUpButton;
    wxBitmapButton* m_bpMoveDownButton;
    wxBitmapButton* m_bpSaveInstrButton;
    wxBitmapButton* m_bpImportInstrButton;
    wxPanel* m_panelConfig;
    wxScrolledWindow* m_swConfig;
    wxStaticText* m_stName;
    wxTextCtrl* m_tName;
    wxStaticText* m_stTitle;
    wxTextCtrl* m_tTitle;
    wxStaticText* m_stInstrument;
    wxStaticText* m_stTimeout;
    wxSpinCtrl* m_spTimeout;
    wxStaticText* m_stSeconds;
    wxFlexGridSizer* SettingsItemSizer;
    wxCheckBox* m_cbEnabled;
    wxStaticText* m_stCanvas;
    wxSpinCtrl* m_spCanvas;
    wxStaticText* m_stAnchor;
    wxChoice* m_chAnchor;
    wxStaticText* m_stOffsetX;
    wxSpinCtrl* m_spOffsetX;
    wxStaticText* m_stOffsetY;
    wxSpinCtrl* m_spOffsetY;
    wxStaticText* m_stSpacingH;
    wxSpinCtrl* m_spSpacingH;
    wxStaticText* m_stSpacingV;
    wxSpinCtrl* m_spSpacingV;
    wxStdDialogButtonSizer* m_sdbSizer;
    wxButton* m_sdbSizerOK;
    wxButton* m_sdbSizerCancel;

    // Virtual event handlers, override them in your derived class
    virtual void m_btnSignalKOnButtonClick(wxCommandEvent& event)
    {
        event.Skip();
    }
    virtual void m_comboDashboardOnChoice(wxCommandEvent& event)
    {
        event.Skip();
    }
    virtual void m_btnRemoveDashboardOnButtonClick(wxCommandEvent& event)
    {
        event.Skip();
    }
    virtual void m_btnNewDashboardOnButtonClick(wxCommandEvent& event)
    {
        event.Skip();
    }
    virtual void m_lbInstrumentsOnListBox(wxCommandEvent& event)
    {
        event.Skip();
    }
    virtual void m_bpAddButtonOnButtonClick(wxCommandEvent& event)
    {
        event.Skip();
    }
    virtual void m_bpRemoveButtonOnButtonClick(wxCommandEvent& event)
    {
        event.Skip();
    }
    virtual void m_bpMoveUpButtonOnButtonClick(wxCommandEvent& event)
    {
        event.Skip();
    }
    virtual void m_bpMoveDownButtonOnButtonClick(wxCommandEvent& event)
    {
        event.Skip();
    }
    virtual void m_bpSaveInstrButtonOnButtonClick(wxCommandEvent& event)
    {
        event.Skip();
    }
    virtual void m_bpImportInstrButtonOnButtonClick(wxCommandEvent& event)
    {
        event.Skip();
    }
    virtual void m_cbEnabledOnCheckBox(wxCommandEvent& event) { event.Skip(); }
    virtual void m_spCanvasOnSpinCtrl(wxSpinEvent& event) { event.Skip(); }
    virtual void m_chAnchorOnChoice(wxCommandEvent& event) { event.Skip(); }
    virtual void m_spOffsetXOnSpinCtrl(wxSpinEvent& event) { event.Skip(); }
    virtual void m_spOffsetYOnSpinCtrl(wxSpinEvent& event) { event.Skip(); }
    virtual void m_spSpacingHOnSpinCtrl(wxSpinEvent& event) { event.Skip(); }
    virtual void m_spSpacingVOnSpinCtrl(wxSpinEvent& event) { event.Skip(); }
    virtual void m_sdbSizerOnCancelButtonClick(wxCommandEvent& event)
    {
        event.Skip();
    }
    virtual void m_sdbSizerOnOKButtonClick(wxCommandEvent& event)
    {
        event.Skip();
    }

public:
    MainConfigFrame(wxWindow* parent, wxWindowID id = wxID_ANY,
        const wxString& title = _("DashboardSK Configuration"),
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxSize(640, 510),
        long style = wxDEFAULT_DIALOG_STYLE | wxTAB_TRAVERSAL);

    ~MainConfigFrame();
};

///////////////////////////////////////////////////////////////////////////////
/// Class SKDataTree
///////////////////////////////////////////////////////////////////////////////
class SKDataTree : public wxDialog {
private:
protected:
    wxStyledTextCtrl* m_scintillaCode;
    wxStdDialogButtonSizer* m_sdbSizerBtns;
    wxButton* m_sdbSizerBtnsOK;
    wxButton* m_sdbSizerBtnsCancel;

    // Virtual event handlers, override them in your derived class
    virtual void m_sdbSizerBtnsOnCancelButtonClick(wxCommandEvent& event)
    {
        event.Skip();
    }
    virtual void m_sdbSizerBtnsOnOKButtonClick(wxCommandEvent& event)
    {
        event.Skip();
    }

public:
    SKDataTree(wxWindow* parent, wxWindowID id = wxID_ANY,
        const wxString& title = _("SignalK data tree"),
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxSize(500, 450),
        long style = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);

    ~SKDataTree();
};

///////////////////////////////////////////////////////////////////////////////
/// Class SKKeyCtrl
///////////////////////////////////////////////////////////////////////////////
class SKKeyCtrl : public wxPanel {
private:
protected:
    wxTextCtrl* m_tSKKey;
    wxButton* m_btnSelect;

    // Virtual event handlers, override them in your derived class
    virtual void m_btnSelectOnButtonClick(wxCommandEvent& event)
    {
        event.Skip();
    }

public:
    SKKeyCtrl(wxWindow* parent, wxWindowID id = wxID_ANY,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxSize(200, 32), long style = wxTAB_TRAVERSAL,
        const wxString& name = wxEmptyString);

    ~SKKeyCtrl();
};

///////////////////////////////////////////////////////////////////////////////
/// Class SKPathBrowser
///////////////////////////////////////////////////////////////////////////////
class SKPathBrowser : public wxDialog {
private:
protected:
    wxTreeCtrl* m_treePaths;
    wxStdDialogButtonSizer* m_sdbSizerButtons;
    wxButton* m_sdbSizerButtonsOK;
    wxButton* m_sdbSizerButtonsCancel;

    // Virtual event handlers, override them in your derived class
    virtual void m_treeCtrl1OnTreeSelChanged(wxTreeEvent& event)
    {
        event.Skip();
    }
    virtual void m_sdbSizerButtonsOnCancelButtonClick(wxCommandEvent& event)
    {
        event.Skip();
    }
    virtual void m_sdbSizerButtonsOnOKButtonClick(wxCommandEvent& event)
    {
        event.Skip();
    }

public:
    SKPathBrowser(wxWindow* parent, wxWindowID id = wxID_ANY,
        const wxString& title = _("SignalK Browser"),
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxSize(500, 300),
        long style = wxDEFAULT_FRAME_STYLE | wxTAB_TRAVERSAL);

    ~SKPathBrowser();
};

} // namespace DashboardSKPlugin