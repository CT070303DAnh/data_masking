#pragma once

ref class ProfileForm : public Form {
private:
    String^ connStr;
    Int64 userId;
    TextBox^ txtHoTenMasked;
    TextBox^ txtSdtMasked;
    TextBox^ txtEmailMasked;
    TextBox^ txtCccdMasked;
    TextBox^ txtDiaChiMasked;
    TextBox^ txtHoTenRaw;
    TextBox^ txtSdtRaw;
    TextBox^ txtEmailRaw;
    TextBox^ txtCccdRaw;
    TextBox^ txtDiaChiRaw;
    Button^ btnLuuHoSo;
    Button^ btnXemGoc;

    TextBox^ taoTextBoxReadonly(int x, int y, int w) {
        TextBox^ t = gcnew TextBox();
        t->Location = Point(x, y);
        t->Size = Drawing::Size(w, 24);
        t->ReadOnly = true;
        this->Controls->Add(t);
        return t;
    }

    TextBox^ taoTextBoxInput(int x, int y, int w) {
        TextBox^ t = gcnew TextBox();
        t->Location = Point(x, y);
        t->Size = Drawing::Size(w, 24);
        this->Controls->Add(t);
        return t;
    }

    void taiHoSoMasked() {
        try {
            OdbcConnection^ conn = gcnew OdbcConnection(connStr);
            conn->Open();
            damBaoBangHoSoNguoiDungTonTai(conn);

            OdbcCommand^ cmd = gcnew OdbcCommand(
                "SELECT ho_ten_masked, so_dien_thoai_masked, email_masked, cccd_masked, dia_chi_masked "
                "FROM HoSoNguoiDung WHERE nguoi_dung_id = ?",
                conn
            );
            cmd->Parameters->AddWithValue("", userId);
            OdbcDataReader^ r = cmd->ExecuteReader();
            if (!r->Read()) {
                r->Close();
                conn->Close();
                return;
            }
            txtHoTenMasked->Text = Convert::ToString(r[0]);
            txtSdtMasked->Text = Convert::ToString(r[1]);
            txtEmailMasked->Text = Convert::ToString(r[2]);
            txtCccdMasked->Text = Convert::ToString(r[3]);
            txtDiaChiMasked->Text = Convert::ToString(r[4]);
            r->Close();
            conn->Close();
        }
        catch (Exception^ ex) {
            MessageBox::Show("Loi tai ho so:\n" + ex->Message, "Loi", MessageBoxButtons::OK, MessageBoxIcon::Error);
        }
    }

    void OnLuuHoSoClick(Object^ sender, EventArgs^ e) {
        String^ key = nhapChuoiDonGian("Key ho so", "Nhap key de ma hoa ho so:", true);
        if (String::IsNullOrWhiteSpace(key) || key->Length < 6) {
            MessageBox::Show("Key khong hop le (toi thieu 6 ky tu).", "Loi", MessageBoxButtons::OK, MessageBoxIcon::Warning);
            return;
        }
        try {
            char keyNative[MAX_KEY] = { 0 };
            copyManagedStringToCharBuffer(key, keyNative, MAX_KEY);
            UserRecord goc = {};
            UserRecord gocEnc = {};
            UserRecord masked = {};
            copyManagedStringToCharBuffer(txtHoTenRaw->Text, goc.hoTen, MAX_HO_TEN);
            copyManagedStringToCharBuffer(txtSdtRaw->Text, goc.soDienThoai, MAX_SDT);
            copyManagedStringToCharBuffer(txtEmailRaw->Text, goc.email, MAX_EMAIL);
            copyManagedStringToCharBuffer(txtCccdRaw->Text, goc.cccd, MAX_CCCD);
            copyManagedStringToCharBuffer(txtDiaChiRaw->Text, goc.diaChi, MAX_DIA_CHI);
            taoBanGhiDaMask(goc, masked);
            if (!maHoaBanGhi(goc, keyNative, gocEnc)) {
                MessageBox::Show("Khong ma hoa duoc ho so.", "Loi", MessageBoxButtons::OK, MessageBoxIcon::Error);
                return;
            }

            OdbcConnection^ conn = gcnew OdbcConnection(connStr);
            conn->Open();
            damBaoBangHoSoNguoiDungTonTai(conn);
            damBaoBangNhatKyTonTaiChung(conn);

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
            cmd->Parameters->AddWithValue("", userId);
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
            cmd->Parameters->AddWithValue("", bamSHA256Hex(key));
            cmd->ExecuteNonQuery();
            ghiNhatKyChung(conn, "PROFILE_UPDATE", Nullable<Int64>(userId), "SUCCESS", "Cap nhat ho so ca nhan");
            conn->Close();

            txtHoTenMasked->Text = gcnew String(masked.hoTen);
            txtSdtMasked->Text = gcnew String(masked.soDienThoai);
            txtEmailMasked->Text = gcnew String(masked.email);
            txtCccdMasked->Text = gcnew String(masked.cccd);
            txtDiaChiMasked->Text = gcnew String(masked.diaChi);
            MessageBox::Show("Da luu ho so ca nhan.", "Thong bao", MessageBoxButtons::OK, MessageBoxIcon::Information);
        }
        catch (Exception^ ex) {
            MessageBox::Show("Loi luu ho so:\n" + ex->Message, "Loi", MessageBoxButtons::OK, MessageBoxIcon::Error);
        }
    }

    void OnXemGocClick(Object^ sender, EventArgs^ e) {
        String^ key = nhapChuoiDonGian("Xem du lieu goc", "Nhap key giai ma ho so:", true);
        if (String::IsNullOrWhiteSpace(key)) {
            return;
        }

        try {
            OdbcConnection^ conn = gcnew OdbcConnection(connStr);
            conn->Open();
            damBaoBangHoSoNguoiDungTonTai(conn);
            damBaoBangNhatKyTonTaiChung(conn);
            OdbcCommand^ cmd = gcnew OdbcCommand(
                "SELECT ho_ten_enc, so_dien_thoai_enc, email_enc, cccd_enc, dia_chi_enc, key_hash "
                "FROM HoSoNguoiDung WHERE nguoi_dung_id = ?",
                conn
            );
            cmd->Parameters->AddWithValue("", userId);
            OdbcDataReader^ r = cmd->ExecuteReader();
            if (!r->Read()) {
                r->Close();
                conn->Close();
                MessageBox::Show("Chua co du lieu ho so de giai ma.", "Thong bao", MessageBoxButtons::OK, MessageBoxIcon::Information);
                return;
            }
            String^ keyHashDb = Convert::ToString(r[5]);
            if (!String::Equals(keyHashDb, bamSHA256Hex(key), StringComparison::OrdinalIgnoreCase)) {
                r->Close();
                ghiNhatKyChung(conn, "VIEW_PROFILE_RAW", Nullable<Int64>(userId), "FAIL", "Sai key xem ho so goc");
                conn->Close();
                MessageBox::Show("Key khong dung.", "Loi", MessageBoxButtons::OK, MessageBoxIcon::Warning);
                return;
            }

            UserRecord enc = {};
            UserRecord raw = {};
            copyManagedStringToCharBuffer(Convert::ToString(r[0]), enc.hoTen, MAX_HO_TEN);
            copyManagedStringToCharBuffer(Convert::ToString(r[1]), enc.soDienThoai, MAX_SDT);
            copyManagedStringToCharBuffer(Convert::ToString(r[2]), enc.email, MAX_EMAIL);
            copyManagedStringToCharBuffer(Convert::ToString(r[3]), enc.cccd, MAX_CCCD);
            copyManagedStringToCharBuffer(Convert::ToString(r[4]), enc.diaChi, MAX_DIA_CHI);
            r->Close();

            char keyNative[MAX_KEY] = { 0 };
            copyManagedStringToCharBuffer(key, keyNative, MAX_KEY);
            if (!giaiMaBanGhi(enc, keyNative, raw)) {
                ghiNhatKyChung(conn, "VIEW_PROFILE_RAW", Nullable<Int64>(userId), "FAIL", "Loi giai ma ho so");
                conn->Close();
                MessageBox::Show("Khong giai ma duoc du lieu.", "Loi", MessageBoxButtons::OK, MessageBoxIcon::Error);
                return;
            }

            txtHoTenRaw->Text = gcnew String(raw.hoTen);
            txtSdtRaw->Text = gcnew String(raw.soDienThoai);
            txtEmailRaw->Text = gcnew String(raw.email);
            txtCccdRaw->Text = gcnew String(raw.cccd);
            txtDiaChiRaw->Text = gcnew String(raw.diaChi);
            ghiNhatKyChung(conn, "VIEW_PROFILE_RAW", Nullable<Int64>(userId), "SUCCESS", "Xem ho so goc thanh cong");
            conn->Close();
        }
        catch (Exception^ ex) {
            MessageBox::Show("Loi xem ho so goc:\n" + ex->Message, "Loi", MessageBoxButtons::OK, MessageBoxIcon::Error);
        }
    }

public:
    ProfileForm(String^ ketNoi, Int64 idDangNhap) {
        connStr = ketNoi;
        userId = idDangNhap;
        this->Text = "Ho so ca nhan";
        this->Size = Drawing::Size(900, 430);
        this->StartPosition = FormStartPosition::CenterParent;

        Label^ lb1 = gcnew Label();
        lb1->Text = "Du lieu masked";
        lb1->Font = gcnew Drawing::Font("Segoe UI", 10, FontStyle::Bold);
        lb1->Location = Point(20, 15);
        lb1->AutoSize = true;
        this->Controls->Add(lb1);

        Label^ lb2 = gcnew Label();
        lb2->Text = "Du lieu goc (nhap/sua hoac giai ma)";
        lb2->Font = gcnew Drawing::Font("Segoe UI", 10, FontStyle::Bold);
        lb2->Location = Point(460, 15);
        lb2->AutoSize = true;
        this->Controls->Add(lb2);

        array<String^>^ ten = gcnew array<String^>{"Ho ten", "SDT", "Email", "CCCD", "Dia chi"};
        int y = 50;
        int i = 0;
        while (i < ten->Length) {
            Label^ lb = gcnew Label();
            lb->Text = ten[i];
            lb->Location = Point(20, y + 3);
            lb->Size = Drawing::Size(70, 22);
            this->Controls->Add(lb);

            Label^ lbR = gcnew Label();
            lbR->Text = ten[i];
            lbR->Location = Point(460, y + 3);
            lbR->Size = Drawing::Size(70, 22);
            this->Controls->Add(lbR);
            y = y + 52;
            i = i + 1;
        }

        txtHoTenMasked = taoTextBoxReadonly(95, 50, 330);
        txtSdtMasked = taoTextBoxReadonly(95, 102, 330);
        txtEmailMasked = taoTextBoxReadonly(95, 154, 330);
        txtCccdMasked = taoTextBoxReadonly(95, 206, 330);
        txtDiaChiMasked = taoTextBoxReadonly(95, 258, 330);

        txtHoTenRaw = taoTextBoxInput(535, 50, 330);
        txtSdtRaw = taoTextBoxInput(535, 102, 330);
        txtEmailRaw = taoTextBoxInput(535, 154, 330);
        txtCccdRaw = taoTextBoxInput(535, 206, 330);
        txtDiaChiRaw = taoTextBoxInput(535, 258, 330);

        btnLuuHoSo = gcnew Button();
        btnLuuHoSo->Text = "Luu ho so (ma hoa)";
        btnLuuHoSo->Location = Point(535, 312);
        btnLuuHoSo->Size = Drawing::Size(155, 32);
        btnLuuHoSo->Click += gcnew EventHandler(this, &ProfileForm::OnLuuHoSoClick);
        this->Controls->Add(btnLuuHoSo);

        btnXemGoc = gcnew Button();
        btnXemGoc->Text = "Giai ma de xem goc";
        btnXemGoc->Location = Point(710, 312);
        btnXemGoc->Size = Drawing::Size(155, 32);
        btnXemGoc->Click += gcnew EventHandler(this, &ProfileForm::OnXemGocClick);
        this->Controls->Add(btnXemGoc);

        taiHoSoMasked();
    }
};
