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

    String^ thuMucLuuFile() {
        String^ p = Path::Combine(Application::StartupPath, "uploaded_files");
        Directory::CreateDirectory(p);
        return p;
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

        String^ key = nhapChuoiDonGian("Key tai xuong file", "Nhap key (se can key nay de tai file):", true);
        if (String::IsNullOrWhiteSpace(key) || key->Length < 6) {
            MessageBox::Show("Key khong hop le (toi thieu 6 ky tu).", "Loi", MessageBoxButtons::OK, MessageBoxIcon::Warning);
            return;
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
            String^ duongDan = Path::Combine(thuMucLuuFile(), tenLuu);
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
            cmd->Parameters->AddWithValue("", bamSHA256Hex(key));
            cmd->ExecuteNonQuery();
            ghiNhatKyChung(conn, "UPLOAD_FILE", Nullable<Int64>(userId), "SUCCESS", "Tai len file: " + tenGoc + (maHoa ? " (DES)" : " (raw)"));
            conn->Close();
            taiDanhSachFile();
            MessageBox::Show("Tai len file thanh cong.", "Thong bao", MessageBoxButtons::OK, MessageBoxIcon::Information);
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

            String^ key = nhapChuoiDonGian("Key tai file", "Nhap key de tai file:", true);
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

            if (daMaHoa == 1) {
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
        this->Size = Drawing::Size(980, 520);
        this->StartPosition = FormStartPosition::CenterParent;

        dgvFile = gcnew DataGridView();
        dgvFile->Location = Point(20, 20);
        dgvFile->Size = Drawing::Size(930, 390);
        dgvFile->ReadOnly = true;
        dgvFile->AllowUserToAddRows = false;
        dgvFile->AllowUserToDeleteRows = false;
        dgvFile->SelectionMode = DataGridViewSelectionMode::FullRowSelect;
        dgvFile->MultiSelect = false;
        dgvFile->AutoSizeColumnsMode = DataGridViewAutoSizeColumnsMode::Fill;
        this->Controls->Add(dgvFile);

        btnUpload = gcnew Button();
        btnUpload->Text = "Tai len file";
        btnUpload->Location = Point(20, 425);
        btnUpload->Size = Drawing::Size(130, 35);
        btnUpload->Click += gcnew EventHandler(this, &FileManagementForm::OnUploadClick);
        this->Controls->Add(btnUpload);

        btnDownload = gcnew Button();
        btnDownload->Text = "Tai xuong file";
        btnDownload->Location = Point(165, 425);
        btnDownload->Size = Drawing::Size(130, 35);
        btnDownload->Click += gcnew EventHandler(this, &FileManagementForm::OnDownloadClick);
        this->Controls->Add(btnDownload);

        btnTaiLai = gcnew Button();
        btnTaiLai->Text = "Tai lai metadata";
        btnTaiLai->Location = Point(310, 425);
        btnTaiLai->Size = Drawing::Size(150, 35);
        btnTaiLai->Click += gcnew EventHandler(this, &FileManagementForm::OnTaiLaiClick);
        this->Controls->Add(btnTaiLai);

        taiDanhSachFile();
    }
};
