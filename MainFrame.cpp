#include "MainFrame.h"
#include "AddDialog.h"
#include <sstream>
#include <fstream>
#include <wx/choicdlg.h>
#include <wx/menu.h>
#include <wx/msgdlg.h>
#include <map>

enum {
    ID_CREATE = 100,
    ID_ADD,
    ID_EDIT,
    ID_DELETE,
    ID_DROP_TABLE,      // НОВАЯ КОНСТАНТА
    ID_TABLE_SELECT,
    ID_EXPORT_CSV,
    ID_EXPORT_JSON,
    ID_STATISTICS
};

wxBEGIN_EVENT_TABLE(MainFrame, AppFrame)
EVT_BUTTON(ID_CREATE, MainFrame::OnCreateTable)
EVT_BUTTON(ID_ADD, MainFrame::OnAdd)
EVT_BUTTON(ID_EDIT, MainFrame::OnEdit)
EVT_BUTTON(ID_DELETE, MainFrame::OnDelete)
EVT_BUTTON(ID_DROP_TABLE, MainFrame::OnDropTable)  // НОВАЯ СТРОКА
EVT_CHOICE(ID_TABLE_SELECT, MainFrame::OnTableSelect)
EVT_TEXT(wxID_ANY, MainFrame::OnSearch)
EVT_MENU(ID_EXPORT_CSV, MainFrame::OnExportCSV)
EVT_MENU(ID_EXPORT_JSON, MainFrame::OnExportJSON)
EVT_MENU(ID_STATISTICS, MainFrame::OnStatistics)
wxEND_EVENT_TABLE()

MainFrame::MainFrame(std::shared_ptr<Database> db)
    : AppFrame(nullptr, "DB Manager", wxDefaultPosition, wxSize(900, 600))
    , db_(db)
{
    auto* panel = new wxPanel(this);
    auto* mainSizer = new wxBoxSizer(wxVERTICAL);
    auto* hbox = new wxBoxSizer(wxHORIZONTAL);

    // Выбор таблицы
    tableChoice_ = new wxChoice(panel, ID_TABLE_SELECT);
    hbox->Add(new wxStaticText(panel, wxID_ANY, "Таблица:"), 0, wxALIGN_CENTER | wxRIGHT, 6);
    hbox->Add(tableChoice_, 1, wxEXPAND);

    // Кнопки
    hbox->Add(new wxButton(panel, ID_CREATE, "Создать"), 0, wxLEFT, 4);
    hbox->Add(new wxButton(panel, ID_ADD, "Добавить"), 0, wxLEFT, 4);
    hbox->Add(new wxButton(panel, ID_EDIT, "Редакт."), 0, wxLEFT, 4);
    hbox->Add(new wxButton(panel, ID_DELETE, "Удалить запись"), 0, wxLEFT, 4);
    hbox->Add(new wxButton(panel, ID_DROP_TABLE, "Удалить таблицу"), 0, wxLEFT, 4);  // НОВАЯ КНОПКА

    mainSizer->Add(hbox, 0, wxEXPAND | wxALL, 8);

    // Поле поиска
    auto* searchBox = new wxBoxSizer(wxHORIZONTAL);
    searchBox->Add(new wxStaticText(panel, wxID_ANY, "Поиск:"), 0, wxALIGN_CENTER | wxRIGHT, 6);
    searchCtrl_ = new wxTextCtrl(panel, wxID_ANY, "", wxDefaultPosition, wxSize(250, -1));
    searchCtrl_->SetHint("Введите текст для поиска...");
    searchBox->Add(searchCtrl_, 1, wxEXPAND);
    mainSizer->Add(searchBox, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 8);

    // Таблица
    listCtrl_ = new wxListCtrl(panel, wxID_ANY, wxDefaultPosition,
        wxDefaultSize, wxLC_REPORT | wxBORDER_SUNKEN);
    mainSizer->Add(listCtrl_, 1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 8);

    panel->SetSizer(mainSizer);

    // Меню
    CreateMenuBar();

    // Статус-бар
    wxStatusBar* statusBar = CreateStatusBar(2);
    statusBar->SetStatusText("Готов к работе", 0);
    statusBar->SetStatusText("", 1);
    int widths[2] = { -2, -1 };
    statusBar->SetStatusWidths(2, widths);

    // Горячие клавиши
    wxAcceleratorEntry entries[7];  // БЫЛО 6, СТАЛО 7
    entries[0].Set(wxACCEL_CTRL, 'N', ID_CREATE);
    entries[1].Set(wxACCEL_CTRL, 'A', ID_ADD);
    entries[2].Set(wxACCEL_CTRL, 'E', ID_EDIT);
    entries[3].Set(wxACCEL_CTRL, 'D', ID_DELETE);
    entries[4].Set(wxACCEL_CTRL, 'F', wxID_FIND);
    entries[5].Set(wxACCEL_CTRL, 'S', ID_STATISTICS);
    entries[6].Set(wxACCEL_CTRL | wxACCEL_SHIFT, 'D', ID_DROP_TABLE);  // НОВАЯ ГОРЯЧАЯ КЛАВИША (Ctrl+Shift+D)

    wxAcceleratorTable accel(7, entries);
    SetAcceleratorTable(accel);

    RefreshView();
}

