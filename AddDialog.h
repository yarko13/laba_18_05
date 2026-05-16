
#pragma once
#include <wx/wx.h>
#include <vector>
#include <string>

class AddDialog : public wxDialog {
public:
    AddDialog(wxWindow* parent, const std::vector<std::string>& colNames);
    std::vector<std::string> GetValues() const;

private:
    std::vector<wxTextCtrl*> inputs_;
};
