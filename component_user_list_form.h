#pragma once

ref class UserListForm : public Form {
private:
    String^ connStr;
    DataGridView^ dgvUsers;
    Button^ btnTaiLai;
    Button^ btnXoaNguoiDung;

public:
    UserListForm(String^ ketNoi) {
        connStr = ketNoi;
        this->Text = "Danh sach nguoi dung da mask";
        this->Size = Drawing::Size(980, 520);
        this->StartPosition = FormStartPosition::CenterParent;

        dgvUsers = gcnew DataGridView();
        dgvUsers->Location = Point(20, 20);
        dgvUsers->Size = Drawing::Size(930, 390);
        dgvUsers->ReadOnly = true;
        dgvUsers->AllowUserToAddRows = false;
        dgvUsers->AllowUserToDeleteRows = false;
        dgvUsers->SelectionMode = DataGridViewSelectionMode::FullRowSelect;
        dgvUsers->MultiSelect = false;
        dgvUsers->AutoSizeColumnsMode = DataGridViewAutoSizeColumnsMode::Fill;
        this->Controls->Add(dgvUsers);

        btnTaiLai = gcnew Button();
        btnTaiLai->Text = "Tai lai danh sach";
        btnTaiLai->Location = Point(20, 425);
        btnTaiLai->Size = Drawing::Size(160, 35);
        btnTaiLai->Click += gcnew EventHandler(this, &UserListForm::OnTaiLaiClick);
        this->Controls->Add(btnTaiLai);

        btnXoaNguoiDung = gcnew Button();
        btnXoaNguoiDung->Text = "Xoa nguoi dung da chon";
        btnXoaNguoiDung->Location = Point(200, 425);
        btnXoaNguoiDung->Size = Drawing::Size(220, 35);
        btnXoaNguoiDung->Click += gcnew EventHandler(this, &UserListForm::OnXoaNguoiDungClick);
        this->Controls->Add(btnXoaNguoiDung);

        TaiDuLieuLenLuoi();
    }

private:
    void damBaoBangNhatKyTonTai(OdbcConnection^ conn) {
        OdbcCommand^ cmd = gcnew OdbcCommand(
            "CREATE TABLE IF NOT EXISTS NhatKyTruyCap ("
            "id BIGINT UNSIGNED AUTO_INCREMENT PRIMARY KEY, "
            "hanh_dong VARCHAR(50) NOT NULL, "
            "nguoi_dung_id BIGINT UNSIGNED NULL, "
            "ket_qua VARCHAR(20) NOT NULL, "
            "mo_ta VARCHAR(255) NULL, "
            "thoi_gian_tao TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP"
            ") ENGINE=InnoDB",
            conn
        );
        cmd->ExecuteNonQuery();
    }

    void ghiNhatKy(OdbcConnection^ conn, String^ hanhDong, Nullable<Int64> nguoiDungId, String^ ketQua, String^ moTa) {
        OdbcCommand^ cmd = gcnew OdbcCommand(
            "INSERT INTO NhatKyTruyCap (hanh_dong, nguoi_dung_id, ket_qua, mo_ta) VALUES (?, ?, ?, ?)",
            conn
        );
        cmd->Parameters->AddWithValue("", hanhDong);

        OdbcParameter^ pId = gcnew OdbcParameter();
        pId->OdbcType = OdbcType::BigInt;
        if (nguoiDungId.HasValue) {
            pId->Value = nguoiDungId.Value;
        }
        else {
            pId->Value = DBNull::Value;
        }
        cmd->Parameters->Add(pId);
        cmd->Parameters->AddWithValue("", ketQua);
        cmd->Parameters->AddWithValue("", moTa);
        cmd->ExecuteNonQuery();
    }

    void TaiDuLieuLenLuoi() {
        try {
            OdbcConnection^ conn = gcnew OdbcConnection(connStr);
            conn->Open();
            damBaoBangHoSoNguoiDungTonTai(conn);
            damBaoBangTaiKhoanNguoiDungTonTai(conn);

            OdbcDataAdapter^ adapter = gcnew OdbcDataAdapter(
                "SELECT h.nguoi_dung_id AS id, "
                "t.ten_dang_nhap AS ten_dang_nhap, "
                "h.ho_ten_masked AS ho_ten, "
                "h.so_dien_thoai_masked AS so_dien_thoai, "
                "h.email_masked AS email, "
                "h.cccd_masked AS cccd, "
                "h.dia_chi_masked AS dia_chi "
                "FROM HoSoNguoiDung h "
                "LEFT JOIN TaiKhoanNguoiDung t ON h.nguoi_dung_id = t.id "
                "ORDER BY h.nguoi_dung_id DESC",
                conn
            );

            DataTable^ dt = gcnew DataTable();
            adapter->Fill(dt);
            dgvUsers->DataSource = dt;
            conn->Close();
        }
        catch (Exception^ ex) {
            MessageBox::Show("Loi tai danh sach nguoi dung:\n" + ex->Message, "Loi", MessageBoxButtons::OK, MessageBoxIcon::Error);
        }
    }

    void OnTaiLaiClick(Object^ sender, EventArgs^ e) {
        TaiDuLieuLenLuoi();
    }

    void OnXoaNguoiDungClick(Object^ sender, EventArgs^ e) {
        if (dgvUsers->SelectedRows->Count <= 0) {
            MessageBox::Show("Hay chon mot nguoi dung can xoa.", "Thong bao", MessageBoxButtons::OK, MessageBoxIcon::Information);
            return;
        }

        Object^ idObj = dgvUsers->SelectedRows[0]->Cells["id"]->Value;
        if (idObj == nullptr) {
            MessageBox::Show("Khong lay duoc ID nguoi dung.", "Loi", MessageBoxButtons::OK, MessageBoxIcon::Warning);
            return;
        }

        Int64 id = Convert::ToInt64(idObj);
        if (MessageBox::Show(
            "Ban co chac chan muon xoa nguoi dung ID = " + Convert::ToString(id) + "?",
            "Xac nhan xoa",
            MessageBoxButtons::YesNo,
            MessageBoxIcon::Question
        ) != System::Windows::Forms::DialogResult::Yes) {
            return;
        }

        try {
            OdbcConnection^ conn = gcnew OdbcConnection(connStr);
            conn->Open();
            damBaoBangHoSoNguoiDungTonTai(conn);
            damBaoBangNhatKyTonTai(conn);

            OdbcCommand^ cmdDel = gcnew OdbcCommand(
                "DELETE FROM HoSoNguoiDung WHERE nguoi_dung_id = ?",
                conn
            );
            cmdDel->Parameters->AddWithValue("", id);
            int soDong = cmdDel->ExecuteNonQuery();

            if (soDong > 0) {
                ghiNhatKy(conn, "DELETE_PROFILE", id, "SUCCESS", "Xoa ho so nguoi dung tu HoSoNguoiDung");
                MessageBox::Show("Da xoa ho so nguoi dung ID = " + Convert::ToString(id), "Thong bao", MessageBoxButtons::OK, MessageBoxIcon::Information);
            }
            else {
                ghiNhatKy(conn, "DELETE_PROFILE", id, "FAIL", "Khong tim thay ho so de xoa");
                MessageBox::Show("Khong tim thay ho so de xoa.", "Thong bao", MessageBoxButtons::OK, MessageBoxIcon::Warning);
            }

            conn->Close();
            TaiDuLieuLenLuoi();
        }
        catch (Exception^ ex) {
            MessageBox::Show("Loi xoa nguoi dung:\n" + ex->Message, "Loi", MessageBoxButtons::OK, MessageBoxIcon::Error);
        }
    }
};