void MainFrame::CreateMenuBar() {
    wxMenuBar* menuBar = new wxMenuBar();

    // Меню Файл
    wxMenu* fileMenu = new wxMenu();
    fileMenu->Append(ID_EXPORT_CSV, "Экспорт в CSV\tCtrl+Shift+C");
    fileMenu->Append(ID_EXPORT_JSON, "Экспорт в JSON\tCtrl+Shift+J");
    fileMenu->AppendSeparator();
    fileMenu->Append(wxID_EXIT, "Выход\tAlt+F4");

    // Меню Инструменты
    wxMenu* toolsMenu = new wxMenu();
    toolsMenu->Append(ID_STATISTICS, "Статистика\tCtrl+S");
    toolsMenu->AppendSeparator();
    toolsMenu->Append(ID_DROP_TABLE, "Удалить таблицу\tCtrl+Shift+D");

    // Меню Справка
    wxMenu* helpMenu = new wxMenu();
    helpMenu->Append(wxID_ABOUT, "О программе");

    menuBar->Append(fileMenu, "&Файл");
    menuBar->Append(toolsMenu, "&Инструменты");
    menuBar->Append(helpMenu, "&Справка");

    SetMenuBar(menuBar);

    // Обработчики
    Bind(wxEVT_MENU, [this](wxCommandEvent&) { Close(true); }, wxID_EXIT);
    Bind(wxEVT_MENU, [this](wxCommandEvent&) {
        wxMessageBox("DB Manager\nРазработано с использованием wxWidgets и SQLite\n"
            "Функции:\n- Управление таблицами\n- Экспорт в CSV/JSON\n- Поиск с подсветкой\n- Горячие клавиши\n- Удаление таблиц",
            "О программе", wxOK | wxICON_INFORMATION);
        }, wxID_ABOUT);
}

void MainFrame::RefreshView() {
    GetStatusBar()->SetStatusText("Обновление...", 1);
    tableChoice_->Clear();
    try {
        for (auto& t : db_->getTableNames())
            tableChoice_->Append(t);
    }
    catch (const DbException& e) { ShowError(e.what()); }

    if (tableChoice_->GetCount() > 0) {
        tableChoice_->SetSelection(0);
        currentTable_ = tableChoice_->GetString(0).ToStdString();
        PopulateList();
    }
    GetStatusBar()->SetStatusText("Готово", 1);
}

void MainFrame::PopulateList() {
    listCtrl_->ClearAll();
    if (currentTable_.empty()) return;

    try {
        auto columns = db_->getColumns(currentTable_);
        auto records = db_->getRecords(currentTable_);

        if (columns.empty()) {
            listCtrl_->InsertColumn(0, "Нет данных", wxLIST_FORMAT_LEFT, 200);
            return;
        }

        // Заголовки
        listCtrl_->InsertColumn(0, "ID", wxLIST_FORMAT_LEFT, 50);
        for (size_t i = 0; i < columns.size(); ++i) {
            listCtrl_->InsertColumn(i + 1, columns[i], wxLIST_FORMAT_LEFT, 150);
        }

        // Данные
        for (size_t row = 0; row < records.size(); ++row) {
            long idx = listCtrl_->InsertItem(listCtrl_->GetItemCount(),
                wxString::Format("%d", records[row].id));
            for (size_t col = 0; col < records[row].fields.size(); ++col) {
                if (col < columns.size()) {
                    listCtrl_->SetItem(idx, col + 1, records[row].fields[col]);
                }
            }
        }

        // Применяем поиск, если есть
        if (!lastSearchQuery_.empty()) {
            HighlightSearchResults(lastSearchQuery_);
        }

        GetStatusBar()->SetStatusText(
            wxString::Format("Записей: %d", (int)records.size()), 0);
    }
    catch (const DbException& e) {
        ShowError(e.what());
        listCtrl_->ClearAll();
        listCtrl_->InsertColumn(0, "Ошибка загрузки", wxLIST_FORMAT_LEFT, 200);
    }
}

