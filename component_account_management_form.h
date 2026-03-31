#pragma once

ref class AccountManagementForm : public Form {
private:
    String^ connStr;
    Int64 idTaiKhoanDangNhap;
    DataGridView^ dgvTaiKhoan;
    TextBox^ txtTkTen;
    TextBox^ txtTkMatKhau;
    ComboBox^ cboTkVaiTro;
    ComboBox^ cboTkTrangThai;
    Button^ btnTkThem;
    Button^ btnTkCapNhat;
    Button^ btnTkXoa;
    Button^ btnTkTaiLai;
    bool dangGanDuLieuTuLuoi;

    int layTrangThaiSoTuCombo() {
        return cboTkTrangThai->SelectedIndex == 0 ? 1 : 0;
    }

    void datComboTrangThaiTuSo(int tt) {
        cboTkTrangThai->SelectedIndex = (tt == 1) ? 0 : 1;
    }

    void chonVaiTroTrongCombo(String^ v) {
        if (String::IsNullOrWhiteSpace(v)) {
            cboTkVaiTro->SelectedIndex = 0;
            return;
        }
        int i = 0;
        while (i < cboTkVaiTro->Items->Count) {
            if (String::Equals(cboTkVaiTro->Items[i]->ToString(), v, StringComparison::OrdinalIgnoreCase)) {
                cboTkVaiTro->SelectedIndex = i;
                return;
            }
            i = i + 1;
        }
        cboTkVaiTro->SelectedIndex = 0;
    }

    Int64 layIdTaiKhoanTuLuoi() {
        if (dgvTaiKhoan->SelectedRows->Count != 1) {
            return 0;
        }
        Object^ o = dgvTaiKhoan->SelectedRows[0]->Cells["id"]->Value;
        if (o == nullptr || o == DBNull::Value) {
            return 0;
        }
        return Convert::ToInt64(o);
    }

    bool tenDangNhapTrung(OdbcConnection^ conn, String^ ten, Int64 boQuaId) {
        OdbcCommand^ cmd;
        if (boQuaId <= 0) {
            cmd = gcnew OdbcCommand("SELECT COUNT(*) FROM TaiKhoanNguoiDung WHERE ten_dang_nhap = ?", conn);
            cmd->Parameters->AddWithValue("", ten->Trim());
        }
        else {
            cmd = gcnew OdbcCommand("SELECT COUNT(*) FROM TaiKhoanNguoiDung WHERE ten_dang_nhap = ? AND id <> ?", conn);
            cmd->Parameters->AddWithValue("", ten->Trim());
            cmd->Parameters->AddWithValue("", boQuaId);
        }
        return Convert::ToInt32(cmd->ExecuteScalar()) > 0;
    }

    void TaiBangTaiKhoan() {
        try {
            OdbcConnection^ conn = gcnew OdbcConnection(connStr);
            conn->Open();
            damBaoBangNhatKyTonTaiChung(conn);
            damBaoBangTaiKhoanNguoiDungTonTai(conn);
            damBaoTaiKhoanAdminMacDinhNguoiDung(conn);

            OdbcDataAdapter^ adapter = gcnew OdbcDataAdapter(
                "SELECT id, ten_dang_nhap, vai_tro, trang_thai, thoi_gian_tao FROM TaiKhoanNguoiDung ORDER BY id",
                conn
            );
            DataTable^ dt = gcnew DataTable();
            adapter->Fill(dt);
            dangGanDuLieuTuLuoi = true;
            dgvTaiKhoan->DataSource = dt;
            dangGanDuLieuTuLuoi = false;
            conn->Close();
        }
        catch (Exception^ ex) {
            MessageBox::Show("Loi tai danh sach tai khoan:\n" + ex->Message, "Loi", MessageBoxButtons::OK, MessageBoxIcon::Error);
        }
    }

    void OnDgvTaiKhoanSelectionChanged(Object^ sender, EventArgs^ e) {
        if (dangGanDuLieuTuLuoi) {
            return;
        }
        if (dgvTaiKhoan->SelectedRows->Count != 1) {
            txtTkTen->Clear();
            txtTkMatKhau->Clear();
            chonVaiTroTrongCombo("EMPLOYEE");
            datComboTrangThaiTuSo(1);
            return;
        }
        DataGridViewRow^ row = dgvTaiKhoan->SelectedRows[0];
        if (row->IsNewRow) {
            return;
        }
        txtTkTen->Text = Convert::ToString(row->Cells["ten_dang_nhap"]->Value);
        txtTkMatKhau->Clear();
        chonVaiTroTrongCombo(Convert::ToString(row->Cells["vai_tro"]->Value));
        datComboTrangThaiTuSo(Convert::ToInt32(row->Cells["trang_thai"]->Value));
    }

    void OnTkThemClick(Object^ sender, EventArgs^ e) {
        if (String::IsNullOrWhiteSpace(txtTkTen->Text)) {
            MessageBox::Show("Nhap ten dang nhap.", "Thieu du lieu", MessageBoxButtons::OK, MessageBoxIcon::Warning);
            return;
        }
        if (String::IsNullOrWhiteSpace(txtTkMatKhau->Text)) {
            MessageBox::Show("Nhap mat khau cho tai khoan moi.", "Thieu mat khau", MessageBoxButtons::OK, MessageBoxIcon::Warning);
            return;
        }
        if (txtTkTen->Text->Trim()->Length > 64) {
            MessageBox::Show("Ten dang nhap toi da 64 ky tu.", "Khong hop le", MessageBoxButtons::OK, MessageBoxIcon::Warning);
            return;
        }

        try {
            OdbcConnection^ conn = gcnew OdbcConnection(connStr);
            conn->Open();
            damBaoBangNhatKyTonTaiChung(conn);
            damBaoBangTaiKhoanNguoiDungTonTai(conn);

            if (tenDangNhapTrung(conn, txtTkTen->Text, 0)) {
                conn->Close();
                MessageBox::Show("Ten dang nhap da ton tai.", "Trung lap", MessageBoxButtons::OK, MessageBoxIcon::Warning);
                return;
            }

            String^ vt = cboTkVaiTro->SelectedItem->ToString();
            int tt = layTrangThaiSoTuCombo();
            OdbcCommand^ cmd = gcnew OdbcCommand(
                "INSERT INTO TaiKhoanNguoiDung (ten_dang_nhap, mat_khau_hash, vai_tro, trang_thai) VALUES (?, ?, ?, ?)",
                conn
            );
            cmd->Parameters->AddWithValue("", txtTkTen->Text->Trim());
            cmd->Parameters->AddWithValue("", bamSHA256Hex(txtTkMatKhau->Text));
            cmd->Parameters->AddWithValue("", vt);
            cmd->Parameters->AddWithValue("", tt);
            cmd->ExecuteNonQuery();
            ghiNhatKyChung(conn, "ACCOUNT_CREATE", Nullable<Int64>(idTaiKhoanDangNhap), "SUCCESS", "Them TK: " + txtTkTen->Text->Trim());
            conn->Close();
            TaiBangTaiKhoan();
            MessageBox::Show("Da them tai khoan.", "Thong bao", MessageBoxButtons::OK, MessageBoxIcon::Information);
        }
        catch (Exception^ ex) {
            MessageBox::Show("Loi them tai khoan:\n" + ex->Message, "Loi", MessageBoxButtons::OK, MessageBoxIcon::Error);
        }
    }

    void OnTkCapNhatClick(Object^ sender, EventArgs^ e) {
        Int64 id = layIdTaiKhoanTuLuoi();
        if (id <= 0) {
            MessageBox::Show("Chon mot dong trong luoi de cap nhat.", "Thong bao", MessageBoxButtons::OK, MessageBoxIcon::Information);
            return;
        }
        if (String::IsNullOrWhiteSpace(txtTkTen->Text)) {
            MessageBox::Show("Ten dang nhap khong duoc de trong.", "Thieu du lieu", MessageBoxButtons::OK, MessageBoxIcon::Warning);
            return;
        }
        if (txtTkTen->Text->Trim()->Length > 64) {
            MessageBox::Show("Ten dang nhap toi da 64 ky tu.", "Khong hop le", MessageBoxButtons::OK, MessageBoxIcon::Warning);
            return;
        }

        try {
            OdbcConnection^ conn = gcnew OdbcConnection(connStr);
            conn->Open();
            damBaoBangNhatKyTonTaiChung(conn);

            if (tenDangNhapTrung(conn, txtTkTen->Text, id)) {
                conn->Close();
                MessageBox::Show("Ten dang nhap da ton tai.", "Trung lap", MessageBoxButtons::OK, MessageBoxIcon::Warning);
                return;
            }

            String^ vt = cboTkVaiTro->SelectedItem->ToString();
            int tt = layTrangThaiSoTuCombo();
            if (String::IsNullOrWhiteSpace(txtTkMatKhau->Text)) {
                OdbcCommand^ cmd = gcnew OdbcCommand(
                    "UPDATE TaiKhoanNguoiDung SET ten_dang_nhap = ?, vai_tro = ?, trang_thai = ? WHERE id = ?",
                    conn
                );
                cmd->Parameters->AddWithValue("", txtTkTen->Text->Trim());
                cmd->Parameters->AddWithValue("", vt);
                cmd->Parameters->AddWithValue("", tt);
                cmd->Parameters->AddWithValue("", id);
                cmd->ExecuteNonQuery();
            }
            else {
                OdbcCommand^ cmd = gcnew OdbcCommand(
                    "UPDATE TaiKhoanNguoiDung SET ten_dang_nhap = ?, mat_khau_hash = ?, vai_tro = ?, trang_thai = ? WHERE id = ?",
                    conn
                );
                cmd->Parameters->AddWithValue("", txtTkTen->Text->Trim());
                cmd->Parameters->AddWithValue("", bamSHA256Hex(txtTkMatKhau->Text));
                cmd->Parameters->AddWithValue("", vt);
                cmd->Parameters->AddWithValue("", tt);
                cmd->Parameters->AddWithValue("", id);
                cmd->ExecuteNonQuery();
            }
            ghiNhatKyChung(conn, "ACCOUNT_UPDATE", Nullable<Int64>(idTaiKhoanDangNhap), "SUCCESS", "Cap nhat TK id=" + Convert::ToString(id));
            conn->Close();
            TaiBangTaiKhoan();
            MessageBox::Show("Da cap nhat tai khoan.", "Thong bao", MessageBoxButtons::OK, MessageBoxIcon::Information);
        }
        catch (Exception^ ex) {
            MessageBox::Show("Loi cap nhat tai khoan:\n" + ex->Message, "Loi", MessageBoxButtons::OK, MessageBoxIcon::Error);
        }
    }

    void OnTkXoaClick(Object^ sender, EventArgs^ e) {
        Int64 id = layIdTaiKhoanTuLuoi();
        if (id <= 0) {
            MessageBox::Show("Chon mot tai khoan de xoa.", "Thong bao", MessageBoxButtons::OK, MessageBoxIcon::Information);
            return;
        }
        if (id == idTaiKhoanDangNhap) {
            MessageBox::Show("Khong the xoa tai khoan dang dang nhap.", "Khong hop le", MessageBoxButtons::OK, MessageBoxIcon::Warning);
            return;
        }
        if (MessageBox::Show(
            "Xoa tai khoan id = " + Convert::ToString(id) + "?",
            "Xac nhan",
            MessageBoxButtons::YesNo,
            MessageBoxIcon::Question
        ) != System::Windows::Forms::DialogResult::Yes) {
            return;
        }

        try {
            OdbcConnection^ conn = gcnew OdbcConnection(connStr);
            conn->Open();
            damBaoBangNhatKyTonTaiChung(conn);
            OdbcCommand^ cmd = gcnew OdbcCommand("DELETE FROM TaiKhoanNguoiDung WHERE id = ?", conn);
            cmd->Parameters->AddWithValue("", id);
            int n = cmd->ExecuteNonQuery();
            if (n > 0) {
                ghiNhatKyChung(conn, "ACCOUNT_DELETE", Nullable<Int64>(idTaiKhoanDangNhap), "SUCCESS", "Xoa TK id=" + Convert::ToString(id));
            }
            conn->Close();
            TaiBangTaiKhoan();
            MessageBox::Show("Da xoa tai khoan.", "Thong bao", MessageBoxButtons::OK, MessageBoxIcon::Information);
        }
        catch (Exception^ ex) {
            MessageBox::Show("Loi xoa tai khoan:\n" + ex->Message, "Loi", MessageBoxButtons::OK, MessageBoxIcon::Error);
        }
    }

    void OnTkTaiLaiClick(Object^ sender, EventArgs^ e) {
        TaiBangTaiKhoan();
    }

public:
    AccountManagementForm(String^ ketNoi, Int64 idNguoiThaoTac) {
        connStr = ketNoi;
        idTaiKhoanDangNhap = idNguoiThaoTac;
        dangGanDuLieuTuLuoi = false;
        this->Text = "Quan ly tai khoan";
        this->Size = Drawing::Size(780, 480);
        this->StartPosition = FormStartPosition::CenterParent;

        dgvTaiKhoan = gcnew DataGridView();
        dgvTaiKhoan->Location = Point(15, 15);
        dgvTaiKhoan->Size = Drawing::Size(735, 260);
        dgvTaiKhoan->ReadOnly = true;
        dgvTaiKhoan->AllowUserToAddRows = false;
        dgvTaiKhoan->AllowUserToDeleteRows = false;
        dgvTaiKhoan->SelectionMode = DataGridViewSelectionMode::FullRowSelect;
        dgvTaiKhoan->MultiSelect = false;
        dgvTaiKhoan->AutoSizeColumnsMode = DataGridViewAutoSizeColumnsMode::Fill;
        dgvTaiKhoan->SelectionChanged += gcnew EventHandler(this, &AccountManagementForm::OnDgvTaiKhoanSelectionChanged);
        this->Controls->Add(dgvTaiKhoan);

        Label^ lbTen = gcnew Label();
        lbTen->Text = "Ten dang nhap";
        lbTen->Location = Point(15, 288);
        lbTen->Size = Drawing::Size(100, 22);
        this->Controls->Add(lbTen);

        txtTkTen = gcnew TextBox();
        txtTkTen->Location = Point(120, 285);
        txtTkTen->Size = Drawing::Size(220, 24);
        this->Controls->Add(txtTkTen);

        Label^ lbMk = gcnew Label();
        lbMk->Text = "Mat khau";
        lbMk->Location = Point(15, 318);
        lbMk->Size = Drawing::Size(100, 22);
        this->Controls->Add(lbMk);

        txtTkMatKhau = gcnew TextBox();
        txtTkMatKhau->Location = Point(120, 315);
        txtTkMatKhau->Size = Drawing::Size(220, 24);
        txtTkMatKhau->UseSystemPasswordChar = true;
        this->Controls->Add(txtTkMatKhau);

        Label^ lbGoiY = gcnew Label();
        lbGoiY->Text = "(De trong khi sua = giu mat khau cu)";
        lbGoiY->Location = Point(350, 318);
        lbGoiY->Size = Drawing::Size(280, 22);
        this->Controls->Add(lbGoiY);

        Label^ lbVt = gcnew Label();
        lbVt->Text = "Vai tro";
        lbVt->Location = Point(15, 350);
        lbVt->Size = Drawing::Size(100, 22);
        this->Controls->Add(lbVt);

        cboTkVaiTro = gcnew ComboBox();
        cboTkVaiTro->Location = Point(120, 347);
        cboTkVaiTro->Size = Drawing::Size(120, 24);
        cboTkVaiTro->DropDownStyle = ComboBoxStyle::DropDownList;
        cboTkVaiTro->Items->Add("EMPLOYEE");
        cboTkVaiTro->Items->Add("ADMIN");
        cboTkVaiTro->SelectedIndex = 0;
        this->Controls->Add(cboTkVaiTro);

        Label^ lbTt = gcnew Label();
        lbTt->Text = "Trang thai";
        lbTt->Location = Point(260, 350);
        lbTt->Size = Drawing::Size(80, 22);
        this->Controls->Add(lbTt);

        cboTkTrangThai = gcnew ComboBox();
        cboTkTrangThai->Location = Point(345, 347);
        cboTkTrangThai->Size = Drawing::Size(140, 24);
        cboTkTrangThai->DropDownStyle = ComboBoxStyle::DropDownList;
        cboTkTrangThai->Items->Add("Hoat dong");
        cboTkTrangThai->Items->Add("Ngung");
        cboTkTrangThai->SelectedIndex = 0;
        this->Controls->Add(cboTkTrangThai);

        btnTkThem = gcnew Button();
        btnTkThem->Text = "Them";
        btnTkThem->Location = Point(15, 390);
        btnTkThem->Size = Drawing::Size(90, 30);
        btnTkThem->Click += gcnew EventHandler(this, &AccountManagementForm::OnTkThemClick);
        this->Controls->Add(btnTkThem);

        btnTkCapNhat = gcnew Button();
        btnTkCapNhat->Text = "Cap nhat";
        btnTkCapNhat->Location = Point(115, 390);
        btnTkCapNhat->Size = Drawing::Size(90, 30);
        btnTkCapNhat->Click += gcnew EventHandler(this, &AccountManagementForm::OnTkCapNhatClick);
        this->Controls->Add(btnTkCapNhat);

        btnTkXoa = gcnew Button();
        btnTkXoa->Text = "Xoa";
        btnTkXoa->Location = Point(215, 390);
        btnTkXoa->Size = Drawing::Size(90, 30);
        btnTkXoa->Click += gcnew EventHandler(this, &AccountManagementForm::OnTkXoaClick);
        this->Controls->Add(btnTkXoa);

        btnTkTaiLai = gcnew Button();
        btnTkTaiLai->Text = "Tai lai";
        btnTkTaiLai->Location = Point(315, 390);
        btnTkTaiLai->Size = Drawing::Size(90, 30);
        btnTkTaiLai->Click += gcnew EventHandler(this, &AccountManagementForm::OnTkTaiLaiClick);
        this->Controls->Add(btnTkTaiLai);

        TaiBangTaiKhoan();
    }
};
