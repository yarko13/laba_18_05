#pragma once
#include "AppFrame.h"
#include "Database.h"
#include <memory>
#include <wx/listctrl.h>
#include <wx/statusbr.h>

class MainFrame : public AppFrame {
public:
    explicit MainFrame(std::shared_ptr<Database> db);
    void RefreshView() override;

    // Экспорт
    void OnExportCSV(wxCommandEvent&);
    void OnExportJSON(wxCommandEvent&);
    void ExportToCSV(const std::string& filename);
    void ExportToJSON(const std::string& filename);

    // Поиск
    void OnSearch(wxCommandEvent&);
    void HighlightSearchResults(const std::string& query);

    // Статистика
    void OnStatistics(wxCommandEvent&);
    void ShowStatistics();

private:
    std::shared_ptr<Database> db_;
    wxListCtrl* listCtrl_;
    wxChoice* tableChoice_;
    wxTextCtrl* searchCtrl_;
    std::string currentTable_;
    std::string lastSearchQuery_;

    void OnCreateTable(wxCommandEvent&);
    void OnAdd(wxCommandEvent&);
    void OnEdit(wxCommandEvent&);
    void OnDelete(wxCommandEvent&);
    void OnDropTable(wxCommandEvent&);      // НОВЫЙ МЕТОД
    void OnTableSelect(wxCommandEvent&);
    void PopulateList();
    void CreateMenuBar();

    wxDECLARE_EVENT_TABLE();
};