// ==================== ПОИСК ====================
void MainFrame::OnSearch(wxCommandEvent& event) {
    lastSearchQuery_ = event.GetString().ToStdString();
    HighlightSearchResults(lastSearchQuery_);
}

void MainFrame::HighlightSearchResults(const std::string& query) {
    if (query.empty()) {
        // Сброс подсветки
        for (int i = 0; i < listCtrl_->GetItemCount(); ++i) {
            listCtrl_->SetItemBackgroundColour(i, wxNullColour);
            listCtrl_->SetItemTextColour(i, wxNullColour);
        }
        GetStatusBar()->SetStatusText("", 1);
        return;
    }

    int foundCount = 0;
    wxString searchLower = wxString(query).Lower();

    for (int i = 0; i < listCtrl_->GetItemCount(); ++i) {
        bool found = false;

        // Поиск в ID
        if (listCtrl_->GetItemText(i).Lower().Contains(searchLower)) {
            found = true;
        }

        // Поиск в остальных колонках
        for (int col = 1; col < listCtrl_->GetColumnCount(); ++col) {
            wxListItem item;
            item.SetId(i);
            item.SetColumn(col);
            item.SetMask(wxLIST_MASK_TEXT);
            listCtrl_->GetItem(item);

            if (item.GetText().Lower().Contains(searchLower)) {
                found = true;
                break;
            }
        }

        // Подсветка
        if (found) {
            listCtrl_->SetItemBackgroundColour(i, wxColour(255, 255, 150));
            listCtrl_->SetItemTextColour(i, wxColour(0, 0, 0));
            foundCount++;
        }
        else {
            listCtrl_->SetItemBackgroundColour(i, wxNullColour);
            listCtrl_->SetItemTextColour(i, wxNullColour);
        }
    }

    GetStatusBar()->SetStatusText(
        wxString::Format("Найдено: %d", foundCount), 1);
}

