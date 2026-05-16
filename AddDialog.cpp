#include "AddDialog.h"

AddDialog::AddDialog(wxWindow* parent, const std::vector<std::string>& colNames)
    : wxDialog(parent, wxID_ANY, _("Добавить запись"),
        wxDefaultPosition, wxSize(320, 80 + 40 * (int)colNames.size()))
{
    auto* vbox = new wxBoxSizer(wxVERTICAL);

    for (const auto& name : colNames) {
        auto* hb = new wxBoxSizer(wxHORIZONTAL);
        hb->Add(new wxStaticText(this, wxID_ANY, wxString(name) + ":"),
            0, wxALIGN_CENTER | wxRIGHT, 8);
        auto* inp = new wxTextCtrl(this, wxID_ANY);
        inputs_.push_back(inp);
        hb->Add(inp, 1, wxEXPAND);
        vbox->Add(hb, 0, wxEXPAND | wxALL, 6);
    }

    auto* btns = new wxBoxSizer(wxHORIZONTAL);
    btns->Add(new wxButton(this, wxID_OK, _("Добавить")), 0, wxRIGHT, 6);
    btns->Add(new wxButton(this, wxID_CANCEL, _("Отмена")));
    vbox->Add(btns, 0, wxALIGN_RIGHT | wxALL, 8);

    SetSizer(vbox);
    Fit();
}

std::vector<std::string> AddDialog::GetValues() const {
    std::vector<std::string> vals;
    for (auto* inp : inputs_)
        vals.push_back(inp->GetValue().ToStdString());
    return vals;
}
