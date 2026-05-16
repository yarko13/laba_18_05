#pragma once
#include <wx/wx.h>


class AppFrame : public wxFrame {
public:
    AppFrame(wxWindow* parent, const wxString& title,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize);

    virtual void RefreshView() = 0;

    virtual ~AppFrame() = default;

protected:
    void ShowError(const std::string& msg);
};