// ==================== ЭКСПОРТ ====================
void MainFrame::OnExportCSV(wxCommandEvent&) {
    if (currentTable_.empty()) {
        ShowError("Не выбрана таблица");
        return;
    }

    wxFileDialog dlg(this, "Сохранить как CSV", "", currentTable_ + ".csv",
        "CSV files (*.csv)|*.csv", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (dlg.ShowModal() == wxID_OK) {
        GetStatusBar()->SetStatusText("Экспорт в CSV...", 1);
        ExportToCSV(dlg.GetPath().ToStdString());
        GetStatusBar()->SetStatusText("Экспорт завершён", 1);
    }
}

void MainFrame::OnExportJSON(wxCommandEvent&) {
    if (currentTable_.empty()) {
        ShowError("Не выбрана таблица");
        return;
    }

    wxFileDialog dlg(this, "Сохранить как JSON", "", currentTable_ + ".json",
        "JSON files (*.json)|*.json", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (dlg.ShowModal() == wxID_OK) {
        GetStatusBar()->SetStatusText("Экспорт в JSON...", 1);
        ExportToJSON(dlg.GetPath().ToStdString());
        GetStatusBar()->SetStatusText("Экспорт завершён", 1);
    }
}

void MainFrame::ExportToCSV(const std::string& filename) {
    try {
        auto columns = db_->getColumns(currentTable_);
        auto records = db_->getRecords(currentTable_);

        std::ofstream file(filename);
        if (!file.is_open()) {
            throw std::runtime_error("Не удалось создать файл");
        }

        // Заголовки
        file << "ID";
        for (const auto& col : columns) {
            file << "," << col;
        }
        file << "\n";

        // Данные
        for (const auto& rec : records) {
            file << rec.id;
            for (const auto& field : rec.fields) {
                std::string escaped = field;
                size_t pos = 0;
                while ((pos = escaped.find('"', pos)) != std::string::npos) {
                    escaped.replace(pos, 1, "\"\"");
                    pos += 2;
                }
                file << ",\"" << escaped << "\"";
            }
            file << "\n";
        }

        wxMessageBox(wxString::Format("Экспортировано %d записей в CSV", (int)records.size()),
            "Успех", wxOK | wxICON_INFORMATION);
    }
    catch (const std::exception& e) {
        ShowError(e.what());
    }
}

void MainFrame::ExportToJSON(const std::string& filename) {
    try {
        auto columns = db_->getColumns(currentTable_);
        auto records = db_->getRecords(currentTable_);

        std::ofstream file(filename);
        if (!file.is_open()) {
            throw std::runtime_error("Не удалось создать файл");
        }

        file << "[\n";
        for (size_t i = 0; i < records.size(); ++i) {
            file << "  {\n";
            file << "    \"id\": " << records[i].id << ",\n";
            for (size_t j = 0; j < columns.size(); ++j) {
                file << "    \"" << columns[j] << "\": \"" << records[i].fields[j] << "\"";
                if (j < columns.size() - 1) file << ",";
                file << "\n";
            }
            file << "  }";
            if (i < records.size() - 1) file << ",";
            file << "\n";
        }
        file << "]\n";

        wxMessageBox(wxString::Format("Экспортировано %d записей в JSON", (int)records.size()),
            "Успех", wxOK | wxICON_INFORMATION);
    }
    catch (const std::exception& e) {
        ShowError(e.what());
    }
}

// ==================== СТАТИСТИКА ====================
void MainFrame::OnStatistics(wxCommandEvent&) {
    ShowStatistics();
}

void MainFrame::ShowStatistics() {
    if (currentTable_.empty()) {
        ShowError("Не выбрана таблица");
        return;
    }

    try {
        auto columns = db_->getColumns(currentTable_);
        auto records = db_->getRecords(currentTable_);

        if (records.empty()) {
            wxMessageBox("Нет данных для статистики", "Статистика",
                wxOK | wxICON_INFORMATION);
            return;
        }

        wxString stats;
        stats += wxString::Format("=== СТАТИСТИКА ТАБЛИЦЫ '%s' ===\n\n", currentTable_);
        stats += wxString::Format("Всего записей: %d\n", (int)records.size());
        stats += wxString::Format("Количество колонок: %d\n", (int)columns.size());
        stats += "\nКолонки:\n";
        for (const auto& col : columns) {
            stats += wxString::Format("  • %s\n", col);
        }

        // Статистика по первой колонке (если есть)
        if (!columns.empty() && !records.empty() && !records[0].fields.empty()) {
            std::map<std::string, int> frequency;
            for (const auto& rec : records) {
                if (!rec.fields.empty()) {
                    frequency[rec.fields[0]]++;
                }
            }

            stats += "\nЧастота значений в колонке '" + columns[0] + "':\n";
            for (std::map<std::string, int>::const_iterator it = frequency.begin(); it != frequency.end(); ++it) {
                double percent = (double)it->second / records.size() * 100;
                stats += wxString::Format("  • %s: %d (%.1f%%)\n",
                    it->first, it->second, percent);
            }
        }

        wxMessageBox(stats, "Статистика", wxOK | wxICON_INFORMATION);
    }
    catch (const DbException& e) {
        ShowError(e.what());
    }
}

// ==================== УДАЛЕНИЕ ТАБЛИЦЫ ====================
void MainFrame::OnDropTable(wxCommandEvent&) {
    if (currentTable_.empty()) {
        ShowError("Не выбрана таблица");
        return;
    }

    // Защита от удаления системных таблиц
    if (currentTable_.find("sqlite_") == 0) {
        ShowError("Нельзя удалить системную таблицу SQLite");
        return;
    }

    wxString msg = wxString::Format("Вы уверены, что хотите удалить таблицу '%s'?\n\n"
        "ВСЕ ДАННЫЕ В ТАБЛИЦЕ БУДУТ УТЕРЯНЫ!\n"
        "Это действие нельзя отменить.",
        currentTable_);

    int answer = wxMessageBox(msg, "Подтверждение удаления таблицы",
        wxYES_NO | wxICON_WARNING);

    if (answer == wxYES) {
        try {
            GetStatusBar()->SetStatusText("Удаление таблицы...", 1);
            db_->dropTable(currentTable_);
            GetStatusBar()->SetStatusText("Таблица удалена", 1);

            // Обновляем список таблиц
            RefreshView();
        }
        catch (const DbException& e) {
            ShowError(e.what());
            GetStatusBar()->SetStatusText("Ошибка удаления", 1);
        }
    }
}

// ==================== ОСТАЛЬНЫЕ МЕТОДЫ ====================
void MainFrame::OnCreateTable(wxCommandEvent&) {
    wxTextEntryDialog dlg(this, "Имя таблицы:", "Создать таблицу");
    if (dlg.ShowModal() != wxID_OK) return;

    wxTextEntryDialog dlg2(this, "Колонки через запятую (напр. name,age):", "Колонки");
    if (dlg2.ShowModal() != wxID_OK) return;

    std::vector<std::string> cols;
    std::istringstream ss(dlg2.GetValue().ToStdString());
    std::string tok;
    while (std::getline(ss, tok, ',')) {
        if (!tok.empty()) cols.push_back(tok);
    }

    try {
        db_->createTable(dlg.GetValue().ToStdString(), cols);
        RefreshView();
        GetStatusBar()->SetStatusText("Таблица создана", 1);
    }
    catch (const DbException& e) { ShowError(e.what()); }
}

void MainFrame::OnAdd(wxCommandEvent&) {
    if (currentTable_.empty()) return;

    try {
        auto cols = db_->getColumns(currentTable_);
        if (cols.empty()) {
            ShowError("У таблицы нет колонок");
            return;
        }
        auto dlg = std::make_unique<AddDialog>(this, cols);
        if (dlg->ShowModal() == wxID_OK) {
            db_->insertRecord(currentTable_, dlg->GetValues());
            PopulateList();
            GetStatusBar()->SetStatusText("Запись добавлена", 1);
        }
    }
    catch (const DbException& e) { ShowError(e.what()); }
}

void MainFrame::OnEdit(wxCommandEvent&) {
    long sel = listCtrl_->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    if (sel == -1) {
        wxMessageBox("Выберите запись", "Ошибка", wxOK | wxICON_WARNING);
        return;
    }

    int id = std::stoi(listCtrl_->GetItemText(sel).ToStdString());
    auto columns = db_->getColumns(currentTable_);
    if (columns.empty()) return;

    wxArrayString choices;
    for (const auto& col : columns) {
        choices.Add(col);
    }

    wxSingleChoiceDialog dlg(this, "Выберите колонку для редактирования:",
        "Редактировать", choices);
    if (dlg.ShowModal() != wxID_OK) return;

    std::string selectedCol = dlg.GetStringSelection().ToStdString();
    std::string oldValue = listCtrl_->GetItemText(sel, dlg.GetSelection() + 1).ToStdString();

    wxTextEntryDialog dlg2(this, "Новое значение для " + selectedCol + ":",
        "Редактировать", oldValue);
    if (dlg2.ShowModal() == wxID_OK) {
        try {
            db_->updateRecord(currentTable_, id, selectedCol, dlg2.GetValue().ToStdString());
            PopulateList();
            GetStatusBar()->SetStatusText("Запись обновлена", 1);
        }
        catch (const DbException& e) { ShowError(e.what()); }
    }
}

void MainFrame::OnDelete(wxCommandEvent&) {
    long sel = listCtrl_->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    if (sel == -1) {
        wxMessageBox("Выберите запись", "Ошибка", wxOK | wxICON_WARNING);
        return;
    }

    int id = std::stoi(listCtrl_->GetItemText(sel).ToStdString());
    if (wxMessageBox("Удалить запись?", "Подтверждение", wxYES_NO) == wxYES) {
        try {
            db_->deleteRecord(currentTable_, id);
            PopulateList();
            GetStatusBar()->SetStatusText("Запись удалена", 1);
        }
        catch (const DbException& e) { ShowError(e.what()); }
    }
}

void MainFrame::OnTableSelect(wxCommandEvent&) {
    currentTable_ = tableChoice_->GetStringSelection().ToStdString();
    lastSearchQuery_ = "";
    searchCtrl_->SetValue("");
    PopulateList();
}