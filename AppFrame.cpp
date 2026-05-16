#include "AppFrame.h"

AppFrame::AppFrame(wxWindow* parent, const wxString& title,
    const wxPoint& pos, const wxSize& size)
    : wxFrame(parent, wxID_ANY, title, pos, size) {
}

void AppFrame::ShowError(const std::string& msg) {
    wxMessageBox(msg, "Ошибка", wxOK | wxICON_ERROR);
}