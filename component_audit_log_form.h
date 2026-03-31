#pragma once

ref class AuditLogForm : public Form {
private:
    String^ connStr;
    DataGridView^ dgvNhatKy;
    Button^ btnTaiLai;

    void taiNhatKy() {
        try {
            OdbcConnection^ conn = gcnew OdbcConnection(connStr);
            conn->Open();
            damBaoBangNhatKyTonTaiChung(conn);

            OdbcDataAdapter^ adapter = gcnew OdbcDataAdapter(
                "SELECT id, hanh_dong, nguoi_dung_id, ket_qua, mo_ta, thoi_gian_tao "
                "FROM NhatKyTruyCap ORDER BY id DESC LIMIT 1000",
                conn
            );

            DataTable^ dt = gcnew DataTable();
            adapter->Fill(dt);
            dgvNhatKy->DataSource = dt;
            conn->Close();
        }
        catch (Exception^ ex) {
            MessageBox::Show("Loi tai nhat ky he thong:\n" + ex->Message, "Loi", MessageBoxButtons::OK, MessageBoxIcon::Error);
        }
    }

    void OnTaiLaiClick(Object^ sender, EventArgs^ e) {
        taiNhatKy();
    }

public:
    AuditLogForm(String^ ketNoi) {
        connStr = ketNoi;
        this->Text = "Nhat ky he thong (Admin)";
        this->Size = Drawing::Size(1100, 580);
        this->StartPosition = FormStartPosition::CenterParent;

        dgvNhatKy = gcnew DataGridView();
        dgvNhatKy->Location = Point(20, 20);
        dgvNhatKy->Size = Drawing::Size(1040, 460);
        dgvNhatKy->ReadOnly = true;
        dgvNhatKy->AllowUserToAddRows = false;
        dgvNhatKy->AllowUserToDeleteRows = false;
        dgvNhatKy->SelectionMode = DataGridViewSelectionMode::FullRowSelect;
        dgvNhatKy->MultiSelect = false;
        dgvNhatKy->AutoSizeColumnsMode = DataGridViewAutoSizeColumnsMode::Fill;
        this->Controls->Add(dgvNhatKy);

        btnTaiLai = gcnew Button();
        btnTaiLai->Text = "Tai lai nhat ky";
        btnTaiLai->Location = Point(20, 495);
        btnTaiLai->Size = Drawing::Size(150, 34);
        btnTaiLai->Click += gcnew EventHandler(this, &AuditLogForm::OnTaiLaiClick);
        this->Controls->Add(btnTaiLai);

        taiNhatKy();
    }
};
