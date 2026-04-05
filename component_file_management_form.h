#pragma once

ref class FileManagementForm : public Form {
private:
    String^ connStr;
    Int64 userId;
    bool laAdmin;
    DataGridView^ dgvFile;
    Button^ btnUpload;
    Button^ btnDownload;
    Button^ btnTaiLai;
    TextBox^ txtReview;

    String^ thuMucLuuFileMaHoa() {
        String^ p = Path::Combine(Application::StartupPath, "filemahoa");
        Directory::CreateDirectory(p);
        return p;
    }

    String^ thuMucLuuFileKhongMaHoa() {
        String^ p = Path::Combine(Application::StartupPath, "filekomahoa");
        Directory::CreateDirectory(p);
        return p;
    }

    String^ bytesToHexXemTruoc(array<Byte>^ data, int gioiHanByte) {
        if (data == nullptr || data->Length == 0) {
            return "(File rong)";
        }
        int soByte = data->Length;
        if (soByte > gioiHanByte) {
            soByte = gioiHanByte;
        }
        StringBuilder^ sb = gcnew StringBuilder(soByte * 2 + 128);
        int i = 0;
        while (i < soByte) {
            sb->Append(data[i].ToString("X2"));
            if ((i + 1) % 32 == 0) {
                sb->Append(Environment::NewLine);
            }
            else {
                sb->Append(' ');
            }
            i = i + 1;
        }
        if (data->Length > gioiHanByte) {
            sb->Append(Environment::NewLine + "...(chi hien thi " + Convert::ToString(gioiHanByte) + " byte dau)");
        }
        return sb->ToString();
    }

    String^ textXemTruocTuBytes(array<Byte>^ data, int gioiHanByte) {
        if (data == nullptr || data->Length == 0) {
            return "(File rong)";
        }
        int soByte = data->Length;
        if (soByte > gioiHanByte) {
            soByte = gioiHanByte;
        }
        array<Byte>^ cat = gcnew array<Byte>(soByte);
        Buffer::BlockCopy(data, 0, cat, 0, soByte);
        String^ noiDung = Encoding::UTF8->GetString(cat);
        if (data->Length > gioiHanByte) {
            noiDung = noiDung + Environment::NewLine + "...(chi hien thi " + Convert::ToString(gioiHanByte) + " byte dau)";
        }
        return noiDung;
    }

    void hienThiReviewTheoDongDuocChon() {
        if (dgvFile == nullptr || dgvFile->SelectedRows->Count != 1) {
            txtReview->Text = "Chon 1 file de xem review noi dung.";
            return;
        }
        try {
            Int64 idFile = Convert::ToInt64(dgvFile->SelectedRows[0]->Cells["id"]->Value);
            OdbcConnection^ conn = gcnew OdbcConnection(connStr);
            conn->Open();
            OdbcCommand^ cmd = gcnew OdbcCommand(
                "SELECT ten_file_goc, duong_dan_luu, da_ma_hoa FROM TepTinNguoiDung WHERE id = ?",
                conn
            );
            cmd->Parameters->AddWithValue("", idFile);
            OdbcDataReader^ r = cmd->ExecuteReader();
            if (!r->Read()) {
                r->Close();
                conn->Close();
                txtReview->Text = "Khong tim thay metadata file.";
                return;
            }
            String^ tenGoc = Convert::ToString(r[0]);
            String^ duongDanLuu = Convert::ToString(r[1]);
            int daMaHoa = Convert::ToInt32(r[2]);
            r->Close();
            conn->Close();

            if (!File::Exists(duongDanLuu)) {
                txtReview->Text = "Khong tim thay file vat ly tai: " + duongDanLuu;
                return;
            }
            array<Byte>^ data = File::ReadAllBytes(duongDanLuu);
            if (daMaHoa == 1) {
                txtReview->Text = "[Review noi dung ma hoa] File: " + tenGoc + Environment::NewLine
                    + "Noi dung duoc hien thi duoi dang HEX (ciphertext):" + Environment::NewLine
                    + bytesToHexXemTruoc(data, 2048);
            }
            else {
                txtReview->Text = "[Review noi dung goc] File: " + tenGoc + Environment::NewLine
                    + textXemTruocTuBytes(data, 8192);
            }
        }
        catch (Exception^ ex) {
            txtReview->Text = "Loi doc review file: " + ex->Message;
        }
    }

    void OnDgvFileSelectionChanged(Object^ sender, EventArgs^ e) {
        hienThiReviewTheoDongDuocChon();
    }

    void taiDanhSachFile() {
        try {
            OdbcConnection^ conn = gcnew OdbcConnection(connStr);
            conn->Open();
            damBaoBangTepTinNguoiDungTonTai(conn);
            damBaoBangTaiKhoanNguoiDungTonTai(conn);
            String^ sql = "SELECT f.id, f.ten_file_goc, f.loai_file, u.ten_dang_nhap AS nguoi_tai_len, f.da_ma_hoa, f.thoi_gian_tao "
                "FROM TepTinNguoiDung f JOIN TaiKhoanNguoiDung u ON f.nguoi_tai_len_id = u.id ORDER BY f.id DESC";
            OdbcDataAdapter^ ad = gcnew OdbcDataAdapter(sql, conn);
            DataTable^ dt = gcnew DataTable();
            ad->Fill(dt);
            dgvFile->DataSource = dt;
            hienThiReviewTheoDongDuocChon();
            conn->Close();
        }
        catch (Exception^ ex) {
            MessageBox::Show("Loi tai metadata file:\n" + ex->Message, "Loi", MessageBoxButtons::OK, MessageBoxIcon::Error);
        }
    }

    void OnUploadClick(Object^ sender, EventArgs^ e) {
        OpenFileDialog^ ofd = gcnew OpenFileDialog();
        ofd->Title = "Chon file tai len";
        if (ofd->ShowDialog() != System::Windows::Forms::DialogResult::OK) {
            return;
        }

        bool maHoa = MessageBox::Show(
            "Ban co muon ma hoa file bang DES truoc khi tai len khong?",
            "Lua chon ma hoa",
            MessageBoxButtons::YesNo,
            MessageBoxIcon::Question
        ) == System::Windows::Forms::DialogResult::Yes;

        String^ key = "";
        if (maHoa) {
            key = nhapChuoiDonGian("Key ma hoa file", "Nhap key de ma hoa file (se can key nay de tai/giai ma):", true);
            if (String::IsNullOrWhiteSpace(key) || key->Length < 6) {
                MessageBox::Show("Key khong hop le (toi thieu 6 ky tu).", "Loi", MessageBoxButtons::OK, MessageBoxIcon::Warning);
                return;
            }
        }

        try {
            array<Byte>^ goc = File::ReadAllBytes(ofd->FileName);
            array<Byte>^ duLieuLuu = goc;
            if (maHoa) {
                duLieuLuu = maHoaDESDuLieuNhiPhan(goc, key);
                if (duLieuLuu == nullptr) {
                    MessageBox::Show("Khong ma hoa duoc file.", "Loi", MessageBoxButtons::OK, MessageBoxIcon::Error);
                    return;
                }
            }

            String^ tenGoc = Path::GetFileName(ofd->FileName);
            String^ ext = Path::GetExtension(ofd->FileName);
            String^ tenLuu = DateTime::Now.Ticks.ToString() + "_" + tenGoc + (maHoa ? ".des" : ".raw");
            String^ duongDan = Path::Combine(maHoa ? thuMucLuuFileMaHoa() : thuMucLuuFileKhongMaHoa(), tenLuu);
            File::WriteAllBytes(duongDan, duLieuLuu);

            OdbcConnection^ conn = gcnew OdbcConnection(connStr);
            conn->Open();
            damBaoBangTepTinNguoiDungTonTai(conn);
            damBaoBangNhatKyTonTaiChung(conn);
            OdbcCommand^ cmd = gcnew OdbcCommand(
                "INSERT INTO TepTinNguoiDung (ten_file_goc, duong_dan_luu, loai_file, nguoi_tai_len_id, da_ma_hoa, key_hash) "
                "VALUES (?, ?, ?, ?, ?, ?)",
                conn
            );
            cmd->Parameters->AddWithValue("", tenGoc);
            cmd->Parameters->AddWithValue("", duongDan);
            cmd->Parameters->AddWithValue("", ext);
            cmd->Parameters->AddWithValue("", userId);
            cmd->Parameters->AddWithValue("", maHoa ? 1 : 0);
            if (maHoa) {
                cmd->Parameters->AddWithValue("", bamSHA256Hex(key));
            }
            else {
                cmd->Parameters->AddWithValue("", DBNull::Value);
            }
            cmd->ExecuteNonQuery();
            ghiNhatKyChung(conn, "UPLOAD_FILE", Nullable<Int64>(userId), "SUCCESS", "Tai len file: " + tenGoc + (maHoa ? " -> filemahoa" : " -> filekomahoa"));
            conn->Close();
            taiDanhSachFile();
            MessageBox::Show("Tai len file thanh cong (" + (maHoa ? "da ma hoa, luu filemahoa" : "khong ma hoa, luu filekomahoa") + ").", "Thong bao", MessageBoxButtons::OK, MessageBoxIcon::Information);
        }
        catch (Exception^ ex) {
            MessageBox::Show("Loi tai len file:\n" + ex->Message, "Loi", MessageBoxButtons::OK, MessageBoxIcon::Error);
        }
    }

    void OnDownloadClick(Object^ sender, EventArgs^ e) {
        if (dgvFile->SelectedRows->Count != 1) {
            MessageBox::Show("Hay chon 1 file trong danh sach.", "Thong bao", MessageBoxButtons::OK, MessageBoxIcon::Information);
            return;
        }
        Int64 idFile = Convert::ToInt64(dgvFile->SelectedRows[0]->Cells["id"]->Value);

        try {
            OdbcConnection^ conn = gcnew OdbcConnection(connStr);
            conn->Open();
            damBaoBangTepTinNguoiDungTonTai(conn);
            damBaoBangNhatKyTonTaiChung(conn);

            String^ sql = "SELECT ten_file_goc, duong_dan_luu, da_ma_hoa, key_hash, nguoi_tai_len_id FROM TepTinNguoiDung WHERE id = ?";
            OdbcCommand^ cmd = gcnew OdbcCommand(sql, conn);
            cmd->Parameters->AddWithValue("", idFile);
            OdbcDataReader^ r = cmd->ExecuteReader();
            if (!r->Read()) {
                r->Close();
                conn->Close();
                MessageBox::Show("Khong tim thay file.", "Thong bao", MessageBoxButtons::OK, MessageBoxIcon::Warning);
                return;
            }

            String^ tenGoc = Convert::ToString(r[0]);
            String^ duongDanLuu = Convert::ToString(r[1]);
            int daMaHoa = Convert::ToInt32(r[2]);
            String^ keyHash = r->IsDBNull(3) ? "" : Convert::ToString(r[3]);
            Int64 nguoiTaiLenId = Convert::ToInt64(r[4]);
            r->Close();

            SaveFileDialog^ sfd = gcnew SaveFileDialog();
            sfd->FileName = tenGoc;
            if (sfd->ShowDialog() != System::Windows::Forms::DialogResult::OK) {
                conn->Close();
                return;
            }

            if (daMaHoa == 1) {
                String^ key = nhapChuoiDonGian("Key tai file", "Nhap key de tai file da ma hoa:", true);
                if (String::IsNullOrWhiteSpace(key)) {
                    conn->Close();
                    return;
                }
                if (!String::Equals(keyHash, bamSHA256Hex(key), StringComparison::OrdinalIgnoreCase)) {
                    ghiNhatKyChung(conn, "DOWNLOAD_FILE", Nullable<Int64>(userId), "FAIL", "Sai key tai file id=" + Convert::ToString(idFile) + ", uploader=" + Convert::ToString(nguoiTaiLenId));
                    conn->Close();
                    MessageBox::Show("Key khong dung.", "Loi", MessageBoxButtons::OK, MessageBoxIcon::Warning);
                    return;
                }
                array<Byte>^ dataEnc = File::ReadAllBytes(duongDanLuu);
                array<Byte>^ dataRaw = giaiMaDESDuLieuNhiPhan(dataEnc, key);
                if (dataRaw == nullptr) {
                    ghiNhatKyChung(conn, "DOWNLOAD_FILE", Nullable<Int64>(userId), "FAIL", "Loi giai ma file id=" + Convert::ToString(idFile));
                    conn->Close();
                    MessageBox::Show("Khong the giai ma file.", "Loi", MessageBoxButtons::OK, MessageBoxIcon::Error);
                    return;
                }
                File::WriteAllBytes(sfd->FileName, dataRaw);
            }
            else {
                File::Copy(duongDanLuu, sfd->FileName, true);
            }

            ghiNhatKyChung(conn, "DOWNLOAD_FILE", Nullable<Int64>(userId), "SUCCESS", "Tai file id=" + Convert::ToString(idFile));
            conn->Close();
            MessageBox::Show("Tai file thanh cong.", "Thong bao", MessageBoxButtons::OK, MessageBoxIcon::Information);
        }
        catch (Exception^ ex) {
            MessageBox::Show("Loi tai file:\n" + ex->Message, "Loi", MessageBoxButtons::OK, MessageBoxIcon::Error);
        }
    }

    void OnTaiLaiClick(Object^ sender, EventArgs^ e) {
        taiDanhSachFile();
    }

public:
    FileManagementForm(String^ ketNoi, Int64 idDangNhap, bool roleAdmin) {
        connStr = ketNoi;
        userId = idDangNhap;
        laAdmin = roleAdmin;
        this->Text = "Quan ly file dung chung";
        this->Size = Drawing::Size(980, 760);
        this->StartPosition = FormStartPosition::CenterParent;

        dgvFile = gcnew DataGridView();
        dgvFile->Location = Point(20, 20);
        dgvFile->Size = Drawing::Size(930, 320);
        dgvFile->ReadOnly = true;
        dgvFile->AllowUserToAddRows = false;
        dgvFile->AllowUserToDeleteRows = false;
        dgvFile->SelectionMode = DataGridViewSelectionMode::FullRowSelect;
        dgvFile->MultiSelect = false;
        dgvFile->AutoSizeColumnsMode = DataGridViewAutoSizeColumnsMode::Fill;
        dgvFile->SelectionChanged += gcnew EventHandler(this, &FileManagementForm::OnDgvFileSelectionChanged);
        this->Controls->Add(dgvFile);

        btnUpload = gcnew Button();
        btnUpload->Text = "Tai len file";
        btnUpload->Location = Point(20, 355);
        btnUpload->Size = Drawing::Size(130, 35);
        btnUpload->Click += gcnew EventHandler(this, &FileManagementForm::OnUploadClick);
        this->Controls->Add(btnUpload);

        btnDownload = gcnew Button();
        btnDownload->Text = "Tai xuong file";
        btnDownload->Location = Point(165, 355);
        btnDownload->Size = Drawing::Size(130, 35);
        btnDownload->Click += gcnew EventHandler(this, &FileManagementForm::OnDownloadClick);
        this->Controls->Add(btnDownload);

        btnTaiLai = gcnew Button();
        btnTaiLai->Text = "Tai lai metadata";
        btnTaiLai->Location = Point(310, 355);
        btnTaiLai->Size = Drawing::Size(150, 35);
        btnTaiLai->Click += gcnew EventHandler(this, &FileManagementForm::OnTaiLaiClick);
        this->Controls->Add(btnTaiLai);

        Label^ lbReview = gcnew Label();
        lbReview->Text = "Review noi dung file";
        lbReview->Location = Point(20, 405);
        lbReview->Size = Drawing::Size(220, 24);
        this->Controls->Add(lbReview);

        txtReview = gcnew TextBox();
        txtReview->Location = Point(20, 432);
        txtReview->Size = Drawing::Size(930, 280);
        txtReview->Multiline = true;
        txtReview->ReadOnly = true;
        txtReview->ScrollBars = ScrollBars::Vertical;
        txtReview->Font = gcnew Drawing::Font("Consolas", 9);
        txtReview->Text = "Chon 1 file de xem review noi dung.";
        this->Controls->Add(txtReview);

        taiDanhSachFile();
    }
};
