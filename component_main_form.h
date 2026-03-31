#pragma once

ref class MainForm : public Form {
private:
    TextBox^ txtHoTen;
    TextBox^ txtSoDienThoai;
    TextBox^ txtEmail;
    TextBox^ txtCCCD;
    TextBox^ txtDiaChi;
    TextBox^ txtSearchId;
    TextBox^ txtKey;
    TextBox^ txtRaw;
    TextBox^ txtMasked;
    Button^ btnMaskAndSave;
    Button^ btnSearchById;
    Button^ btnViewRaw;
    Button^ btnViewAllMaskedUsers;
    Button^ btnQuanLyTaiKhoan;
    Button^ btnHoSoCaNhan;
    Button^ btnQuanLyFile;
    Button^ btnNhatKyHeThong;
    Button^ btnDangXuat;
    String^ connStr;
    bool daTaiBanGhiTheoId;
    bool daDangNhapAdmin;
    Int64 idTaiKhoanDangNhap;
    String^ vaiTroDangNhap;
    long long idDangChon;
    String^ keyHashDangChon;
    UserRecord* banGhiEncDangChon;
    UserRecord* banGhiMaskedDangChon;

public:
    bool YeuCauDangXuat;

    MainForm(Int64 userId, String^ vaiTro) {
        this->Text = "Data Masking - WinForms C++/CLI";
        this->Size = Drawing::Size(860, 620);
        this->StartPosition = FormStartPosition::CenterScreen;
        YeuCauDangXuat = false;
        daTaiBanGhiTheoId = false;
        idTaiKhoanDangNhap = userId;
        vaiTroDangNhap = vaiTro;
        daDangNhapAdmin = vaiTro != nullptr && String::Equals(vaiTro, "ADMIN", StringComparison::OrdinalIgnoreCase);
        idDangChon = 0;
        keyHashDangChon = "";
        banGhiEncDangChon = new UserRecord();
        banGhiMaskedDangChon = new UserRecord();

        connStr = layChuoiKetNoiDb();

        Label^ lbTitle = gcnew Label();
        lbTitle->Text = "Nhap du lieu nguoi dung";
        lbTitle->Font = gcnew Drawing::Font("Segoe UI", 11, FontStyle::Bold);
        lbTitle->Location = Point(20, 15);
        lbTitle->AutoSize = true;
        this->Controls->Add(lbTitle);

        btnQuanLyTaiKhoan = gcnew Button();
        btnQuanLyTaiKhoan->Text = "Quan ly tai khoan";
        btnQuanLyTaiKhoan->Location = Point(640, 10);
        btnQuanLyTaiKhoan->Size = Drawing::Size(170, 28);
        btnQuanLyTaiKhoan->Click += gcnew EventHandler(this, &MainForm::OnQuanLyTaiKhoanClick);
        btnQuanLyTaiKhoan->Visible = daDangNhapAdmin;
        this->Controls->Add(btnQuanLyTaiKhoan);

        btnHoSoCaNhan = gcnew Button();
        btnHoSoCaNhan->Text = "Ho so ca nhan";
        btnHoSoCaNhan->Location = Point(460, 10);
        btnHoSoCaNhan->Size = Drawing::Size(170, 28);
        btnHoSoCaNhan->Click += gcnew EventHandler(this, &MainForm::OnHoSoCaNhanClick);
        this->Controls->Add(btnHoSoCaNhan);

        btnQuanLyFile = gcnew Button();
        btnQuanLyFile->Text = "Quan ly file";
        btnQuanLyFile->Location = Point(280, 10);
        btnQuanLyFile->Size = Drawing::Size(170, 28);
        btnQuanLyFile->Click += gcnew EventHandler(this, &MainForm::OnQuanLyFileClick);
        this->Controls->Add(btnQuanLyFile);

        btnDangXuat = gcnew Button();
        btnDangXuat->Text = "Dang xuat";
        btnDangXuat->Location = Point(160, 10);
        btnDangXuat->Size = Drawing::Size(110, 28);
        btnDangXuat->Click += gcnew EventHandler(this, &MainForm::OnDangXuatClick);
        this->Controls->Add(btnDangXuat);

        if (daDangNhapAdmin) {
            taoLabel("Ho ten", 20, 55);
            txtHoTen = taoTextBox(160, 50, 300);

            taoLabel("So dien thoai", 20, 95);
            txtSoDienThoai = taoTextBox(160, 90, 300);

            taoLabel("Email", 20, 135);
            txtEmail = taoTextBox(160, 130, 300);

            taoLabel("CCCD", 20, 175);
            txtCCCD = taoTextBox(160, 170, 300);

            taoLabel("Dia chi", 20, 215);
            txtDiaChi = taoTextBox(160, 210, 600);

            btnMaskAndSave = gcnew Button();
            btnMaskAndSave->Text = "Mask + Luu DB";
            btnMaskAndSave->Location = Point(160, 255);
            btnMaskAndSave->Size = Drawing::Size(140, 35);
            btnMaskAndSave->Click += gcnew EventHandler(this, &MainForm::OnMaskAndSaveClick);
            this->Controls->Add(btnMaskAndSave);

            btnNhatKyHeThong = gcnew Button();
            btnNhatKyHeThong->Text = "Nhat ky he thong";
            btnNhatKyHeThong->Location = Point(20, 255);
            btnNhatKyHeThong->Size = Drawing::Size(130, 35);
            btnNhatKyHeThong->Click += gcnew EventHandler(this, &MainForm::OnNhatKyHeThongClick);
            this->Controls->Add(btnNhatKyHeThong);

            taoLabel("Tim theo ID", 20, 305);
            txtSearchId = taoTextBox(160, 300, 120);

            btnSearchById = gcnew Button();
            btnSearchById->Text = "Tim";
            btnSearchById->Location = Point(300, 298);
            btnSearchById->Size = Drawing::Size(90, 30);
            btnSearchById->Click += gcnew EventHandler(this, &MainForm::OnSearchByIdClick);
            this->Controls->Add(btnSearchById);

            btnViewRaw = gcnew Button();
            btnViewRaw->Text = "Xem du lieu goc";
            btnViewRaw->Location = Point(410, 298);
            btnViewRaw->Size = Drawing::Size(150, 30);
            btnViewRaw->Click += gcnew EventHandler(this, &MainForm::OnViewRawClick);
            this->Controls->Add(btnViewRaw);

            btnViewAllMaskedUsers = gcnew Button();
            btnViewAllMaskedUsers->Text = "Xem DS nguoi dung (mask)";
            btnViewAllMaskedUsers->Location = Point(580, 298);
            btnViewAllMaskedUsers->Size = Drawing::Size(240, 30);
            btnViewAllMaskedUsers->Click += gcnew EventHandler(this, &MainForm::OnViewAllMaskedUsersClick);
            this->Controls->Add(btnViewAllMaskedUsers);

            taoLabel("Key ma/giai ma", 20, 345);
            txtKey = taoTextBox(160, 340, 400);
            txtKey->UseSystemPasswordChar = true;

            taoLabel("Du lieu goc ", 20, 390);
            txtRaw = taoTextBoxDaDong(20, 415, 390, 145);
            txtRaw->Text = "Du lieu goc dang an. Bam 'Xem du lieu goc' va nhap key de xem.";

            taoLabel("Du lieu da che (mac dinh hien)", 430, 390);
            txtMasked = taoTextBoxDaDong(430, 415, 390, 145);
        }
        else {
            lbTitle->Text = "Trang nhan vien";
            Label^ lbHint = gcnew Label();
            lbHint->Text = "Role EMPLOYEE: chi duoc xem/sua ho so cua minh, upload va download file.";
            lbHint->Location = Point(20, 65);
            lbHint->Size = Drawing::Size(760, 30);
            this->Controls->Add(lbHint);
        }
    }

    ~MainForm() {
        this->!MainForm();
    }

    !MainForm() {
        if (banGhiEncDangChon != nullptr) {
            delete banGhiEncDangChon;
            banGhiEncDangChon = nullptr;
        }
        if (banGhiMaskedDangChon != nullptr) {
            delete banGhiMaskedDangChon;
            banGhiMaskedDangChon = nullptr;
        }
    }

private:
    void taoLabel(String^ text, int x, int y) {
        Label^ lb = gcnew Label();
        lb->Text = text;
        lb->Location = Point(x, y);
        lb->Size = Drawing::Size(130, 25);
        this->Controls->Add(lb);
    }

    TextBox^ taoTextBox(int x, int y, int w) {
        TextBox^ txt = gcnew TextBox();
        txt->Location = Point(x, y);
        txt->Size = Drawing::Size(w, 25);
        this->Controls->Add(txt);
        return txt;
    }

    TextBox^ taoTextBoxDaDong(int x, int y, int w, int h) {
        TextBox^ txt = gcnew TextBox();
        txt->Location = Point(x, y);
        txt->Size = Drawing::Size(w, h);
        txt->Multiline = true;
        txt->ReadOnly = true;
        txt->ScrollBars = ScrollBars::Vertical;
        this->Controls->Add(txt);
        return txt;
    }

    bool laToanSo(String^ s) {
        if (String::IsNullOrWhiteSpace(s)) {
            return false;
        }
        int i = 0;
        while (i < s->Length) {
            wchar_t c = s[i];
            if (c < L'0' || c > L'9') {
                return false;
            }
            i = i + 1;
        }
        return true;
    }

    bool emailCoDangCoBan(String^ s) {
        if (String::IsNullOrWhiteSpace(s)) {
            return false;
        }
        int viTriA = -1;
        int viTriCham = -1;
        int i = 0;
        while (i < s->Length) {
            if (s[i] == L'@' && viTriA == -1) {
                viTriA = i;
            }
            if (s[i] == L'.') {
                viTriCham = i;
            }
            i = i + 1;
        }

        // Can co @, co dau cham sau @, va khong o vi tri dau/cuoi
        if (viTriA <= 0) {
            return false;
        }
        if (viTriCham <= viTriA + 1) {
            return false;
        }
        if (viTriCham >= s->Length - 1) {
            return false;
        }
        return true;
    }

    bool validateInput() {
        if (String::IsNullOrWhiteSpace(txtHoTen->Text)) {
            MessageBox::Show("Ho ten khong duoc de trong.", "Validate", MessageBoxButtons::OK, MessageBoxIcon::Warning);
            txtHoTen->Focus();
            return false;
        }
        if (txtHoTen->Text->Length > 100) {
            MessageBox::Show("Ho ten toi da 100 ky tu.", "Validate", MessageBoxButtons::OK, MessageBoxIcon::Warning);
            txtHoTen->Focus();
            return false;
        }
        if (!laToanSo(txtSoDienThoai->Text) || txtSoDienThoai->Text->Length < 9 || txtSoDienThoai->Text->Length > 11) {
            MessageBox::Show("So dien thoai phai la so va co do dai 9-11 ky tu.", "Validate", MessageBoxButtons::OK, MessageBoxIcon::Warning);
            txtSoDienThoai->Focus();
            return false;
        }
        if (!emailCoDangCoBan(txtEmail->Text)) {
            MessageBox::Show("Email khong hop le.", "Validate", MessageBoxButtons::OK, MessageBoxIcon::Warning);
            txtEmail->Focus();
            return false;
        }
        if (txtEmail->Text->Length > 120) {
            MessageBox::Show("Email toi da 120 ky tu.", "Validate", MessageBoxButtons::OK, MessageBoxIcon::Warning);
            txtEmail->Focus();
            return false;
        }
        if (!laToanSo(txtCCCD->Text) || txtCCCD->Text->Length != 12) {
            MessageBox::Show("CCCD phai gom dung 12 chu so.", "Validate", MessageBoxButtons::OK, MessageBoxIcon::Warning);
            txtCCCD->Focus();
            return false;
        }
        if (String::IsNullOrWhiteSpace(txtDiaChi->Text)) {
            MessageBox::Show("Dia chi khong duoc de trong.", "Validate", MessageBoxButtons::OK, MessageBoxIcon::Warning);
            txtDiaChi->Focus();
            return false;
        }
        if (txtDiaChi->Text->Length > 250) {
            MessageBox::Show("Dia chi toi da 250 ky tu.", "Validate", MessageBoxButtons::OK, MessageBoxIcon::Warning);
            txtDiaChi->Focus();
            return false;
        }
        if (String::IsNullOrWhiteSpace(txtKey->Text) || txtKey->Text->Length < 6) {
            MessageBox::Show("Key ma/giai ma khong hop le (toi thieu 6 ky tu).", "Validate", MessageBoxButtons::OK, MessageBoxIcon::Warning);
            txtKey->Focus();
            return false;
        }
        return true;
    }

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

    void luuHoSoNguoiDung(OdbcConnection^ conn, Int64 nguoiDungId, const UserRecord& gocEnc, const UserRecord& masked, String^ keyHash) {
        OdbcCommand^ cmd = gcnew OdbcCommand(
            "INSERT INTO HoSoNguoiDung (nguoi_dung_id, ho_ten_enc, so_dien_thoai_enc, email_enc, cccd_enc, dia_chi_enc, "
            "ho_ten_masked, so_dien_thoai_masked, email_masked, cccd_masked, dia_chi_masked, key_hash) "
            "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?) "
            "ON DUPLICATE KEY UPDATE ho_ten_enc = VALUES(ho_ten_enc), so_dien_thoai_enc = VALUES(so_dien_thoai_enc), "
            "email_enc = VALUES(email_enc), cccd_enc = VALUES(cccd_enc), dia_chi_enc = VALUES(dia_chi_enc), "
            "ho_ten_masked = VALUES(ho_ten_masked), so_dien_thoai_masked = VALUES(so_dien_thoai_masked), "
            "email_masked = VALUES(email_masked), cccd_masked = VALUES(cccd_masked), dia_chi_masked = VALUES(dia_chi_masked), "
            "key_hash = VALUES(key_hash)",
            conn
        );
        cmd->Parameters->AddWithValue("", nguoiDungId);
        cmd->Parameters->AddWithValue("", gcnew String(gocEnc.hoTen));
        cmd->Parameters->AddWithValue("", gcnew String(gocEnc.soDienThoai));
        cmd->Parameters->AddWithValue("", gcnew String(gocEnc.email));
        cmd->Parameters->AddWithValue("", gcnew String(gocEnc.cccd));
        cmd->Parameters->AddWithValue("", gcnew String(gocEnc.diaChi));
        cmd->Parameters->AddWithValue("", gcnew String(masked.hoTen));
        cmd->Parameters->AddWithValue("", gcnew String(masked.soDienThoai));
        cmd->Parameters->AddWithValue("", gcnew String(masked.email));
        cmd->Parameters->AddWithValue("", gcnew String(masked.cccd));
        cmd->Parameters->AddWithValue("", gcnew String(masked.diaChi));
        cmd->Parameters->AddWithValue("", keyHash);
        cmd->ExecuteNonQuery();
    }

    String^ docChuoiCot(OdbcDataReader^ reader, int index) {
        if (reader->IsDBNull(index)) {
            return "";
        }
        return Convert::ToString(reader[index]);
    }

    bool docHoSoTheoIdNguoiDung(OdbcConnection^ conn, long long id, UserRecord& gocEnc, UserRecord& masked, String^% keyHash) {
        OdbcCommand^ cmd = gcnew OdbcCommand(
            "SELECT ho_ten_enc, so_dien_thoai_enc, email_enc, cccd_enc, dia_chi_enc, "
            "ho_ten_masked, so_dien_thoai_masked, email_masked, cccd_masked, dia_chi_masked, key_hash "
            "FROM HoSoNguoiDung WHERE nguoi_dung_id = ?",
            conn
        );
        cmd->Parameters->AddWithValue("", id);

        OdbcDataReader^ reader = cmd->ExecuteReader();
        if (!reader->Read()) {
            reader->Close();
            return false;
        }

        copyManagedStringToCharBuffer(docChuoiCot(reader, 0), gocEnc.hoTen, MAX_HO_TEN);
        copyManagedStringToCharBuffer(docChuoiCot(reader, 1), gocEnc.soDienThoai, MAX_SDT);
        copyManagedStringToCharBuffer(docChuoiCot(reader, 2), gocEnc.email, MAX_EMAIL);
        copyManagedStringToCharBuffer(docChuoiCot(reader, 3), gocEnc.cccd, MAX_CCCD);
        copyManagedStringToCharBuffer(docChuoiCot(reader, 4), gocEnc.diaChi, MAX_DIA_CHI);

        copyManagedStringToCharBuffer(docChuoiCot(reader, 5), masked.hoTen, MAX_HO_TEN);
        copyManagedStringToCharBuffer(docChuoiCot(reader, 6), masked.soDienThoai, MAX_SDT);
        copyManagedStringToCharBuffer(docChuoiCot(reader, 7), masked.email, MAX_EMAIL);
        copyManagedStringToCharBuffer(docChuoiCot(reader, 8), masked.cccd, MAX_CCCD);
        copyManagedStringToCharBuffer(docChuoiCot(reader, 9), masked.diaChi, MAX_DIA_CHI);
        keyHash = docChuoiCot(reader, 10);

        reader->Close();
        return true;
    }

    void capNhatManHinh(const UserRecord& goc, const UserRecord& masked, bool choXemGoc) {
        txtMasked->Text = recordToDisplayString(masked);
        if (choXemGoc) {
            txtRaw->Text = recordToDisplayString(goc);
        }
        else {
            txtRaw->Text = "Du lieu goc dang an. Bam 'Xem du lieu goc' va nhap key de xem.";
        }
    }

    void OnMaskAndSaveClick(Object^ sender, EventArgs^ e) {
        if (!daDangNhapAdmin) {
            MessageBox::Show("Ban khong co quyen thuc hien chuc nang nay.", "Tu choi", MessageBoxButtons::OK, MessageBoxIcon::Warning);
            return;
        }
        if (!validateInput()) {
            return;
        }

        UserRecord goc = {};
        UserRecord masked = {};
        UserRecord gocEnc = {};
        char key[MAX_KEY] = { 0 };

        copyManagedStringToCharBuffer(txtHoTen->Text, goc.hoTen, MAX_HO_TEN);
        copyManagedStringToCharBuffer(txtSoDienThoai->Text, goc.soDienThoai, MAX_SDT);
        copyManagedStringToCharBuffer(txtEmail->Text, goc.email, MAX_EMAIL);
        copyManagedStringToCharBuffer(txtCCCD->Text, goc.cccd, MAX_CCCD);
        copyManagedStringToCharBuffer(txtDiaChi->Text, goc.diaChi, MAX_DIA_CHI);
        copyManagedStringToCharBuffer(txtKey->Text, key, MAX_KEY);
        String^ keyHash = bamSHA256Hex(txtKey->Text);

        taoBanGhiDaMask(goc, masked);
        if (!maHoaBanGhi(goc, key, gocEnc)) {
            MessageBox::Show("Khong ma hoa duoc du lieu. Kiem tra key.", "Loi", MessageBoxButtons::OK, MessageBoxIcon::Error);
            return;
        }

        // Mac dinh chi hien du lieu da che
        capNhatManHinh(goc, masked, false);

        try {
            Int64 idHoSo = idTaiKhoanDangNhap;
            if (!String::IsNullOrWhiteSpace(txtSearchId->Text) && !Int64::TryParse(txtSearchId->Text, idHoSo)) {
                MessageBox::Show("ID khong hop le.", "Loi", MessageBoxButtons::OK, MessageBoxIcon::Warning);
                return;
            }

            OdbcConnection^ conn = gcnew OdbcConnection(connStr);
            conn->Open();
            damBaoBangHoSoNguoiDungTonTai(conn);
            damBaoBangNhatKyTonTai(conn);

            luuHoSoNguoiDung(conn, idHoSo, gocEnc, masked, keyHash);
            ghiNhatKy(conn, "SAVE_PROFILE_RECORD", idHoSo, "SUCCESS", "Luu ho so vao bang HoSoNguoiDung");

            conn->Close();
            txtSearchId->Text = Convert::ToString(idHoSo);
            daTaiBanGhiTheoId = true;
            idDangChon = (long long)idHoSo;
            keyHashDangChon = keyHash;
            saoChepChuoi(banGhiEncDangChon->hoTen, gocEnc.hoTen);
            saoChepChuoi(banGhiEncDangChon->soDienThoai, gocEnc.soDienThoai);
            saoChepChuoi(banGhiEncDangChon->email, gocEnc.email);
            saoChepChuoi(banGhiEncDangChon->cccd, gocEnc.cccd);
            saoChepChuoi(banGhiEncDangChon->diaChi, gocEnc.diaChi);
            saoChepChuoi(banGhiMaskedDangChon->hoTen, masked.hoTen);
            saoChepChuoi(banGhiMaskedDangChon->soDienThoai, masked.soDienThoai);
            saoChepChuoi(banGhiMaskedDangChon->email, masked.email);
            saoChepChuoi(banGhiMaskedDangChon->cccd, masked.cccd);
            saoChepChuoi(banGhiMaskedDangChon->diaChi, masked.diaChi);
            MessageBox::Show("Luu ho so thanh cong! nguoi_dung_id = " + Convert::ToString(idHoSo), "Thong bao", MessageBoxButtons::OK, MessageBoxIcon::Information);
        }
        catch (Exception^ ex) {
            MessageBox::Show("Loi ket noi/luu DB:\n" + ex->Message, "Loi", MessageBoxButtons::OK, MessageBoxIcon::Error);
        }
    }

    void OnSearchByIdClick(Object^ sender, EventArgs^ e) {
        if (!daDangNhapAdmin) {
            MessageBox::Show("Ban khong co quyen thuc hien chuc nang nay.", "Tu choi", MessageBoxButtons::OK, MessageBoxIcon::Warning);
            return;
        }
        Int64 id = 0;
        if (!Int64::TryParse(txtSearchId->Text, id)) {
            MessageBox::Show("ID khong hop le.", "Loi", MessageBoxButtons::OK, MessageBoxIcon::Warning);
            return;
        }

        try {
            OdbcConnection^ conn = gcnew OdbcConnection(connStr);
            conn->Open();
            damBaoBangHoSoNguoiDungTonTai(conn);
            damBaoBangNhatKyTonTai(conn);

            UserRecord goc = {};
            UserRecord gocEnc = {};
            UserRecord masked = {};
            String^ keyHash = "";
            bool timThay = docHoSoTheoIdNguoiDung(conn, (long long)id, gocEnc, masked, keyHash);

            if (!timThay) {
                ghiNhatKy(conn, "SEARCH_PROFILE_BY_ID", id, "FAIL", "Khong tim thay nguoi_dung_id trong HoSoNguoiDung");
                conn->Close();
                MessageBox::Show("Khong tim thay du lieu voi ID nay.", "Thong bao", MessageBoxButtons::OK, MessageBoxIcon::Information);
                return;
            }

            // Tim theo ID chi hien du lieu da che
            daTaiBanGhiTheoId = true;
            idDangChon = (long long)id;
            keyHashDangChon = keyHash;
            saoChepChuoi(banGhiEncDangChon->hoTen, gocEnc.hoTen);
            saoChepChuoi(banGhiEncDangChon->soDienThoai, gocEnc.soDienThoai);
            saoChepChuoi(banGhiEncDangChon->email, gocEnc.email);
            saoChepChuoi(banGhiEncDangChon->cccd, gocEnc.cccd);
            saoChepChuoi(banGhiEncDangChon->diaChi, gocEnc.diaChi);
            saoChepChuoi(banGhiMaskedDangChon->hoTen, masked.hoTen);
            saoChepChuoi(banGhiMaskedDangChon->soDienThoai, masked.soDienThoai);
            saoChepChuoi(banGhiMaskedDangChon->email, masked.email);
            saoChepChuoi(banGhiMaskedDangChon->cccd, masked.cccd);
            saoChepChuoi(banGhiMaskedDangChon->diaChi, masked.diaChi);

            capNhatManHinh(goc, masked, false);
            ghiNhatKy(conn, "SEARCH_PROFILE_BY_ID", id, "SUCCESS", "Doc du lieu mask tu HoSoNguoiDung");
            conn->Close();
        }
        catch (Exception^ ex) {
            MessageBox::Show("Loi truy van theo ID:\n" + ex->Message, "Loi", MessageBoxButtons::OK, MessageBoxIcon::Error);
        }
    }

    void OnViewAllMaskedUsersClick(Object^ sender, EventArgs^ e) {
        if (!daDangNhapAdmin) {
            MessageBox::Show("Ban khong co quyen thuc hien chuc nang nay.", "Tu choi", MessageBoxButtons::OK, MessageBoxIcon::Warning);
            return;
        }
        UserListForm^ dsNguoiDungForm = gcnew UserListForm(connStr);
        dsNguoiDungForm->ShowDialog(this);
    }

    void OnNhatKyHeThongClick(Object^ sender, EventArgs^ e) {
        if (!daDangNhapAdmin) {
            MessageBox::Show("Ban khong co quyen thuc hien chuc nang nay.", "Tu choi", MessageBoxButtons::OK, MessageBoxIcon::Warning);
            return;
        }
        AuditLogForm^ nhatKyForm = gcnew AuditLogForm(connStr);
        nhatKyForm->ShowDialog(this);
    }

    void OnQuanLyTaiKhoanClick(Object^ sender, EventArgs^ e) {
        AccountManagementForm^ ql = gcnew AccountManagementForm(connStr, idTaiKhoanDangNhap);
        ql->ShowDialog(this);
    }

    void OnHoSoCaNhanClick(Object^ sender, EventArgs^ e) {
        ProfileForm^ hs = gcnew ProfileForm(connStr, idTaiKhoanDangNhap);
        hs->ShowDialog(this);
    }

    void OnQuanLyFileClick(Object^ sender, EventArgs^ e) {
        FileManagementForm^ fm = gcnew FileManagementForm(connStr, idTaiKhoanDangNhap, daDangNhapAdmin);
        fm->ShowDialog(this);
    }

    void OnDangXuatClick(Object^ sender, EventArgs^ e) {
        YeuCauDangXuat = true;
        this->Close();
    }

    void OnViewRawClick(Object^ sender, EventArgs^ e) {
        if (!daDangNhapAdmin) {
            MessageBox::Show("Ban khong co quyen thuc hien chuc nang nay.", "Tu choi", MessageBoxButtons::OK, MessageBoxIcon::Warning);
            return;
        }
        if (!daTaiBanGhiTheoId) {
            MessageBox::Show("Hay tim theo ID truoc khi xem du lieu goc.", "Thong bao", MessageBoxButtons::OK, MessageBoxIcon::Information);
            return;
        }
        if (String::IsNullOrWhiteSpace(txtKey->Text)) {
            MessageBox::Show("Can nhap key de giai ma du lieu goc.", "Thieu key", MessageBoxButtons::OK, MessageBoxIcon::Warning);
            txtKey->Focus();
            return;
        }

        char key[MAX_KEY] = { 0 };
        UserRecord goc = {};
        copyManagedStringToCharBuffer(txtKey->Text, key, MAX_KEY);

        try {
            OdbcConnection^ conn = gcnew OdbcConnection(connStr);
            conn->Open();
            damBaoBangNhatKyTonTai(conn);

            if (!String::IsNullOrWhiteSpace(keyHashDangChon) &&
                !String::Equals(keyHashDangChon, bamSHA256Hex(txtKey->Text), StringComparison::OrdinalIgnoreCase)) {
                ghiNhatKy(conn, "VIEW_RAW_BY_ID", idDangChon, "FAIL", "Sai key (khong khop key_hash)");
                conn->Close();
                MessageBox::Show("Key khong dung.", "Loi", MessageBoxButtons::OK, MessageBoxIcon::Warning);
                return;
            }

            if (!giaiMaBanGhi(*banGhiEncDangChon, key, goc)) {
                ghiNhatKy(conn, "VIEW_RAW_BY_ID", idDangChon, "FAIL", "Sai key giai ma");
                conn->Close();
                MessageBox::Show("Sai key giai ma hoac du lieu khong dung dinh dang.", "Loi giai ma", MessageBoxButtons::OK, MessageBoxIcon::Error);
                return;
            }

            capNhatManHinh(goc, *banGhiMaskedDangChon, true);
            ghiNhatKy(conn, "VIEW_RAW_BY_ID", idDangChon, "SUCCESS", "Xem du lieu goc theo ID");
            conn->Close();
        }
        catch (Exception^ ex) {
            MessageBox::Show("Loi xem du lieu goc:\n" + ex->Message, "Loi", MessageBoxButtons::OK, MessageBoxIcon::Error);
        }
    }
};
